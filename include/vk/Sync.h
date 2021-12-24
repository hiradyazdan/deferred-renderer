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

			std::vector<VkFence> waitFences;
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
	};
}