#pragma once

#include "utils.h"

namespace vk
{
	class Pipeline
	{
		public:
			static const uint16_t s_layoutCount;
			static const uint16_t s_pushConstCount;

		public:
			enum class Type : uint16_t;

			struct PipeLayoutDescSetsMap
			{
				STACK_ONLY(PipeLayoutDescSetsMap);

				uint16_t							layoutIndex				= 0;
				std::vector<uint16_t>	setsIndices				= { 0 };
				std::vector<uint16_t>	pushConstIndices	= {};
			};

			template<
				uint16_t shaderModuleCount,
				uint16_t pipelineLayoutCount	= 1,
				uint16_t pushConstCount				= 0
			>
			struct Data
			{
				Data() { ASSERT_ENUMS(Type); }

				Vector<VkPipeline>																	pipelines;
				Array<VkPipelineLayout,				pipelineLayoutCount>	layouts;
				Array<VkShaderModule,					shaderModuleCount>		shaderModules;
				Array<VkPushConstantRange,		pushConstCount>				pushConstRanges;
				Array<PipeLayoutDescSetsMap,	pipelineLayoutCount>	pipeLayoutDescSets;

				VkPipelineCache																		cache = VK_NULL_HANDLE;
			};

			struct PSO
			{
				STACK_ONLY(PSO);

				struct
				{
					friend class Pipeline;

					VkPrimitiveTopology																topology								= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
					VkBool32																					primitiveRestartEnable	= VK_FALSE;
					VkPipelineInputAssemblyStateCreateFlags 					flags										= 0;

					private:
						VkPipelineInputAssemblyStateCreateInfo					info = {};
				} inputAssemblyState;

				struct
				{
					friend class Pipeline;

					VkPolygonMode																			polygonMode	= VK_POLYGON_MODE_FILL;
					VkCullModeFlags																		cullMode		= VK_CULL_MODE_BACK_BIT;
					VkFrontFace																				frontFace		= VK_FRONT_FACE_COUNTER_CLOCKWISE;
					VkPipelineRasterizationStateCreateFlags 					flags				= 0;

					private:
						VkPipelineRasterizationStateCreateInfo					info = {};
				} rasterizationState;

				struct
				{
					friend class Pipeline;

					std::vector<VkPipelineColorBlendAttachmentState>	attachments 	= {
						setColorBlendAttachment() // Framebuffer (Default)
					};
					VkBool32																					logicOpEnable	= VK_FALSE;

					private:
						VkPipelineColorBlendStateCreateInfo							info = {};
				} colorBlendState;

				struct
				{
					friend class Pipeline;

					VkBool32																					depthTestEnable		= VK_TRUE;
					VkBool32																					depthWriteEnable	= VK_TRUE;
					VkCompareOp																				depthCompareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;

					private:
						VkPipelineDepthStencilStateCreateInfo						info = {};
				} depthStencilState;

				struct
				{
					friend class Pipeline;

					uint32_t 																					viewportCount = 1;
					uint32_t 																					scissorCount	= 1;
					VkPipelineViewportStateCreateFlags								flags					= 0;

					private:
						VkPipelineViewportStateCreateInfo								info = {};
				} viewportState;

				struct
				{
					friend class Pipeline;

					VkSampleCountFlagBits															rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
					VkPipelineMultisampleStateCreateFlags							flags = 0;

					private:
						VkPipelineMultisampleStateCreateInfo						info = {};
				} multisampleState;

				struct
				{
					friend class Pipeline;

					std::vector<VkDynamicState>              					dynamicStateEnables = {
						VK_DYNAMIC_STATE_VIEWPORT,
						VK_DYNAMIC_STATE_SCISSOR
					};
					VkPipelineDynamicStateCreateFlags									flags = 0;

					private:
						VkPipelineDynamicStateCreateInfo								info = {};
				} dynamicState;

				struct
				{
					friend class Pipeline;

					std::vector<VkVertexInputBindingDescription>      vertexBindingDescs;
					std::vector<VkVertexInputAttributeDescription>    vertexAttrDescs;
					VkPipelineVertexInputStateCreateFlags							flags = 0;

