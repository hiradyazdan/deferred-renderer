#pragma once

#include "utils.h"

namespace vk
{
	class Sync
	{
		friend class Device;

		public:
			enum class SemaphoreType
			{
				PRESENT_COMPLETE	= 0,
				RENDER_COMPLETE		= 1,

				_count_					 	= 2
			};

		struct Data
		{
			Array<VkSemaphore, toInt(SemaphoreType::_count_)> semaphores;
			Vector<VkFence>																		waitFences; // inflight
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

			static void destroySemaphore(
				const VkDevice							&_logicalDevice,
				const VkSemaphore						&_semaphore,
				const VkAllocationCallbacks	*_pAllocator = nullptr
			) noexcept;

			static void destroyFence(
				const VkDevice							&_logicalDevice,
				const VkFence								&_fence,
				const VkAllocationCallbacks	*_pAllocator = nullptr
			) noexcept;

			template<size_t semaphoreCount>
			static void destroy(
				const VkDevice														&_logicalDevice,
				const Array<VkSemaphore, semaphoreCount>	&_semaphores,
				Vector<VkFence>														&_fences,
				const VkAllocationCallbacks								*_pAllocator = nullptr
			) noexcept
			{
				for(const auto &semaphore : _semaphores)
				{
					destroySemaphore(_logicalDevice, semaphore, _pAllocator);
				}
				for(const auto &fence : _fences)
				{
					destroyFence(_logicalDevice, fence, _pAllocator);
				}

				_fences.clear();
			}

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