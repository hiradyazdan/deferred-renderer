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
		VkCommandBuffer							*_pCmdBuffers,
		uint32_t										_cmdCount,
		const VkCommandBufferLevel	&_allocLevel
	) noexcept
	{
		VkCommandBufferAllocateInfo allocInfo	= {};
		allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool                 = _cmdPool;
		allocInfo.commandBufferCount          = _cmdCount;
		allocInfo.level                       = _allocLevel;

		const auto &result = vkAllocateCommandBuffers(
			_logicalDevice,
			&allocInfo,
			_pCmdBuffers
		);
		ASSERT_VK(result, "Failed to allocate command buffers");
	}

	void Command::destroyCmdBuffers(
		const VkDevice							&_logicalDevice,
		const VkCommandPool					&_cmdPool,
		VkCommandBuffer							*_pCmdBuffers,
		uint32_t										_cmdCount
	) noexcept
	{
		vkFreeCommandBuffers(
			_logicalDevice,
			_cmdPool,
			_cmdCount,
			_pCmdBuffers
		);
	}

	void Command::recordCmdBuffer(
		const VkCommandBuffer															&_cmdBuffer,
		const std::function<void(const VkCommandBuffer&)>	&_recordCallback,
		const VkCommandBufferUsageFlags										&_usage
	) noexcept
	{
		VkCommandBufferBeginInfo beginInfo	= {};
		beginInfo.sType											= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext											= nullptr;
		beginInfo.pInheritanceInfo					= nullptr;
		beginInfo.flags											= _usage;

		auto result = vkBeginCommandBuffer(_cmdBuffer, &beginInfo);
		ASSERT_VK(result, "Failed to begin recording command buffer");

		_recordCallback(_cmdBuffer);

		result = vkEndCommandBuffer(_cmdBuffer);
		ASSERT_VK(result, "Failed to record command buffer");
	}

	void Command::submitQueue(
		const VkQueue								&_queue,
		const VkSubmitInfo					&_submitInfo,
		const std::string						&_queueName,
		const VkFence								&_fence
	) noexcept
	{
		auto result = vkQueueSubmit(
			_queue,
			1,
			&_submitInfo,
			_fence
		);
		ASSERT_VK(result, "Failed to submit" + (!_queueName.empty() ? " " + _queueName + " " : " ") + "command buffer queue!");
	}

	void Command::setViewport(
		const VkCommandBuffer	&_cmdBuffer,
		const VkExtent2D			&_extent,
		float _minDepth,			float _maxDepth
	) noexcept
	{
		VkViewport viewport = {
			0.0f, 0.0f,
			(float) _extent.width, (float) _extent.height,
			_minDepth, _maxDepth
		};
		vkCmdSetViewport(
			_cmdBuffer,
			0,
			1,
			&viewport
		);
	}

	void Command::setScissor(
		const VkCommandBuffer	&_cmdBuffer,
		const VkExtent2D			&_extent,
		int _offsetX,					int _offsetY
	) noexcept
	{
		VkRect2D scissor = {
			{ _offsetX, _offsetY },
			{ _extent.width, _extent.height }
		};
		vkCmdSetScissor(
			_cmdBuffer,
			0,
			1,
			&scissor
		);
	}

	void Command::setPushConstants() noexcept
	{}

	void Command::draw(
		const VkCommandBuffer			&_cmdBuffer,
		uint32_t									_vtxCount,
		uint32_t									_instanceCount
	) noexcept
	{
		vkCmdDraw(
			_cmdBuffer,
			_vtxCount,
			_instanceCount,
			0, 0
		);
	}

	void Command::drawIndexed(
		const VkCommandBuffer			&_cmdBuffer,
		uint32_t									_idxCount,
		uint32_t									_instanceCount,
		int 											_vtxOffset
	) noexcept
	{
		vkCmdDrawIndexed(
			_cmdBuffer,
			_idxCount,
			_instanceCount,
			0, _vtxOffset, 0
		);
	}

	void Command::bindPipeline(
		const VkCommandBuffer			&_cmdBuffer,
		const VkPipeline 					&_pipeline,
		const VkPipelineBindPoint &_bindPoint
	) noexcept
	{
		vkCmdBindPipeline(
			_cmdBuffer,
			_bindPoint,
			_pipeline
		);
	}

	void Command::bindDescSets(
		const VkCommandBuffer			&_cmdBuffer,
		const VkDescriptorSet			*_descriptorSets,
		uint32_t									_descSetCount,
		const uint32_t						*_pDynamicOffsets,
		uint32_t									_dynamicOffsetCount,
		const VkPipelineLayout		&_pipelineLayout,
		const VkPipelineBindPoint	&_bindPoint
	) noexcept
	{
		for(auto i = 0u; i < _descSetCount; i++)
		{
			vkCmdBindDescriptorSets(
				_cmdBuffer,
				_bindPoint, _pipelineLayout,
				i,
				_descSetCount, &_descriptorSets[i],
				_dynamicOffsetCount, _pDynamicOffsets
			);
		}
	}

	void Command::bindVtxBuffers(
		const VkCommandBuffer &_cmdBuffer,
		const VkBuffer				&_vtxBuffer,
		const VkDeviceSize 		&_offsets
	) noexcept
	{
		vkCmdBindVertexBuffers(
			_cmdBuffer,
			0, 1,
			&_vtxBuffer,
			&_offsets
		);
	}

	void Command::bindIdxBuffer(
		const VkCommandBuffer &_cmdBuffer,
		const VkBuffer				&_idxBuffer,
		const VkDeviceSize 		&_offset,
		const VkIndexType			&_indexType
	) noexcept
	{
		vkCmdBindIndexBuffer(
			_cmdBuffer,
			_idxBuffer,
			_offset,
			_indexType
		);
	}
}