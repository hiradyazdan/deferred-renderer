#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include "macros.h"
#include "vk.h"

class AssetHelper
{
	using MaterialPtr = std::shared_ptr<vk::Material>;

	public:
		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;
			glm::vec3 texCoord;
			glm::vec3 normal;
		};

	public:
		void load(
			const std::string &_fileName,
			std::vector<uint32_t> &_indices,
			std::vector<Vertex> &_vertices,
			std::vector<vk::Mesh> &_meshes
		) noexcept;

	private:
		void loadMeshes(std::vector<vk::Mesh> &_meshes) noexcept;
		void loadMaterials() noexcept;

	private:
		const aiScene 						*m_scene;

		std::vector<MaterialPtr>	m_materials;
};