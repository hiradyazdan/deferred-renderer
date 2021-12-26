#include "vk/Framebuffer.h"

namespace vk
{
	void Framebuffer::create(
		const VkDevice								&_logicalDevice,
		const VkFramebufferCreateInfo	&_info,
		VkFramebuffer									&_framebuffer
	) noexcept
	{
		auto result = vkCreateFramebuffer(
			_logicalDevice,
			&_info,
			nullptr,
			&_framebuffer
		);
		ASSERT_VK(result, "Failed to create Framebuffer");
	}
}