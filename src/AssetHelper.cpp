#include "AssetHelper.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
//#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif

#include <tiny_gltf.h>

void AssetHelper::load(
	const std::string				&_fileName,
	std::vector<Mesh>				&_meshes,
	const FileLoadingFlags	&_loadingFlags,
	float 									_scale
) noexcept
{
	gltfModel	model;

	auto isNoImageLoad = bool(_loadingFlags & FileLoadingFlags::NO_IMAGE_LOAD);

	loadModel(_fileName, isNoImageLoad, model);

	loadImages		(model);
	loadMaterials	(model);
	loadTextures	(model);

	const auto &scene = model.scenes[0];
	auto &nodes = scene.nodes;

	for(const auto &node : nodes)
	{
		const auto &modelNode = model.nodes[node];
		auto &mesh = _meshes[node];

		loadNode(modelNode, model, mesh.indices, mesh.vertices);
	}
}

void AssetHelper::setImageLoader(
	bool				_isNoImageLoad,
	gltfLoader	&_loader
) noexcept
{
	gltfLoadImgDataFunc loadImgDataFuncEmpty = [](
		gltfImage*, const int, std::string*, std::string*,
		int, int, const unsigned char*, int, void*
	) { return true; };
	gltfLoadImgDataFunc loadImageDataFunc = [](
		gltfImage* _img, const int _imgIdx, std::string* _err, std::string* _warn,
		int _width, int _height, const unsigned char* _bytes, int _size, void* _data
	)
	{
		return Texture::isKtx(_img->uri)
		? true
		: tinygltf::LoadImageData(
			_img, _imgIdx, _err, _warn,
			_width, _height, _bytes, _size, _data
		);
	};

	_loader.SetImageLoader(
		_isNoImageLoad ? loadImgDataFuncEmpty : loadImageDataFunc,
		nullptr
	);
}

void AssetHelper::loadModel(
	const std::string	&_fileName,
	bool							_isNoImageLoad,
	gltfModel					&_model
) noexcept
{
	gltfLoader	loader;
	std::string	err, warn;

	setImageLoader(_isNoImageLoad, loader);

#if defined(__ANDROID__)
	tinygltf::asset_manager = androidApp->activity->assetManager;
#endif

	auto result = loader.LoadASCIIFromFile(&_model, &err, &warn, _fileName);

	if(!warn.empty())
	{ WARN_LOG("Warning @%s: %s", _fileName.c_str(), warn.c_str()); }

	if(!result)
	{
		auto error = !err.empty() ? err : "UNKNOWN ERROR";

		FATAL_ERROR_LOG("Failed to load %s: %s", _fileName.c_str(), error.c_str());
	}
}

void AssetHelper::loadImages(const gltfModel &_model, bool _isNoImageLoad) noexcept
{
	if(_isNoImageLoad) return;

	for(auto &img : _model.images)
	{
//		Texture::load(
//			"/" + img.uri,
//			VK_FORMAT_R8G8B8A8_UNORM,
//
//		);
	}
}

void AssetHelper::loadMaterials(const gltfModel &_model) noexcept
{

}

void AssetHelper::loadTextures(const gltfModel &_model) noexcept
{

}

void AssetHelper::loadNode(
	const gltfNode 						&_node,
	const gltfModel						&_model,
	std::vector<uint32_t> 		&_indices,
	std::vector<Mesh::Vertex>	&_vertices
) noexcept
{

}