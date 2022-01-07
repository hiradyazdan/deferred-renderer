#pragma once

#include "macros.h"
#include "vk.h"

namespace tinygltf
{
	class 	TinyGLTF;
	class 	Model;
	class		Node;

	struct	Image;

	typedef bool (*LoadImageDataFunction)(
		Image *, const int, std::string *,
		std::string *, int, int,
		const unsigned char *, int,
		void *
	);
}

class AssetHelper
{
	using gltfLoadImgDataFunc = tinygltf::LoadImageDataFunction;

	using gltfModel		= tinygltf::Model;
	using gltfNode		= tinygltf::Node;
	using gltfLoader	= tinygltf::TinyGLTF;
	using gltfImage		= tinygltf::Image;

	using Mesh				= vk::Mesh;
	using Texture			= vk::Texture;
	using MaterialPtr	= std::shared_ptr<vk::Material>;

	STACK_ONLY(AssetHelper);

	public:
		enum class FileLoadingFlags : uint16_t
		{
			NO_IMAGE_LOAD,
			FLIP_Y,
			PRE_MULTIPLY_VTX_COLORS,
			PRE_TRANSFORM_VTX,
			NONE
		};

		// @todo: Workout the edge cases
		friend FileLoadingFlags operator|(FileLoadingFlags _lhs, FileLoadingFlags _rhs)
		{
			using LoadingFlagsType = std::underlying_type<FileLoadingFlags>::type;
			return FileLoadingFlags(static_cast<LoadingFlagsType>(_lhs) | static_cast<LoadingFlagsType>(_rhs));
		}

		// @todo: Workout the edge cases
		friend FileLoadingFlags operator&(FileLoadingFlags _lhs, FileLoadingFlags _rhs)
		{
			using LoadingFlagsType = std::underlying_type<FileLoadingFlags>::type;
			return FileLoadingFlags(static_cast<LoadingFlagsType>(_lhs) & static_cast<LoadingFlagsType>(_rhs));
		}

	public:
		void load(
			const std::string				&_fileName,
			std::vector<Mesh>				&_meshes,
			const FileLoadingFlags	&_loadingFlags = FileLoadingFlags::NONE,
			float 									_scale = 1.0f
		) noexcept;

	private:
		static void setImageLoader(
			bool				_isNoImageLoad,
			gltfLoader	&_loader
		) noexcept;
		static void loadModel(
			const std::string	&_fileName,
			bool							_isNoImageLoad,
			gltfModel					&_model
		) noexcept;

	private:
		void loadImages		(const gltfModel &_model, bool _isNoImageLoad = false)	noexcept;
		void loadMaterials(const gltfModel &_model)	noexcept;
		void loadTextures	(const gltfModel &_model)	noexcept;
		void loadNode			(
			const gltfNode						&_node,
			const gltfModel						&_model,
			std::vector<uint32_t>			&_indices,
			std::vector<Mesh::Vertex> &_vertices
		) noexcept;

	private:
		std::vector<MaterialPtr>	m_materials;
};