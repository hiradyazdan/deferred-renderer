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

	void Sync::createFence(
		const VkDevice			&_logicalDevice,
		VkFence							&_fence,
		VkFenceCreateFlags	_flags
	) noexcept
	{
		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = _flags;

		auto result = vkCreateFence(
			_logicalDevice,
			&info,
			nullptr,
			&_fence
		);
		ASSERT_VK(result, "Failed to create Fence!");
	}
}