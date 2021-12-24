#pragma once

#include "utils.h"
#include "vk/RenderPass.h"

namespace vk
{
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
				VkCommandBuffer							*_cmdBuffers,
				uint32_t										_bufferCount,
				const VkCommandBufferLevel	&_allocLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY
			) noexcept;

			static void recordCmdBuffer(
				const VkCommandBuffer															&_cmdBuffer,
				const std::function<void(const VkCommandBuffer&)>	&_recordCallback,
				const VkCommandBufferUsageFlags										&_usage = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
			) noexcept;

			static void recordRenderPassCommands(
				const VkCommandBuffer															&_cmdBuffer,
				const RenderPass::Data::BeginInfo									&_beginInfo,
				const std::function<void(const VkCommandBuffer&)>	&_cmdRenderPassCallback
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
				uint32_t									_instanceCount
			) noexcept;
			static void drawIndexed(
				const VkCommandBuffer			&_cmdBuffer,
				uint32_t									_idxCount,
				uint32_t									_instanceCount,
				int												_vtxOffset = 0
			) noexcept;

			static void bindPipeline(
				const VkCommandBuffer			&_cmdBuffer,
				const VkPipeline					&_pipeline,
				const VkPipelineBindPoint &_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS
			) noexcept;
			static void bindDescSets(
				const VkCommandBuffer								&_cmdBuffer,
				const std::vector<VkDescriptorSet>	&_descriptorSets,
				const uint32_t											*_dynamicOffsets,
				uint32_t														_dynamicOffsetCount,
				const VkPipelineLayout							&_pipelineLayout,
				const VkPipelineBindPoint						&_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS
			)	noexcept;
			static void bindVtxBuffers(
				const VkCommandBuffer &_cmdBuffer,
				const VkBuffer				&_vtxBuffer,
				const VkDeviceSize 		&_offsets
			) noexcept;
			static void bindIdxBuffers(
				const VkCommandBuffer &_cmdBuffer,
				const VkBuffer				&_idxBuffer,
				const VkDeviceSize 		&_offset,
				const VkIndexType			&_indexType = VK_INDEX_TYPE_UINT32
			) noexcept;
	};
}