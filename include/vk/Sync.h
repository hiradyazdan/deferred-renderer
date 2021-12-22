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

				VkSemaphore offscreen				= VK_NULL_HANDLE;
			} semaphores;

		};

		public:
			static void createFence()	noexcept;
			static void createSemaphore(
				const VkDevice &_logicalDevice,
				VkSemaphore &_semaphore
			) noexcept;
	};
}