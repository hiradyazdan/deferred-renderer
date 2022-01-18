#include "vk/Sync.h"
#include "vk/Command.h"

namespace vk
{
	void Command::createPool(
		const VkDevice	&_logicalDevice,
		uint32_t      	_queueFamilyIndex,
		VkCommandPool		&_cmdPool
	) noexcept
	{
		if(_cmdPool != VK_NULL_HANDLE) return;

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

	void Command::destroyCmdPool(
		const VkDevice							&_logicalDevice,
		const VkCommandPool					&_cmdPool,
		const VkAllocationCallbacks	*_pAllocator
	) noexcept
	{
		vkDestroyCommandPool(_logicalDevice, _cmdPool, _pAllocator);
	}

	void Command::record(
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

	void Command::submitToQueue(
		const VkQueue								&_queue,
		VkSubmitInfo								&_submitInfo,
		const std::string						&_batchName,
		const VkFence								&_fence,
		const VkPipelineStageFlags	&_waitDstStageMask
	) noexcept
	{
		_submitInfo.pWaitDstStageMask	= _submitInfo.waitSemaphoreCount > 0
			? &_waitDstStageMask
			: nullptr;

		auto result = vkQueueSubmit(
			_queue,
			1,
			&_submitInfo,
			_fence
		);
		ASSERT_VK(result, "Failed to submit" + (!_batchName.empty() ? " " + _batchName + " " : " ") + "command buffer(s) to the queue!");
	}

	void Command::submitStagingCopyCommand(
		const VkDevice			&_logicalDevice,
		VkCommandBuffer			&_cmdBuffer,
		const VkCommandPool	&_cmdPool,
		const VkQueue				&_queue,
		const std::string		&_batchName
	) noexcept
	{
		VkFence				fence;
		VkSubmitInfo	submitInfo = {};

		Sync::createFence		(_logicalDevice, fence);
		setSubmitInfo				(&_cmdBuffer, submitInfo);
		submitToQueue				(_queue, submitInfo, _batchName, fence);
		Sync::waitForFences	(_logicalDevice, &fence);

		Sync::destroyFence	(_logicalDevice, fence);
		destroyCmdBuffers		(_logicalDevice, _cmdPool, &_cmdBuffer);
	}

	void Command::insertBarriers(
		const VkCommandBuffer					&_cmdBuffer,
		const VkPipelineStageFlagBits	&_srcStageMask,
		const VkPipelineStageFlagBits	&_dstStageMask,
		const VkDependencyFlags				&_dependencyFlags,
		uint32_t											_memoryBarrierCount,
		const VkMemoryBarrier					*_pMemoryBarriers,
		uint32_t											_bufferMemoryBarrierCount,
		const VkBufferMemoryBarrier		*_pBufferMemoryBarriers,
		uint32_t											_imageMemoryBarrierCount,
		const VkImageMemoryBarrier		*_pImageMemoryBarriers
	) noexcept
	{
		vkCmdPipelineBarrier(
			_cmdBuffer, _srcStageMask, _dstStageMask,
			_dependencyFlags,
			_memoryBarrierCount, _pMemoryBarriers,
			_bufferMemoryBarrierCount, _pBufferMemoryBarriers,
			_imageMemoryBarrierCount, _pImageMemoryBarriers
		);
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
		int		_offsetX,				int _offsetY
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

	void Command::setPushConstants(
		const VkCommandBuffer			&_cmdBuffer,
		const VkPipelineLayout		&_pipelineLayout,
		const void								*_pValues,
		uint32_t									_size,
		const VkShaderStageFlags	&_stageFlags,
		uint32_t									_offset
	) noexcept
	{
		vkCmdPushConstants(
			_cmdBuffer, _pipelineLayout,
			_stageFlags, _offset,
			_size, _pValues
		);
	}

	void Command::setPushConstants(
		const VkCommandBuffer			&_cmdBuffer,
		const VkPipelineLayout		&_pipelineLayout,
		const void								*_pValues,
		const VkPushConstantRange &_pcRange
	) noexcept
	{
		vkCmdPushConstants(
			_cmdBuffer, _pipelineLayout,
			_pcRange.stageFlags, _pcRange.offset,
			_pcRange.size, _pValues
		);
	}

	void Command::draw(
		const VkCommandBuffer			&_cmdBuffer,
		uint32_t									_vtxCount,
		uint32_t									_firstVertex,
		uint32_t									_instanceCount,
		uint32_t									_firstInstance
	) noexcept
	{
		vkCmdDraw(
			_cmdBuffer,
			_vtxCount,
			_instanceCount,
			_firstVertex, _firstInstance
		);
	}

	void Command::drawIndexed(
		const VkCommandBuffer			&_cmdBuffer,
		uint32_t									_idxCount,
		uint32_t									_firstIndex,
		int 											_vtxOffset,
		uint32_t									_instanceCount,
		uint32_t									_firstInstance
	) noexcept
	{
		vkCmdDrawIndexed(
			_cmdBuffer,
			_idxCount,
			_instanceCount,
			_firstIndex,
			_vtxOffset,
			_firstInstance
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
		const VkDescriptorSet			*_pDescriptorSets,
		uint32_t									_descSetCount,
		const uint32_t						*_pDynamicOffsets,
		uint32_t									_dynamicOffsetCount,
		const VkPipelineLayout		&_pipelineLayout,
		uint32_t									_firstSetIdx,
		const VkPipelineBindPoint	&_bindPoint
	) noexcept
	{
		vkCmdBindDescriptorSets(
			_cmdBuffer,
			_bindPoint, _pipelineLayout,
			_firstSetIdx,
			_descSetCount, _pDescriptorSets,
			_dynamicOffsetCount, _pDynamicOffsets
		);
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