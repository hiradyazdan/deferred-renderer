#include "Model.h"
#include "_constants.h"

void Model::load(
	const std::string &_fileName
) noexcept
{
	AssetHelper assetHelper;

	const auto &filePath = constants::ASSET_PATH + "models/" + _fileName;

	std::vector<uint32_t> indexBuffer;
	std::vector<AssetHelper::Vertex> vertexBuffer;

	assetHelper.load(filePath, indexBuffer, vertexBuffer);
}

void Model::draw() noexcept
{

}