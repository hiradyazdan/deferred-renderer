#include "vk/Device.h"
#include "vk/Buffer.h"
#include "vk/Texture.h"

namespace vk
{
	bool Texture::isKtx(const std::string &_uri) noexcept
	{
		const auto &extPos = _uri.find_last_of('.');

		if(extPos != std::string::npos)
		{
			if(_uri.substr(extPos + 1) == "ktx")
			{
				return true;
			}
		}

		return false;
	}

	void Texture::destroy(
		const VkDevice	&_logicalDevice,
		const Data			&_data
	) noexcept
	{
		auto &views = _data.imageViews;
		auto &images = _data.images;
		auto &samplers = _data.samplers;
		auto &memories = _data.memories;

		for(auto &view : views)
		{
			Image::destroyImageView(_logicalDevice, view);
		}
		for(auto &image : images)
		{
			Image::destroyImage(_logicalDevice, image);
		}
		for(auto &sampler : samplers)
		{
			Image::destroySampler(_logicalDevice, sampler);
		}
		for(auto &memory : memories)
		{
			Device::freeMemory(_logicalDevice, memory);
		}
	}

	void Texture::load(
		const DevicePtr							&_device,
		const std::string						&_fileName,
		const VkFormat							&_format,
		uint16_t										_index,
		Data												&_data,
		bool               					_isLinearTiling,
		const VkAllocationCallbacks	*_pAllocator,
		const VkImageUsageFlags			&_imageUsageFlags,
		const VkImageLayout					&_imageLayout
	) noexcept
	{
		ASSERT_FATAL(_data.size() > 0, "Texture Data should be given a size!");

//		INFO_LOG("Loading Texture %d...", _index);

		ktxTexture *ktxTexture;

		auto result = load(_fileName, &ktxTexture);

		ASSERT_FATAL(result == KTX_SUCCESS, "Failed to load ktx texture!");

		create(
			_device, _format,
			_data, _index, ktxTexture, _isLinearTiling,
			_pAllocator, _imageUsageFlags, _imageLayout
		);
	}

