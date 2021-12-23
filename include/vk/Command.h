#pragma once

#include "utils.h"

namespace vk
{
	class Command
	{
		friend class Device;
		struct Data
		{
			VkCommandPool									cmdPool							= VK_NULL_HANDLE;
			VkCommandBuffer								offscreenCmdBuffer	= VK_NULL_HANDLE;
			std::vector<VkCommandBuffer>	drawCmdBuffers;
		};

		public:
			static void createPool(
				const VkDevice	&_logicalDevice,
				uint32_t      	_queueFamilyIndex,
				VkCommandPool		&_cmdPool
			) noexcept;

			static void allocateCmdBuffers(
				const VkDevice							&_logicalDevice,
				const VkCommandPool					&_cmdPool,
				VkCommandBuffer							*_cmdBuffers,
				uint32_t										_bufferCount,
				const VkCommandBufferLevel	&_allocLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY
			) noexcept;
	};
}