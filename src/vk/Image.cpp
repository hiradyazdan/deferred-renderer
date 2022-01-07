#include "vk/Image.h"

namespace vk
{
	void Image::createImage(
		const VkDevice &_logicalDevice,
		const VkExtent2D &_extent,
		const VkFormat &_format,
		const VkImageTiling &_tiling,
		const VkImageUsageFlags &_usage,
		VkImage &_image
	)
	{
		VkImageCreateInfo imageInfo = {};

		imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType     = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width  = _extent.width;
		imageInfo.extent.height = _extent.height;
		imageInfo.extent.depth  = 1;
		imageInfo.mipLevels     = 1;
		imageInfo.arrayLayers   = 1;
		imageInfo.format        = _format;
		imageInfo.tiling        = _tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage         = _usage;
		imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags         = 0;

		const auto &result = vkCreateImage(
			_logicalDevice,
			&imageInfo,
			nullptr,
			&_image
		);
		ASSERT_VK(result, "Failed to create Image!");
	}

	void Image::createImageMemory(
		const VkDevice            							&_logicalDevice,
		const VkPhysicalDevice      						&_physicalDevice,
		const VkPhysicalDeviceMemoryProperties	&_memProps,
		VkImage                   							&_image,
		VkDeviceMemory              						&_imageMemory,
		const VkMemoryPropertyFlags							&_propertyFlags
	)
	{
		VkMemoryRequirements memReqs;

		vkGetImageMemoryRequirements(
			_logicalDevice,
			_image,
			&memReqs
		);

		VkMemoryAllocateInfo memAllocInfo = {};

		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.pNext = nullptr;
		memAllocInfo.allocationSize = memReqs.size;

		uint32_t currentTypeFilter;
		VkMemoryPropertyFlags currentProperties;
		bool propertiesMatch;

		for(auto i = 0; i < _memProps.memoryTypeCount; i++)
		{
			currentTypeFilter = memReqs.memoryTypeBits & (1 << i);
			currentProperties = _memProps.memoryTypes[i].propertyFlags;
			propertiesMatch = (currentProperties & _propertyFlags) == _propertyFlags;

			if(currentTypeFilter && propertiesMatch)
			{
				memAllocInfo.memoryTypeIndex = i;
			}
		}

		auto result = vkAllocateMemory(
			_logicalDevice,
			&memAllocInfo,
			nullptr,
			&_imageMemory
		);
		ASSERT_VK(result, "Failed to allocate Image Memory!");

		result = vkBindImageMemory(
			_logicalDevice,
			_image,
			_imageMemory,
			0
		);
		ASSERT_VK(result, "Failed to bind Image Memory!");
	}

	void Image::createImageView(
		const VkDevice &_logicalDevice,
		const VkImage &_image,
		const VkFormat &_format,
		VkImageView &_imageView,
		const VkImageAspectFlags &_aspectMask,
		const VkImageViewType &_viewType
	)
	{
		VkImageViewCreateInfo viewInfo            = {};
		viewInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image                            = _image;
		viewInfo.viewType                         = _viewType;
		viewInfo.format                           = _format;
		viewInfo.subresourceRange.aspectMask      = _aspectMask;
		viewInfo.subresourceRange.baseMipLevel    = 0;
		viewInfo.subresourceRange.levelCount      = 1;
		viewInfo.subresourceRange.baseArrayLayer  = 0;
		viewInfo.subresourceRange.layerCount      = 1;

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
		VkSampler										&_sampler,
		Data::SamplerInfo						_info
	)
	{
		VkSamplerCreateInfo info = {};
		info.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.magFilter			= _info.magFilter;
		info.minFilter			= _info.minFilter;
		info.mipmapMode			= _info.mipmapMode;
		info.addressModeU		= _info.addressModeU;
		info.addressModeV		= _info.addressModeV;
		info.addressModeW		= _info.addressModeW;
		info.mipLodBias			= _info.mipLodBias;
		info.maxAnisotropy	= _info.maxAnisotropy;
		info.minLod					= _info.minLod;
		info.maxLod					= _info.maxLod;
		info.borderColor		= _info.borderColor;

		const auto &result = vkCreateSampler(
			_logicalDevice,
			&info,
			nullptr,
			&_sampler
		);
		ASSERT_VK(result, "Failed to create Sampler");
	}
}