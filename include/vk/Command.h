#pragma once

#include "Framebuffer.h"

namespace vk
{
	class Command
	{
		friend class Device;
		struct Data
		{
			VkCommandPool									cmdPool = VK_NULL_HANDLE;
			std::vector<VkCommandBuffer>	drawCmdBuffers;

			VkSubmitInfo									submitInfo = {};
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
				VkCommandBuffer							*_pCmdBuffers,
				uint32_t										_cmdCount = 1,
				const VkCommandBufferLevel	&_allocLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY
			) noexcept;

			static void destroyCmdBuffers(
				const VkDevice							&_logicalDevice,
				const VkCommandPool					&_cmdPool,
				VkCommandBuffer							*_pCmdBuffers,
				uint32_t										_cmdCount = 1
			) noexcept;

			static void recordCmdBuffer(
				const VkCommandBuffer															&_cmdBuffer,
				const std::function<void(const VkCommandBuffer&)>	&_recordCallback,
				const VkCommandBufferUsageFlags										&_usage = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
			) noexcept;

			template<uint16_t fbAttCount>
			static void recordRenderPassCommands(
				const VkCommandBuffer															&_cmdBuffer,
				const VkExtent2D																	&_swapchainExtent,
				const Framebuffer::Data<fbAttCount>								&_framebufferData,
				const std::function<void(const VkCommandBuffer&)>	&_cmdRenderPassCallback
			) noexcept
			{
				const auto &attachments = _framebufferData.attachments;
				const auto &clearValues = attachments.clearValues;

				VkRenderPassBeginInfo beginInfo = {};
				beginInfo.sType									= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				beginInfo.renderPass						= _framebufferData.renderPass;
				beginInfo.framebuffer						= _framebufferData.framebuffer;
				beginInfo.renderArea.offset			= { 0, 0 };
				beginInfo.renderArea.extent			= _swapchainExtent;
				beginInfo.clearValueCount				= clearValues.size();
				beginInfo.pClearValues					= clearValues.data();

				vkCmdBeginRenderPass(
					_cmdBuffer,
					&beginInfo,
					VK_SUBPASS_CONTENTS_INLINE
				);

				_cmdRenderPassCallback(_cmdBuffer);

				vkCmdEndRenderPass(_cmdBuffer);
			}

			template<uint16_t regionCount = 1>
			static void copyBuffer(
				const VkCommandBuffer	&_cmdBuffer,
				const VkBuffer				&_srcBuffer,
				const VkBuffer				&_dstBuffer,
				const VkBufferCopy		*_regions
			) noexcept
			{
				vkCmdCopyBuffer(
					_cmdBuffer,
					_srcBuffer, _dstBuffer,
					regionCount, _regions
				);
			}

			template<uint16_t waitCount = 1, uint16_t signalCount = 1, uint16_t cmdCount = 1>
			static void setSubmitInfo(
				const VkSemaphore						*_pWaitSemaphores,
				const VkSemaphore 					*_pSignalSemaphores,
				const VkCommandBuffer				*_pCmdBuffers,
				VkSubmitInfo								&_submitInfo
			) noexcept
			{
				_submitInfo.sType									= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				_submitInfo.pNext                	= nullptr;
				_submitInfo.waitSemaphoreCount		= waitCount;
				_submitInfo.pWaitSemaphores				= _pWaitSemaphores;
				_submitInfo.signalSemaphoreCount	= signalCount;
				_submitInfo.pSignalSemaphores			= _pSignalSemaphores;
				_submitInfo.commandBufferCount		= cmdCount;
				_submitInfo.pCommandBuffers				= _pCmdBuffers;
			}

			template<uint16_t waitCount = 1, uint16_t signalCount = 1>
			static void setSubmitInfo(
				const VkSemaphore						*_pWaitSemaphores,
				const VkSemaphore 					*_pSignalSemaphores,
				VkSubmitInfo								&_submitInfo
			) noexcept
			{
				_submitInfo.sType									= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				_submitInfo.pNext                	= nullptr;
				_submitInfo.waitSemaphoreCount		= waitCount;
				_submitInfo.pWaitSemaphores				= _pWaitSemaphores;
				_submitInfo.signalSemaphoreCount	= signalCount;
				_submitInfo.pSignalSemaphores			= _pSignalSemaphores;
			}

			template<uint16_t cmdCount = 1>
			static void setSubmitInfo(
				const VkCommandBuffer				*_pCmdBuffers,
				VkSubmitInfo								&_submitInfo
			) noexcept
			{
				_submitInfo.sType									= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				_submitInfo.pNext                	= nullptr;
				_submitInfo.commandBufferCount		= cmdCount;
				_submitInfo.pCommandBuffers				= _pCmdBuffers;
			}

			static void submitQueue(
				const VkQueue								&_queue,
				VkSubmitInfo								&_submitInfo,
				const std::string						&_queueName					= "",
				const VkFence								&_fence							= VK_NULL_HANDLE,
				const VkPipelineStageFlags	&_waitDstStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			) noexcept;

			// state commands

			static void setViewport(
				const VkCommandBuffer		&_cmdBuffer,
				const VkExtent2D				&_extent,
				float _minDepth = 0.f,	float _maxDepth = 1.f
			) noexcept;
			static void setScissor(
				const VkCommandBuffer		&_cmdBuffer,
				const VkExtent2D				&_extent,
				int		_offsetX = 0,			int _offsetY = 0
			) noexcept;
			static void setPushConstants()	noexcept;

			// action commands

			static void draw(
				const VkCommandBuffer			&_cmdBuffer,
				uint32_t									_vtxCount,
				uint32_t									_firstVertex		= 0,
				uint32_t									_instanceCount	= 1,
				uint32_t									_firstInstance	= 0
			) noexcept;
			static void drawIndexed(
				const VkCommandBuffer			&_cmdBuffer,
				uint32_t									_idxCount,
				uint32_t									_firstIndex			= 0,
				int												_vtxOffset			= 0,
				uint32_t									_instanceCount	= 1,
				uint32_t									_firstInstance	= 0
			) noexcept;

			static void bindPipeline(
				const VkCommandBuffer			&_cmdBuffer,
				const VkPipeline					&_pipeline,
				const VkPipelineBindPoint &_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS
			) noexcept;
			static void bindDescSet(
				const VkCommandBuffer			&_cmdBuffer,
				const VkPipelineBindPoint	&_bindPoint,
				const VkPipelineLayout		&_pipelineLayout,
				uint32_t									_firstSet,
				uint32_t									_descSetCount,
				const VkDescriptorSet			*_descriptorSets,
				uint32_t									_dynamicOffsetCount,
				const uint32_t						*_pDynamicOffsets
			) noexcept;
			static void bindDescSets(
				const VkCommandBuffer			&_cmdBuffer,
				const VkDescriptorSet			*_descriptorSets,
				uint32_t									_descSetCount,
				const uint32_t						*_pDynamicOffsets,
				uint32_t									_dynamicOffsetCount,
				const VkPipelineLayout		&_pipelineLayout,
				uint32_t									_setsStartIndex = 0,
				const VkPipelineBindPoint	&_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS
			)	noexcept;
			static void bindVtxBuffers(
				const VkCommandBuffer &_cmdBuffer,
				const VkBuffer				&_vtxBuffer,
				const VkDeviceSize 		&_offsets
			) noexcept;
			static void bindIdxBuffer(
				const VkCommandBuffer &_cmdBuffer,
				const VkBuffer				&_idxBuffer,
				const VkDeviceSize 		&_offset,
				const VkIndexType			&_indexType = VK_INDEX_TYPE_UINT32
			) noexcept;
	};
}