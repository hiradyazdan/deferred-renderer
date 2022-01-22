#pragma once

#include "Framebuffer.h"

namespace vk
{
//	class Sync;
	class Command
	{
		friend class Device;
		struct Data
		{
			VkCommandPool									cmdPool = VK_NULL_HANDLE;
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
				VkCommandBuffer							*_pCmdBuffers,
				uint32_t										_cmdCount = 1,
				const VkCommandBufferLevel	&_allocLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY
			) noexcept;

			static void reallocateCmdBuffers(

			) noexcept;

			static void destroyCmdBuffers(
				const VkDevice							&_logicalDevice,
				const VkCommandPool					&_cmdPool,
				VkCommandBuffer							*_pCmdBuffers,
				uint32_t										_cmdCount = 1
			) noexcept;

			static void destroyCmdPool(
				const VkDevice							&_logicalDevice,
				const VkCommandPool					&_cmdPool,
				const VkAllocationCallbacks	*_pAllocator = nullptr
			) noexcept;

			static void record(
				const VkCommandBuffer															&_cmdBuffer,
				const std::function<void(const VkCommandBuffer&)>	&_recordCallback,
				const VkCommandBufferUsageFlags										&_usage = 0
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

			static void submitToQueue(
				const VkQueue								&_queue,
				VkSubmitInfo								&_submitInfo,
				const std::string						&_batchName					= "",
				const VkFence								&_fence							= VK_NULL_HANDLE,
				const VkPipelineStageFlags	&_waitDstStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			) noexcept;

			// single time command
			static void submitStagingCopyCommand(
				const VkDevice			&_logicalDevice,
				VkCommandBuffer			&_cmdBuffer,
				const VkCommandPool	&_cmdPool,
				const VkQueue				&_queue,
				const std::string		&_batchName
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
			static void bindPipeline(
				const VkCommandBuffer			&_cmdBuffer,
				const VkPipeline					&_pipeline,
				const VkPipelineBindPoint &_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS
			) noexcept;
			static void bindDescSets(
				const VkCommandBuffer			&_cmdBuffer,
				const VkDescriptorSet			*_pDescriptorSets,
				uint32_t									_descSetCount,
				const uint32_t						*_pDynamicOffsets,
				uint32_t									_dynamicOffsetCount,
				const VkPipelineLayout		&_pipelineLayout,
				uint32_t									_firstSetIdx = 0,
				const VkPipelineBindPoint	&_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS
			)	noexcept;
			static void bindVtxBuffers(
				const VkCommandBuffer			&_cmdBuffer,
				const VkBuffer						&_vtxBuffer,
				const VkDeviceSize 				&_offsets
			) noexcept;
			static void bindIdxBuffer(
				const VkCommandBuffer 		&_cmdBuffer,
				const VkBuffer						&_idxBuffer,
				const VkDeviceSize 				&_offset,
				const VkIndexType					&_indexType = VK_INDEX_TYPE_UINT32
			) noexcept;

			static void setPushConstants(
				const VkCommandBuffer			&_cmdBuffer,
				const VkPipelineLayout		&_pipelineLayout,
				const void								*_pValues,
				uint32_t									_size,
				const VkShaderStageFlags	&_stageFlags	= VK_SHADER_STAGE_VERTEX_BIT,
				uint32_t									_offset				= 0
			)	noexcept;

			static void setPushConstants(
				const VkCommandBuffer			&_cmdBuffer,
				const VkPipelineLayout		&_pipelineLayout,
				const void								*_pValues,
				const VkPushConstantRange &_pcRange
			) noexcept;

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

			// sync action commands

			static void insertBarriers(
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
			) noexcept;

			template<typename TBarrierType>
			inline static void insertBarriers(
				const VkCommandBuffer							&_cmdBuffer,
				const TBarrierType								*_pBarriers,
				uint32_t													_barrierCount			= 1,
				const VkDependencyFlags						&_dependencyFlags	= 0,
				const VkPipelineStageFlagBits			&_srcStageMask		= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				const VkPipelineStageFlagBits			&_dstStageMask		= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
			) noexcept
			{
				const auto &isMem			= std::is_same<TBarrierType, VkMemoryBarrier>				::value;
				const auto &isBuffMem	= std::is_same<TBarrierType, VkBufferMemoryBarrier>	::value;
				const auto &isImgMem	= std::is_same<TBarrierType, VkImageMemoryBarrier>	::value;

				static_assert(
					isMem || isBuffMem || isImgMem,
					"Barrier Type should be a valid vulkan resource barrier."
				);

				const auto &memCount				= isMem			? _barrierCount : 0;
				const auto &buffMemCount		= isBuffMem	? _barrierCount : 0;
				const auto &imgMemCount			= isImgMem	? _barrierCount : 0;

				// @todo workout which assembly is more efficient

				const VkMemoryBarrier				*memBarrier			= nullptr;
				const VkBufferMemoryBarrier	*buffMemBarrier	= nullptr;
				const VkImageMemoryBarrier	*imgMemBarrier	= nullptr;

				if constexpr(isMem)			{ memBarrier			= _pBarriers; }
				if constexpr(isBuffMem)	{ buffMemBarrier	= _pBarriers; }
				if constexpr(isImgMem)	{ imgMemBarrier		= _pBarriers; }

//				const auto &memBarrier			= isMem			? reinterpret_cast<const VkMemoryBarrier*>			(_pBarriers) : nullptr;
//				const auto &buffMemBarrier	= isBuffMem	? reinterpret_cast<const VkBufferMemoryBarrier*>(_pBarriers) : nullptr;
//				const auto &imgMemBarrier		= isImgMem	? reinterpret_cast<const VkImageMemoryBarrier*>	(_pBarriers) : nullptr;

				insertBarriers(
					_cmdBuffer,
					_srcStageMask, _dstStageMask,
					_dependencyFlags,
					memCount,			memBarrier,
					buffMemCount, buffMemBarrier,
					imgMemCount,	imgMemBarrier
				);
			}
	};
}