#include "AssetHelper.h"

void AssetHelper::load(
	const std::string &_fileName,
	std::vector<uint32_t> &_indices,
	std::vector<Vertex> &_vertices,
	std::vector<vk::Mesh> &_meshes
) noexcept
{
	Assimp::Importer importer;

	m_scene = importer.ReadFile(
		_fileName.c_str(),
		aiProcess_FlipWindingOrder			|
		aiProcess_Triangulate						|
		aiProcess_PreTransformVertices	|
		aiProcess_CalcTangentSpace			|
		aiProcess_GenSmoothNormals
	);

	if(m_scene)
	{
		loadMaterials();
		loadMeshes(_meshes);
	}
	else
	{
		ERROR_LOG("Failed to parse %s: %s", _fileName.c_str(), importer.GetErrorString());
	}
}

void AssetHelper::loadMaterials() noexcept
{
	m_materials.resize(m_scene->mNumMaterials);


}

void AssetHelper::loadMeshes(std::vector<vk::Mesh> &_meshes) noexcept
{
	_meshes.resize(m_scene->mNumMeshes);

	const auto meshListSize = _meshes.size();

	for (auto i = 0u; i < meshListSize; i++)
	{
		auto mesh = m_scene->mMeshes[i];

		_meshes[i].material = m_materials[mesh->mMaterialIndex];
	}
}