#pragma once

#include "utils.h"

namespace vk
{
	class Image
	{
		public:
			static void createImageView(
				const VkDevice          	&_logicalDevice,
				const VkImage           	&_image,
				const VkFormat            &_format,
				VkImageView               &_imageView,
				const VkImageAspectFlags	&_aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT,
				const VkImageViewType			&_viewType		= VK_IMAGE_VIEW_TYPE_2D
			);
	};
}