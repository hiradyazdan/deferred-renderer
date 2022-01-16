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
					STACK_ONLY(SamplerInfo)

					VkFilter							magFilter					= VK_FILTER_NEAREST;
					VkFilter 							minFilter					= VK_FILTER_NEAREST;
					VkSamplerMipmapMode		mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
					VkSamplerAddressMode	addressModeU			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
					VkSamplerAddressMode	addressModeV			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
					VkSamplerAddressMode	addressModeW			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
					float									mipLodBias				= 0.0f;
					VkBool32              compareEnable			= true;
					VkCompareOp						compareOp					= VK_COMPARE_OP_NEVER;
					float									maxAnisotropy			= 1.0f;
					VkBool32							anisotropyEnable	= true;
					float									minLod						= 0.0f;
					float									maxLod						= 1.0f;
					VkBorderColor					borderColor				= VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
				};
			};

		public:
			static void create(
				const VkDevice          &_logicalDevice,
				const VkExtent2D        &_extent,
				const VkFormat          &_format,
				const VkImageTiling     &_tiling,
				const VkImageUsageFlags	&_usage,
				VkImage                 &_image,
				uint32_t								_mipLevelCount	= 1,
				bool										_isStaging			= false
			) noexcept;
			static void createMemory(
				const VkDevice            							&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkImage                   				&_image,
				VkDeviceMemory              						&_imageMemory,
				const VkMemoryPropertyFlags							&_propFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VkDeviceSize														_offset			= 0
			) noexcept;
			static void createMemory(
				const VkDevice            							&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkImage                   				&_image,
				VkDeviceMemory              						&_imageMemory,
				VkDeviceSize														&_memReqsSize,
				const VkMemoryPropertyFlags							&_propFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VkDeviceSize														_offset			= 0
			) noexcept;
			static void createMemoryBarrier(
				const VkImage					&_image,
				VkImageMemoryBarrier	&_imgMemBarrier,
				uint32_t						  _levelCount				= 1,
				uint32_t							_layerCount				= 1,
				uint32_t							_baseMipLevel			= 0,
				const VkImageLayout		&_oldImageLayout	= VK_IMAGE_LAYOUT_UNDEFINED,
				const VkImageLayout		&_newImageLayout	= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			) noexcept;
			static void createImageView(
				const VkDevice          	&_logicalDevice,
				const VkImage           	&_image,
				const VkFormat            &_format,
				VkImageView               &_imageView,
				uint32_t									_levelCount		= 1,
				uint32_t									_layerCount		= 1,
				uint32_t									_baseMipLevel = 0,
				const VkImageAspectFlags	&_aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT,
				const VkImageViewType			&_viewType		= VK_IMAGE_VIEW_TYPE_2D
			) noexcept;
			static void createSampler(
				const VkDevice 						&_logicalDevice,
				const Data::SamplerInfo		&_info,
				VkSampler									&_sampler
			) noexcept;

		private:
			static void createMemory(
				const VkDevice            							&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkImage                   				&_image,
				VkMemoryRequirements										&_memReqs,
				VkDeviceMemory              						&_imageMemory,
				const VkMemoryPropertyFlags							&_propFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VkDeviceSize														_offset			= 0
			) noexcept;
	};
}