	void Texture::create(
		const DevicePtr							&_device,
		const VkFormat							&_format,
		Data												&_data,
		uint16_t										_index,
		ktxTexture									*_ktxTexture,
		bool            						_isLinearTiling,
		const VkAllocationCallbacks	*_pAllocator,
		const VkImageUsageFlags			&_imageUsageFlags,
		const VkImageLayout					&_imageLayout
	) noexcept
	{
		auto &deviceData = (*_device).getData();
		auto &logicalDevice = deviceData.logicalDevice;
		auto samplerAnisotropy = deviceData.features.samplerAnisotropy;

		const auto isEmpty = _ktxTexture == nullptr;
		const auto &memProps = deviceData.memProps;
		const auto &cmdPool	= deviceData.cmdData.cmdPool;

		auto tempTextureData = Data::Temp::create();

		const auto extent = tempTextureData.imageExtent = {
			!isEmpty ? _ktxTexture->baseWidth		: 1,
			!isEmpty ? _ktxTexture->baseHeight	: 1
		};
		const auto mipLevelCount = tempTextureData.mipLevelCount = !isEmpty ? _ktxTexture->numLevels : 1;
		const auto &textureSize = !isEmpty ? _ktxTexture->dataSize : 4; // empty tex => (w:1) * (h:1) * 4
		const auto &textureData = !isEmpty ? ktxTexture_GetData(_ktxTexture) : new uint8_t[textureSize];
//		const auto &textureFormat = ktxTexture_GetVkFormat(ktxTexture);

		auto &image = _data.images[_index];
		auto &memory = _data.memories[_index];

		tempTextureData.format					= _format;
		tempTextureData.imageUsageFlags	= _imageUsageFlags;
		tempTextureData.imageLayout			= _data.imageLayouts[_index] = _imageLayout;

		VkCommandBuffer copyCmd;
		Command::allocateCmdBuffers(logicalDevice, cmdPool, &copyCmd);

		if(!_isLinearTiling)
		{
//			INFO_LOG("Texture: Optimal Tiling");

			const auto type = Buffer::Type::TEXTURE;
			const auto TEXTURE = 0;
			const auto bufferCount = 1;
			auto tempBufferData = Buffer::TempData<type, bufferCount>::create();
			auto stagingBufferData = Buffer::Data<type, bufferCount>();
			auto &cpuTextureBuffer = stagingBufferData.buffers[TEXTURE];
			auto &cpuTextureMemory = stagingBufferData.memories[TEXTURE];

			tempBufferData.sizes[TEXTURE] = textureSize;
			tempBufferData.entries[TEXTURE] = textureData;

			Buffer::create<type, bufferCount>(
				logicalDevice, memProps,
				tempBufferData,
				stagingBufferData.buffers, stagingBufferData.memories
			);

			tempTextureData.bufferCopyRegions.resize(mipLevelCount);
			for(auto i = 0u; i < mipLevelCount; ++i)
			{
				VkBufferImageCopy copyRegion = {};

				copyRegion.imageSubresource.aspectMask			= VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.imageSubresource.mipLevel				= i;
				copyRegion.imageSubresource.baseArrayLayer	= 0;
				copyRegion.imageSubresource.layerCount			= 1;
				copyRegion.imageExtent.width								= !isEmpty ? std::max(1u, extent.width >> i)	: 1;
				copyRegion.imageExtent.height								= !isEmpty ? std::max(1u, extent.height >> i)	: 1;
				copyRegion.imageExtent.depth								= 1;
				copyRegion.bufferOffset											= !isEmpty ? getImageOffset(_ktxTexture, i) : 0;

				tempTextureData.bufferCopyRegions[i] = copyRegion;
			}

			const auto &recordCallback = [&](const VkCommandBuffer &_cmdBuffer)
			{
				recordStagingBufferCommands(
					logicalDevice, memProps,
					_cmdBuffer,
					tempTextureData,
					cpuTextureBuffer,
					memory, image
				);
			};

			Command::record(copyCmd, recordCallback);
			Command::submitStagingCopyCommand(
				logicalDevice,
				copyCmd,
				cmdPool, deviceData.graphicsQueue,
				"Texture Buffer to Image Copy"
			);

			Buffer::destroy(logicalDevice, stagingBufferData, _pAllocator);
		}
		else
		{
//			INFO_LOG("Texture: Linear Tiling");

			VkFormatProperties formatProps;
			Device::getFormatProps(deviceData.physicalDevice, _format, formatProps);

			ASSERT_FATAL(
				formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,
				"Linear tiling is not supported!"
			);

			tempTextureData.entry = textureData;

			const auto &recordCallback = [&](const VkCommandBuffer &_cmdBuffer)
			{
				recordStagingImageCommands(
					logicalDevice, memProps,
					_cmdBuffer,
					tempTextureData,
					memory, image
				);
			};

			Command::record(copyCmd, recordCallback);
			Command::submitStagingCopyCommand(
				logicalDevice,
				copyCmd,
				cmdPool, deviceData.graphicsQueue,
				"Direct Image Copy"
			);
		}

		if(!isEmpty)
		{
			ktxTexture_Destroy(_ktxTexture);
		}

		Image::Data::SamplerInfo samplerInfo = {};
		samplerInfo.magFilter					= VK_FILTER_LINEAR;
		samplerInfo.minFilter					= VK_FILTER_LINEAR;
		samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias				= 0.0f;
		samplerInfo.compareOp					= VK_COMPARE_OP_NEVER;
		samplerInfo.maxAnisotropy			= samplerAnisotropy
																		 ? deviceData.props.limits.maxSamplerAnisotropy
																		 : 1.0f;
		samplerInfo.anisotropyEnable	= samplerAnisotropy;
		samplerInfo.minLod						= 0.0f;
		samplerInfo.maxLod						= !_isLinearTiling ? static_cast<float>(mipLevelCount) : 0.0f;
		samplerInfo.borderColor				= VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		Image::createSampler(
			logicalDevice,
			samplerInfo,
			_data.samplers[_index]
		);
		Image::createImageView(
			logicalDevice,
			_data.images[_index],
			_format,
			_data.imageViews[_index],
			!_isLinearTiling ? mipLevelCount : 1
		);

		updateDescriptor(_index, _data);
	}

