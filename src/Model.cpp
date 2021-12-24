#include "Model.h"
#include "_constants.h"

void Model::load(
	const std::string &_fileName
) noexcept
{
	AssetHelper assetHelper;

	const auto &filePath = constants::ASSET_PATH + "models/" + _fileName;

	assetHelper.load(filePath, m_data.indices, m_data.vertices, m_data.meshes);
}

void Model::draw(
	const VkCommandBuffer	&_cmdBuffer
) noexcept
{
	const auto &pipelineData = m_data.pipelineData;

	vk::Command::bindPipeline(
		_cmdBuffer,
		pipelineData.pipeline
	);

	for(const auto &mesh : m_data.meshes)
	{
		vk::Command::bindDescSets(
			_cmdBuffer,
			mesh.material->descriptorSets,
			nullptr, 0,
			pipelineData.pipelineLayout
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