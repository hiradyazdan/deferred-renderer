#include "vk/Image.h"

namespace vk
{
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
}