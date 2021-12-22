#include "vk/Sync.h"

namespace vk
{
	void Sync::createSemaphore(
		const VkDevice &_logicalDevice,
		VkSemaphore &_semaphore
	) noexcept
	{
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;

		auto result = vkCreateSemaphore(
			_logicalDevice,
			&info,
			nullptr,
			&_semaphore
		);
		ASSERT_VK(result, "Failed to create Semaphore!");
	}
}