#include "vk/Device.h"
#include "vk/Image.h"

namespace vk
{
	void Image::create(
		const VkDevice          &_logicalDevice,
		const VkExtent2D        &_extent,
		const VkFormat          &_format,
		const VkImageTiling     &_tiling,
		const VkImageUsageFlags	&_usage,
		VkImage                 &_image,
		uint32_t								_mipLevelCount,
		bool										_isStaging
	) noexcept
	{
		VkImageCreateInfo imageInfo = {};

		imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType     = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width  = _extent.width;
		imageInfo.extent.height = _extent.height;
		imageInfo.extent.depth  = 1;
		imageInfo.mipLevels     = _mipLevelCount;
		imageInfo.arrayLayers   = 1;
		imageInfo.format        = _format;
		imageInfo.tiling        = _tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage         = _usage;
		imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags         = 0;

		if(_isStaging)
		{
			if (!(imageInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
			{
				imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}
		}

		const auto &result = vkCreateImage(
			_logicalDevice,
			&imageInfo,
			nullptr,
			&_image
		);
		ASSERT_VK(result, "Failed to create Image!");
	}

	void Image::createMemory(
		const VkDevice            							&_logicalDevice,
		const VkPhysicalDeviceMemoryProperties	&_memProps,
		const VkImage                   				&_image,
		VkDeviceMemory              						&_imageMemory,
		const VkMemoryPropertyFlags							&_propFlags,
		VkDeviceSize														_offset
	) noexcept
	{
		VkMemoryRequirements memReqs;

		createMemory(
			_logicalDevice, _memProps,
			_image, memReqs, _imageMemory,
			_propFlags, _offset
		);
	}

	void Image::createMemory(
		const VkDevice            							&_logicalDevice,
		const VkPhysicalDeviceMemoryProperties	&_memProps,
		const VkImage                   				&_image,
		VkDeviceMemory              						&_imageMemory,
		VkDeviceSize														&_memReqsSize,
		const VkMemoryPropertyFlags							&_propFlags,
		VkDeviceSize														_offset
	) noexcept
	{
		VkMemoryRequirements memReqs;

		createMemory(
			_logicalDevice, _memProps,
			_image, memReqs, _imageMemory,
			_propFlags, _offset
		);

		_memReqsSize = memReqs.size;
	}

	void Image::createMemory(
		const VkDevice            							&_logicalDevice,
		const VkPhysicalDeviceMemoryProperties	&_memProps,
		const VkImage                   				&_image,
		VkMemoryRequirements										&_memReqs,
		VkDeviceMemory              						&_imageMemory,
		const VkMemoryPropertyFlags							&_propFlags,
		VkDeviceSize														_offset
	) noexcept
	{
		vkGetImageMemoryRequirements(
			_logicalDevice,
			_image,
			&_memReqs
		);

		Device::allocMemory(
			_logicalDevice,
			_memReqs.size,
			Device::getMemoryType(_memReqs.memoryTypeBits, _memProps, _propFlags),
			_imageMemory
		);

		const auto &result = vkBindImageMemory(
			_logicalDevice,
			_image,
			_imageMemory,
			_offset
		);
		ASSERT_VK(result, "Failed to bind Image Memory!");
	}

	void Image::createMemoryBarrier(
		const VkImage					&_image,
		VkImageMemoryBarrier	&_imgMemBarrier,
		uint32_t						  _levelCount,
		uint32_t							_layerCount,
		uint32_t							_baseMipLevel,
		const VkImageLayout		&_oldImageLayout,
		const VkImageLayout		&_newImageLayout
	) noexcept
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask			= VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel 	= _baseMipLevel;
		subresourceRange.levelCount			= _levelCount;
		subresourceRange.layerCount			= _layerCount;

		_imgMemBarrier.sType						= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		_imgMemBarrier.oldLayout				= _oldImageLayout;
		_imgMemBarrier.newLayout				= _newImageLayout;

		// @todo temporary - work out all permutations
		if(_oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		{
			_imgMemBarrier.srcAccessMask		= 0;
		}
		if(_oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			_imgMemBarrier.srcAccessMask		= VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		if(_newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			_imgMemBarrier.dstAccessMask		= VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		if(_newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			if(_imgMemBarrier.srcAccessMask == 0)
			{
				_imgMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}

			_imgMemBarrier.dstAccessMask		= VK_ACCESS_SHADER_READ_BIT;
		}

		_imgMemBarrier.image						= _image;
		_imgMemBarrier.subresourceRange	= subresourceRange;
	}

	void Image::createImageView(
		const VkDevice						&_logicalDevice,
		const VkImage							&_image,
		const VkFormat						&_format,
		VkImageView								&_imageView,
		uint32_t									_levelCount,
		uint32_t									_layerCount,
		uint32_t									_baseMipLevel,
		const VkImageAspectFlags	&_aspectMask,
		const VkImageViewType			&_viewType
	) noexcept
	{
		VkImageViewCreateInfo viewInfo            = {};
		viewInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.flags														= 0;
		viewInfo.image                            = _image;
		viewInfo.viewType                         = _viewType;
		viewInfo.format                           = _format;

		viewInfo.subresourceRange.aspectMask      = _aspectMask;
		viewInfo.subresourceRange.baseMipLevel    = _baseMipLevel;
		viewInfo.subresourceRange.levelCount      = _levelCount;
		viewInfo.subresourceRange.baseArrayLayer  = 0;
		viewInfo.subresourceRange.layerCount      = _layerCount;

		viewInfo.components												= {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};

		const auto &result = vkCreateImageView(
			_logicalDevice,
			&viewInfo,
			nullptr,
			&_imageView
		);
		ASSERT_VK(result, "Failed to create image view");
	}

	void Image::createSampler(
		const VkDevice 							&_logicalDevice,
		const Data::SamplerInfo			&_info,
		VkSampler										&_sampler
	) noexcept
	{
		VkSamplerCreateInfo info = {};
		info.sType						= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.magFilter				= _info.magFilter;
		info.minFilter				= _info.minFilter;
		info.mipmapMode				= _info.mipmapMode;
		info.addressModeU			= _info.addressModeU;
		info.addressModeV			= _info.addressModeV;
		info.addressModeW			= _info.addressModeW;
		info.mipLodBias				= _info.mipLodBias;
		info.compareEnable		= _info.compareEnable;
		info.compareOp				= _info.compareOp;
		info.maxAnisotropy		= _info.maxAnisotropy;
		info.anisotropyEnable	= _info.anisotropyEnable;
		info.minLod						= _info.minLod;
		info.maxLod						= _info.maxLod;
		info.borderColor			= _info.borderColor;

		const auto &result = vkCreateSampler(
			_logicalDevice,
			&info,
			nullptr,
			&_sampler
		);
		ASSERT_VK(result, "Failed to create Sampler");
	}
}