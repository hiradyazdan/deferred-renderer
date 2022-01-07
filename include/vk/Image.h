#pragma once

#include "utils.h"

namespace vk
{
	class Image
	{
		public:
			struct Data
			{


				struct SamplerInfo
				{
					STACK_ONLY(SamplerInfo);

					VkFilter							magFilter			= VK_FILTER_NEAREST;
					VkFilter 							minFilter			= VK_FILTER_NEAREST;
					VkSamplerMipmapMode		mipmapMode		= VK_SAMPLER_MIPMAP_MODE_LINEAR;
					VkSamplerAddressMode	addressModeU	= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
					VkSamplerAddressMode	addressModeV	= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
					VkSamplerAddressMode	addressModeW	= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
					float									mipLodBias		= 0.0f;
					float									maxAnisotropy	= 1.0f;
					float									minLod				= 0.0f;
					float									maxLod				= 1.0f;
					VkBorderColor					borderColor		= VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
				};
			};

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
			static void createSampler(
				const VkDevice 							&_logicalDevice,
				VkSampler										&_sampler,
				Data::SamplerInfo						_info = getSamplerInfo()
			);

		private:
			static Data::SamplerInfo getSamplerInfo() noexcept { return {}; }
	};
}