	void Texture::recordStagingBufferCommands(
		const VkDevice					&_logicalDevice,
		const VkPhysicalDeviceMemoryProperties &_memProps,
		const VkCommandBuffer		&_cmdBuffer,
		const Data::Temp				&_inData,
		const VkBuffer 					&_buffer,
		VkDeviceMemory					&_memory,
		VkImage									&_image
	) noexcept
	{
		const auto &mipLevelCount			= _inData.mipLevelCount;
		const auto &bufferCopyRegions	= _inData.bufferCopyRegions;

		Image::create(
			_logicalDevice,
			_inData.imageExtent,
			_inData.format,
			VK_IMAGE_TILING_OPTIMAL, _inData.imageUsageFlags,
			_image, mipLevelCount, true
		);
		Image::createMemory(
			_logicalDevice, _memProps,
			_image,
			_memory
		);

		VkImageMemoryBarrier imgMemBarrier	= {};

		Image::createMemoryBarrier(
			_image,
			imgMemBarrier,
			mipLevelCount
		);
		Command::insertBarriers(
			_cmdBuffer, &imgMemBarrier
		);

		vkCmdCopyBufferToImage(
			_cmdBuffer,
			_buffer,
			_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			bufferCopyRegions.size(),
			bufferCopyRegions.data()
		);

		Image::createMemoryBarrier(
			_image,
			imgMemBarrier,
			mipLevelCount, 1, 0,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			_inData.imageLayout
		);
		Command::insertBarriers(
			_cmdBuffer, &imgMemBarrier
		);
	}

	void Texture::recordStagingImageCommands(
		const VkDevice					&_logicalDevice,
		const VkPhysicalDeviceMemoryProperties &_memProps,
		const VkCommandBuffer		&_cmdBuffer,
		Data::Temp							&_inData,
		VkDeviceMemory					&_memory,
		VkImage									&_image
	) noexcept
	{
		Image::create(
			_logicalDevice,
			_inData.imageExtent,
			_inData.format,
			VK_IMAGE_TILING_LINEAR, _inData.imageUsageFlags,
			_image
		);
		Image::createMemory(
			_logicalDevice, _memProps,
			_image,
			_memory,
			_inData.size,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		VkSubresourceLayout subResLayout;
		VkImageSubresource subRes = {};
		subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subRes.mipLevel = 0;
		vkGetImageSubresourceLayout(
			_logicalDevice,
			_image,
			&subRes, &subResLayout
		);

		void *dataDst;

		Device::mapMemory(_logicalDevice, _memory, &dataDst, _inData.size);

			memcpy(dataDst, _inData.entry, _inData.size);

		Device::unmapMemory(_logicalDevice, _memory);

		VkImageMemoryBarrier imgMemBarrier = {};

		Image::createMemoryBarrier(
			_image,
			imgMemBarrier,
			1, 1, 0,
			VK_IMAGE_LAYOUT_UNDEFINED,
			_inData.imageLayout
		);
		Command::insertBarriers(
			_cmdBuffer, &imgMemBarrier
		);
	}

	void Texture::load() noexcept
	{

	}

	ktxResult Texture::load(
		const std::string	&_fileName,
		ktxTexture				**_texture
	) noexcept
	{
		ktxResult result = KTX_SUCCESS;

#if defined(__ANDROID__)
		// todo
#else
		// @todo Should verify if the file exists
		result = ktxTexture_CreateFromNamedFile(
			_fileName.c_str(),
			KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
			_texture
		);
#endif
		assert(result == KTX_SUCCESS);

		return result;
	}

	void Texture::updateDescriptor(uint16_t _index, Data &_data) noexcept
	{
		auto &descriptor = _data.imageInfos[_index];

		descriptor.sampler			= _data.samplers		[_index];
		descriptor.imageView		= _data.imageViews	[_index];
		descriptor.imageLayout	= _data.imageLayouts[_index];
	}
}