					private:
						VkPipelineVertexInputStateCreateInfo						info = {};
				} vertexInputState;

				struct
				{
					friend class Pipeline;

					VkPipelineTessellationStateCreateFlags 						flags = 0;

					private:
						VkPipelineTessellationStateCreateInfo						info = {};
				} tessellationState;
			};

		public:
			static void createCache(
				const VkDevice	&_logicalDevice,
				VkPipelineCache	&_pipelineCache
			) noexcept;

			template<size_t descSetLayoutCount, size_t pushConstRangeCount, size_t pipelineLayoutCount>
			static void createLayout(
				const VkDevice																						&_logicalDevice,
				const Array<VkPushConstantRange,		pushConstRangeCount>	&_pushConstantRanges,
				const Array<VkDescriptorSetLayout,	descSetLayoutCount>		&_descSetLayouts,
				Array<PipeLayoutDescSetsMap,				pipelineLayoutCount>	&_pipeLayoutDescSets,
				Array<VkPipelineLayout,							pipelineLayoutCount>	&_layouts,
				uint16_t																									_layoutIndex = 0
			) noexcept
			{
				VkPipelineLayoutCreateInfo layoutInfo = {};
				layoutInfo.pushConstantRangeCount			= pushConstRangeCount;
				layoutInfo.pPushConstantRanges				= _pushConstantRanges.data();

				_pipeLayoutDescSets[_layoutIndex].layoutIndex = _layoutIndex;

				for(auto i = 0; i < pushConstRangeCount; ++i)
				{
//					_pipeLayoutDescSets[_layoutIndex].pushConstIndices[i] = i;
				}

				createLayout(_logicalDevice, _descSetLayouts, layoutInfo, _layouts[_layoutIndex]);
			}

			template<size_t descSetLayoutCount, size_t pipelineLayoutCount>
			static void createLayout(
				const VkDevice																						&_logicalDevice,
				const Array<VkDescriptorSetLayout,	descSetLayoutCount>		&_descSetLayouts,
				Array<PipeLayoutDescSetsMap,				pipelineLayoutCount>	&_pipeLayoutDescSets,
				Array<VkPipelineLayout,							pipelineLayoutCount>	&_layouts,
				uint16_t																									_layoutIndex = 0
			) noexcept
			{
				VkPipelineLayoutCreateInfo layoutInfo = {};
				layoutInfo.pushConstantRangeCount			= 0;
				layoutInfo.pPushConstantRanges				= nullptr;

				_pipeLayoutDescSets[_layoutIndex].layoutIndex = _layoutIndex;

				createLayout(_logicalDevice, _descSetLayouts, layoutInfo, _layouts[_layoutIndex]);
			}

			template<size_t shaderStageCount>
			static void createGraphicsPipeline(
				const VkDevice																									&_logicalDevice,
				const VkRenderPass																							&_renderPass,
				const VkPipelineCache																						&_cache,
				const VkPipelineLayout																					&_layout,
				const Array<VkPipelineShaderStageCreateInfo, shaderStageCount>	&_shaderStages,
				PSO																															&_psoData,
				VkPipeline																											&_pipeline
			) noexcept
			{
				initPSOs(_psoData);

				VkGraphicsPipelineCreateInfo pipelineInfo = {};
				pipelineInfo.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipelineInfo.stageCount           = static_cast<uint32_t>(_shaderStages.size());
				pipelineInfo.pStages              = _shaderStages.data();

				pipelineInfo.pDynamicState        = &_psoData.dynamicState.info;
				pipelineInfo.pVertexInputState    = &_psoData.vertexInputState.info;
				pipelineInfo.pInputAssemblyState  = &_psoData.inputAssemblyState.info;
				pipelineInfo.pRasterizationState  = &_psoData.rasterizationState.info;
				pipelineInfo.pColorBlendState     = &_psoData.colorBlendState.info;
				pipelineInfo.pViewportState       = &_psoData.viewportState.info;
				pipelineInfo.pDepthStencilState   = &_psoData.depthStencilState.info;
				pipelineInfo.pMultisampleState    = &_psoData.multisampleState.info;
				pipelineInfo.pTessellationState		= &_psoData.tessellationState.info;

				pipelineInfo.layout               = _layout;
				pipelineInfo.renderPass           = _renderPass;

				auto result = vkCreateGraphicsPipelines(
					_logicalDevice,
					_cache,
					1,
					&pipelineInfo,
					nullptr,
					&_pipeline
				);
				ASSERT_VK(result, "Failed to create graphics pipeline");
			}

