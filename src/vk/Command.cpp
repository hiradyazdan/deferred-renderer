#include "vk/Command.h"

namespace vk
{
	void Command::createPool(
		const VkDevice	&_logicalDevice,
		uint32_t      	_queueFamilyIndex,
		VkCommandPool		&_cmdPool
	) noexcept
	{
		VkCommandPoolCreateInfo info	= {};
		info.sType										= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.queueFamilyIndex					= _queueFamilyIndex;
		info.flags										= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		auto result = vkCreateCommandPool(
			_logicalDevice,
			&info,
			nullptr,
			&_cmdPool
		);
		ASSERT_VK(result, "Failed to create a Command Pool!");
	}

	void Command::allocateCmdBuffers(
		const VkDevice							&_logicalDevice,
		const VkCommandPool					&_cmdPool,
		VkCommandBuffer							*_cmdBuffers,
		uint32_t										_bufferCount,
		const VkCommandBufferLevel	&_allocLevel
	) noexcept
	{
		VkCommandBufferAllocateInfo allocInfo  = {};

		allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool                 = _cmdPool;
		allocInfo.commandBufferCount          = _bufferCount;
		allocInfo.level                       = _allocLevel;

		const auto &result = vkAllocateCommandBuffers(
			_logicalDevice,
			&allocInfo,
			_cmdBuffers
		);
		ASSERT_VK(result, "Failed to allocate command buffers");
	}
}