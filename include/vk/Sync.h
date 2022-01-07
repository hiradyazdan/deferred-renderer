#pragma once

#include "utils.h"

namespace vk
{
	class Sync
	{
		friend class Device;
		struct Data
		{
			struct
			{
				VkSemaphore presentComplete	= VK_NULL_HANDLE;
				VkSemaphore renderComplete	= VK_NULL_HANDLE;
			} semaphores;

			std::vector<VkFence> waitFences; // inflight
		};

		public:
			static void createSemaphore(
				const VkDevice	&_logicalDevice,
				VkSemaphore			&_semaphore
			) noexcept;

			static void createFence(
				const VkDevice			&_logicalDevice,
				VkFence							&_fence,
				VkFenceCreateFlags	_flags = 0
			)	noexcept;

			static void destroyFence(
				const VkDevice			&_logicalDevice,
				VkFence							&_fence
			) noexcept;

			template<uint16_t fenceCount = 1>
			static void waitForFences(
				const VkDevice	&_logicalDevice,
				const VkFence		*_pFences,
				VkBool32				_waitAll = VK_TRUE,
				uint64_t				_timeout = 100000000000
			) noexcept
			{
				const auto result = vkWaitForFences(
					_logicalDevice,
					fenceCount,
					_pFences,
					VK_TRUE, _timeout
				);

				ASSERT_VK(result, "Failed to wait for fences!");
			}
	};
}