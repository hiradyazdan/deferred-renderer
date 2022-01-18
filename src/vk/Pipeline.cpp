#include "vk/Pipeline.h"
#include "vk/Shader.h"

namespace vk
{
	void Pipeline::destroy(
		const VkDevice							&_logicalDevice,
		const VkPipeline						&_pipeline,
		const VkAllocationCallbacks	*_pAllocator
	) noexcept
	{
		vkDestroyPipeline(_logicalDevice, _pipeline, _pAllocator);
	}

	void Pipeline::destroyShader(
		const VkDevice							&_logicalDevice,
		const VkShaderModule				&_shaderModule,
		const VkAllocationCallbacks	*_pAllocator
	) noexcept
	{
		Shader::destroy(_logicalDevice, _shaderModule, _pAllocator);
	}

	void Pipeline::destroyLayout(
		const VkDevice							&_logicalDevice,
		const VkPipelineLayout			&_layout,
		const VkAllocationCallbacks	*_pAllocator
	) noexcept
	{
		vkDestroyPipelineLayout(_logicalDevice, _layout, _pAllocator);
	}

	void Pipeline::destroyCache(
		const VkDevice							&_logicalDevice,
		const VkPipelineCache				&_cache,
		const VkAllocationCallbacks	*_pAllocator
	) noexcept
	{
		vkDestroyPipelineCache(_logicalDevice, _cache, _pAllocator);
	}

	void Pipeline::createCache(
		const VkDevice	&_logicalDevice,
		VkPipelineCache	&_pipelineCache
	) noexcept
	{
		VkPipelineCacheCreateInfo cacheInfo = {};

		cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		auto result = vkCreatePipelineCache(
			_logicalDevice,
			&cacheInfo,
			nullptr,
			&_pipelineCache
		);
		ASSERT_VK(result, "Failed to create pipeline cache");
	}

	VkPipelineColorBlendAttachmentState Pipeline::setColorBlendAttachment(
		const VkColorComponentFlags &_colorWriteMask,
		VkBool32										_isBlendEnable
	) noexcept
	{
		return {
			_isBlendEnable,
			VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			_colorWriteMask
		};
	}
	
	void Pipeline::initPSOs(PSO &_psoData) noexcept
	{
		setDynamicState(_psoData);
		setVertexInputState(_psoData);
		setInputAssemblyState(_psoData);
		setRasterizationState(_psoData);
		setColorBlendState(_psoData);
		setViewportState(_psoData);
		setDepthStencilState(_psoData);
		setMultisampleState(_psoData);
		setTessellationState(_psoData);
	}

	void Pipeline::setDynamicState(PSO &_psoData) noexcept
	{
		auto &dynamicState = _psoData.dynamicState;
		auto &info = dynamicState.info;
		auto &dynamicStateEnables = dynamicState.dynamicStateEnables;

		info.sType							= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		info.dynamicStateCount	= dynamicStateEnables.size();
		info.pDynamicStates			= dynamicStateEnables.data();
		info.flags							= dynamicState.flags;
	}

	void Pipeline::setVertexInputState(PSO &_psoData) noexcept
	{
		auto &vertexInputState = _psoData.vertexInputState;
		auto &info = vertexInputState.info;
		auto &vertexBindingDescs = vertexInputState.vertexBindingDescs;
		auto &vertexAttrDescs = vertexInputState.vertexAttrDescs;

		info.sType														= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		info.vertexBindingDescriptionCount		= vertexBindingDescs.size();
		info.pVertexBindingDescriptions				= vertexBindingDescs.data();
		info.vertexAttributeDescriptionCount	= vertexAttrDescs.size();
		info.pVertexAttributeDescriptions			= vertexAttrDescs.data();
		info.pNext														= nullptr;
		info.flags														= vertexInputState.flags;
	}

	void Pipeline::setTessellationState(PSO &_psoData) noexcept
	{
		auto &tessellationState = _psoData.tessellationState;
		auto &info							= tessellationState.info;

		info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		info.patchControlPoints;
		info.pNext = nullptr;
		info.flags = tessellationState.flags;
	}

	void Pipeline::setInputAssemblyState(PSO &_psoData) noexcept
	{
		auto &inputAssemblyState = _psoData.inputAssemblyState;
		auto &info = inputAssemblyState.info;

		info.sType									= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		info.topology								= inputAssemblyState.topology;
		info.primitiveRestartEnable	= inputAssemblyState.primitiveRestartEnable;
		info.flags									= inputAssemblyState.flags;
	}

	void Pipeline::setRasterizationState(PSO &_psoData) noexcept
	{
		auto &rasterizationState = _psoData.rasterizationState;
		auto &info = rasterizationState.info;

		info.sType										= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		info.depthClampEnable					= VK_FALSE;
		info.rasterizerDiscardEnable	= VK_FALSE;
		info.polygonMode							= rasterizationState.polygonMode;
		info.cullMode									= rasterizationState.cullMode;
		info.frontFace								= rasterizationState.frontFace;
		info.depthBiasEnable					= VK_FALSE;
		info.depthBiasConstantFactor	= 0.0f;
		info.depthBiasClamp						= 0.0f;
		info.depthBiasSlopeFactor			= 0.0f;
		info.lineWidth								= 1.0f;
		info.flags										= rasterizationState.flags;
	}

	void Pipeline::setColorBlendState(PSO &_psoData) noexcept
	{
		auto &colorBlendState = _psoData.colorBlendState;
		auto &info = colorBlendState.info;
		auto &attachments = colorBlendState.attachments;

		info.sType						= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		info.logicOpEnable		= colorBlendState.logicOpEnable;
		info.attachmentCount	= attachments.size();
		info.pAttachments			= attachments.data();
	}

	void Pipeline::setViewportState(PSO &_psoData) noexcept
	{
		auto &viewportState = _psoData.viewportState;
		auto &info = viewportState.info;

		info.sType					= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		info.viewportCount	= viewportState.viewportCount;
		info.pViewports			= nullptr;
		info.scissorCount		= viewportState.scissorCount;
		info.pScissors			= nullptr;
		info.flags					= viewportState.flags;
	}

	void Pipeline::setDepthStencilState(PSO &_psoData) noexcept
	{
		auto &depthStencilState = _psoData.depthStencilState;
		auto &info = depthStencilState.info;

		info.sType						= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		info.depthTestEnable	= depthStencilState.depthTestEnable;
		info.depthWriteEnable	= depthStencilState.depthWriteEnable;
		info.depthCompareOp		= depthStencilState.depthCompareOp;
	}

	void Pipeline::setMultisampleState(PSO &_psoData) noexcept
	{
		auto &multisampleState = _psoData.multisampleState;
		auto &info = multisampleState.info;

		info.sType									= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		info.rasterizationSamples		= multisampleState.rasterizationSamples;
		info.sampleShadingEnable		= VK_FALSE;
		info.minSampleShading				= 1.0f;
		info.pSampleMask						= nullptr;
		info.alphaToCoverageEnable	= VK_FALSE;
		info.alphaToOneEnable				= VK_FALSE;
		info.flags									= multisampleState.flags;
	}
}