			template<
			  uint16_t shaderModuleCount,
				uint16_t pipelineLayoutCount,
				uint16_t pushConstCount
			>
			inline static void destroy(
				const VkDevice																											&_logicalDevice,
				const Data<shaderModuleCount, pipelineLayoutCount, pushConstCount>	&_data,
				const VkAllocationCallbacks																					*_pAllocator = nullptr
			) noexcept
			{
				for(const auto &pipeline : _data.pipelines)
				{
					destroy(_logicalDevice, pipeline, _pAllocator);
				}
				for(const auto &layout : _data.layouts)
				{
					destroyLayout(_logicalDevice, layout, _pAllocator);
				}
				for(const auto &module : _data.shaderModules)
				{
					destroyShader(_logicalDevice, module, _pAllocator);
				}

				destroyCache(_logicalDevice, _data.cache, _pAllocator);
			}

			static void destroy(
				const VkDevice							&_logicalDevice,
				const VkPipeline						&_pipeline,
				const VkAllocationCallbacks	*_pAllocator = nullptr
			) noexcept;

			static void destroyLayout(
				const VkDevice							&_logicalDevice,
				const VkPipelineLayout			&_layout,
				const VkAllocationCallbacks	*_pAllocator = nullptr
			) noexcept;

			static void destroyCache(
				const VkDevice							&_logicalDevice,
				const VkPipelineCache				&_cache,
				const VkAllocationCallbacks	*_pAllocator = nullptr
			) noexcept;

			static VkPipelineColorBlendAttachmentState setColorBlendAttachment(
				const VkColorComponentFlags &_colorWriteMask	= VK_COLOR_COMPONENT_R_BIT |
																												VK_COLOR_COMPONENT_G_BIT |
																												VK_COLOR_COMPONENT_B_BIT |
																												VK_COLOR_COMPONENT_A_BIT,
				VkBool32										_isBlendEnable		= VK_FALSE
			) noexcept;

		private:
			template<size_t descSetLayoutCount>
			static void createLayout(
				const VkDevice																					&_logicalDevice,
				const Array<VkDescriptorSetLayout, descSetLayoutCount>	&_descSetLayouts,
				VkPipelineLayoutCreateInfo															&_layoutInfo,
				VkPipelineLayout																				&_pipelineLayout
			) noexcept
			{
				_layoutInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				_layoutInfo.setLayoutCount	= static_cast<uint32_t>(_descSetLayouts.size());
				_layoutInfo.pSetLayouts			= _descSetLayouts.data();

				const auto &result = vkCreatePipelineLayout(
					_logicalDevice,
					&_layoutInfo,
					nullptr,
					&_pipelineLayout
				);
				ASSERT_VK(result, "Failed to create pipeline layout");
			}

			static void destroyShader(
				const VkDevice							&_logicalDevice,
				const VkShaderModule				&_shaderModule,
				const VkAllocationCallbacks	*_pAllocator = nullptr
			) noexcept;

		private:
			static void initPSOs							(PSO &_psoData) noexcept;

			static void setDynamicState				(PSO &_psoData) noexcept;
			static void setVertexInputState   (PSO &_psoData) noexcept;
			static void setInputAssemblyState	(PSO &_psoData) noexcept;
			static void setRasterizationState (PSO &_psoData) noexcept;
			static void setColorBlendState    (PSO &_psoData) noexcept;
			static void setViewportState      (PSO &_psoData) noexcept;
			static void setDepthStencilState  (PSO &_psoData) noexcept;
			static void setMultisampleState   (PSO &_psoData) noexcept;
			static void setTessellationState	(PSO &_psoData) noexcept;
	};
}