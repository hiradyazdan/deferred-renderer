#include "Model.h"

void Model::load(
	const std::string &_fileName
) noexcept
{
	AssetHelper assetHelper;

	assetHelper.load(constants::MODELS_PATH + _fileName, m_data.indices, m_data.vertices, m_data.meshes);
}

void Model::draw(
	const VkCommandBuffer		&_cmdBuffer,
	const VkPipeline				&_pipeline,
	const VkPipelineLayout	&_pipelineLayout
) noexcept
{
	vk::Command::bindPipeline(
		_cmdBuffer,
		_pipeline
	);

	for(const auto &mesh : m_data.meshes)
	{
		auto &descSets = mesh.material->descriptorSets;

		vk::Command::bindDescSets(
			_cmdBuffer,
			descSets.data(),
			descSets.size(),
			nullptr, 0,
			_pipelineLayout
		);
		vk::Command::bindVtxBuffers(
			_cmdBuffer, mesh.vertexBuffer, 0
		);
		vk::Command::bindIdxBuffers(
			_cmdBuffer, mesh.indexBuffer, 0
		);
		vk::Command::drawIndexed(
			_cmdBuffer,
			mesh.indexCount,
			1
		);
	}
}