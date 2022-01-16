#include "vk/Device.h"
#include "vk/Model.h"

namespace vk
{
	void Model::drawNode(
		const VkCommandBuffer					&_cmdBuffer,
		const Vector<VkDescriptorSet>	&_descSets,
		const Vector<VkPipeline>			&_pipelines,
		const VkPipelineLayout				&_pipelineLayout,
		const NodePtr									&_node,
		uint16_t											_matSetFirstIdx,
		uint16_t											_matPipeFirstIdx
	) noexcept
	{
		const auto &mesh	= _node->mesh;
		auto nodeMtx			= _node->matrix;
		auto curParent		= _node->parent;

		while(curParent)
		{
			nodeMtx		= curParent->matrix * nodeMtx;
			curParent	= curParent->parent;
		}

		Command::setPushConstants(
			_cmdBuffer, _pipelineLayout,
			&nodeMtx, sizeof(Node::matrix) // glm::mat4
		);

		for(const auto &primitive : mesh.primitives)
		{
			const auto &primIdxParams = primitive.indexParams;
			const auto &matSet = _descSets[_matSetFirstIdx + primitive.matIndex];
			const auto &matPipeline = _pipelines[_matPipeFirstIdx + primitive.matIndex];

			if(primitive.idxCount == 0) { continue; }

//			DEBUG_LOG("idxCount: %d\nfirstIndex: %d", primitive.idxCount, primIdxParams.firstIndex);
			Command::bindPipeline(_cmdBuffer, matPipeline);

			Command::bindDescSets(
				_cmdBuffer,
				&matSet, 1,
				nullptr, 0,
				_pipelineLayout
			);

			Command::drawIndexed(
				_cmdBuffer,
				primitive.idxCount,
				primIdxParams.firstIndex,			primIdxParams.vtxOffset,
				primIdxParams.instanceCount,	primIdxParams.firstInstance
			);
		}

		for(const auto &child : _node->children)
		{
			drawNode(
				_cmdBuffer, _descSets,
				_pipelines, _pipelineLayout,
				child, _matSetFirstIdx, _matPipeFirstIdx
			);
		}
	}

	void Model::cleanUpBuffers(
		const VkDevice																	&_logicalDevice,
		const Array<VkBuffer,				Buffer::s_mbtCount>	&_buffers,
		const Array<VkDeviceMemory,	Buffer::s_mbtCount>	&_memories
	) noexcept
	{
		for(auto i = 0u; i < Buffer::s_mbtCount; ++i)
		{
			Buffer::destroy		(_logicalDevice, _buffers[i]);
			Device::freeMemory(_logicalDevice, _memories[i]);
		}
	}
}