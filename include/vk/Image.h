#pragma once

#include "utils.h"

namespace vk
{
	class Image
	{
		public:
			static void createImage(
				const VkDevice          &_logicalDevice,
				const VkExtent2D        &_extent,
				const VkFormat          &_format,
				const VkImageTiling     &_tiling,
				const VkImageUsageFlags	&_usage,
				VkImage                 &_image
			);
			static void createImageMemory(
				const VkDevice            							&_logicalDevice,
				const VkPhysicalDevice      						&_physicalDevice,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				VkImage                   							&_image,
				VkDeviceMemory              						&_imageMemory,
				const VkMemoryPropertyFlags							&_propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);
			static void createImageMemoryBarrier();
			static void createImageView(
				const VkDevice          	&_logicalDevice,
				const VkImage           	&_image,
				const VkFormat            &_format,
				VkImageView               &_imageView,
				const VkImageAspectFlags	&_aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT,
				const VkImageViewType			&_viewType		= VK_IMAGE_VIEW_TYPE_2D
			);
			static void createSampler();
	};
}