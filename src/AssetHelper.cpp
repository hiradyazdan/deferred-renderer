#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
//#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif

#include <tiny_gltf.h>

#include "AssetHelper.h"

enum class AssetHelper::IdxComponentType : int
{
	UNSIGNED_INT		= TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT,
	UNSIGNED_SHORT	= TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT,
	UNSIGNED_BYTE		= TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE
};

void AssetHelper::load(
	const std::string	&_fileName,
	vk::Model::Data		&_data,
	float 						_scale
) noexcept
{
	gltfModel	model;

	loadModel(_fileName, model);

	loadImages		(model);
	loadMaterials	(model);
	loadTextures	(model);

	const auto &scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
	auto &nodes = scene.nodes;
	auto meshCount = model.meshes.size();

	for(const auto &node : nodes)
	{
		const auto &modelNode = model.nodes[node];

		loadNode(modelNode, model, _scale, _data);
	}
}

void AssetHelper::loadModel(
	const std::string	&_fileName,
	gltfModel					&_model
) noexcept
{
	gltfLoader	loader;
	std::string	err, warn;

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

void AssetHelper::loadImages(const gltfModel &_model) noexcept
{
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
	const gltfNode 				&_node,
	const gltfModel				&_model,
	float 								_scale,
	vk::Model::Data				&_data,
	const NodePtr					&_parent
) noexcept
{
	auto newNode = std::make_shared<Node>();

	newNode->name		= _node.name;
	newNode->parent	= _parent;
	newNode->matrix	= glm::mat4(1.0f);

	const auto &nodeChildren = _node.children;
	if(!nodeChildren.empty())
	{
		const auto &nodeChildrenCount = nodeChildren.size();
		for(auto n = 0u; n < nodeChildrenCount; n++)
		{
			INFO_LOG("node child index: %d", n);

			loadNode(_model.nodes[nodeChildren[n]], _model, _scale, _data, newNode);
		}
	}

	if(_node.mesh > -1)
	{
		const auto &mesh = _model.meshes[_node.mesh];

		Mesh newMesh;

		newMesh.name = mesh.name;

		const auto &primitivesCount = mesh.primitives.size();

		newMesh.primitives.resize(primitivesCount);

		for(auto p = 0u; p < primitivesCount; p++)
		{
			const auto &primitive = mesh.primitives[p];

			if(primitive.indices < 0) { continue; }

			auto firstIdx = static_cast<uint32_t>(_data.indices.size());
			auto firstVtx = static_cast<uint32_t>(_data.vertices.size());

			uint32_t idxCount = 0;
			uint32_t vtxCount = 0;

			glm::vec3	posMin;
			glm::vec3 posMax;

			auto hasSkin = false;

			// Vertices
			{
				auto tempData = vk::Model::Data::Temp::create();
				auto &buffers = tempData.buffers;
				const auto &attrs = primitive.attributes;

				ASSERT(hasAttribute<VertexAttr::POS>(attrs), "Position attribute is required");

				getVtxBuffers(_model, attrs, buffers);

				vtxCount	= static_cast<uint32_t>(getAccessor<VertexAttr::POS>(_model, attrs).count);
				hasSkin		= buffers[VertexAttr::JOINTS] && buffers[VertexAttr::WEIGHTS];

				_data.vertices.resize(vtxCount);

				// @todo: linearize & remove branches
				for(auto vtx = 0u; vtx < vtxCount; vtx++)
				{
					Vertex vertex;

					vertex.position	= glm::vec4(glm::make_vec3(&buffers[VertexAttr::POS][vtx * 3]), 1.0f);
					vertex.normal		= glm::normalize(glm::vec3(
						buffers[VertexAttr::NORMAL]
						? glm::make_vec3(&buffers[VertexAttr::NORMAL][vtx * 3])
						: glm::vec3(0.0f)
						)
					);
					vertex.texCoord	= buffers[VertexAttr::UV]
						? glm::make_vec2(&buffers[VertexAttr::UV][vtx * 2])
						: glm::vec3(0.0f);
					vertex.color		= buffers[VertexAttr::COLOR]
						? (
							getAccessor<VertexAttr::COLOR>(_model, attrs).type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3
							? glm::vec4(glm::make_vec3(&buffers[VertexAttr::COLOR][vtx * 3]), 1.0f)
							: glm::make_vec4(&buffers[VertexAttr::COLOR][vtx * 4])
						)
						: glm::vec4(1.0f);
					vertex.tangent	= buffers[VertexAttr::TANGENT]
						? glm::vec4(glm::make_vec4(&buffers[VertexAttr::TANGENT][vtx * 4]))
						: glm::vec4(0.0f);
					vertex.joint0		= hasSkin
						? glm::vec4(glm::make_vec4(&reinterpret_cast<const uint16_t*>(buffers[VertexAttr::JOINTS])[vtx * 4]))
						: glm::vec4(0.0f);
					vertex.weight0	= hasSkin
						? glm::make_vec4(&buffers[VertexAttr::WEIGHTS][vtx * 4])
						: glm::vec4(0.0f);

					_data.vertices[vtx] = vertex;
				}
			}

			// Indices
			{
				const auto &accessor = getAccessor(_model, primitive.indices);
				const auto &bufferView = getBufferView(_model, accessor);
				const auto &buffer = getBuffer(_model, bufferView);
				const auto bufferIndex = accessor.byteOffset + bufferView.byteOffset;
				const auto idxComponentType = accessor.componentType;

				// @todo: should add to prev mesh index count or reset?
				idxCount = static_cast<uint32_t>(accessor.count);

				switch (idxComponentType)
				{
					case vk::toInt(IdxComponentType::UNSIGNED_INT):
						getIndices<uint32_t>(buffer, bufferIndex, firstVtx, idxCount, _data.indices);
						break;
					case vk::toInt(IdxComponentType::UNSIGNED_SHORT):
						getIndices<uint16_t>(buffer, bufferIndex, firstVtx, idxCount, _data.indices);
						break;
					case vk::toInt(IdxComponentType::UNSIGNED_BYTE):
						getIndices<uint8_t>(buffer, bufferIndex, firstVtx, idxCount, _data.indices);
						break;
					default:
						WARN_LOG("Index component type %s not supported!", idxComponentType);
						return;
				}
			}

			Primitive newPrimitive;
			newPrimitive.indexParams.firstIndex = firstIdx;
			newPrimitive.idxCount								= idxCount;
			newPrimitive.matIndex								= primitive.material;

			newMesh.primitives[p] = newPrimitive;
		}

//		newNode.mesh = newMesh;
	}

	auto &nodes = _parent ? _parent->children : _data.nodes;

	nodes.push_back(newNode);
}

const AssetHelper::gltfAccessor		&AssetHelper::getAccessor		(const gltfModel &_model, 	int										_index)				noexcept
{ return _model.accessors		[_index]; }

const AssetHelper::gltfBufferView &AssetHelper::getBufferView	(const gltfModel &_model, 	const gltfAccessor		&_accessor)		noexcept
{ return _model.bufferViews	[_accessor.bufferView]; }

const AssetHelper::gltfBuffer			&AssetHelper::getBuffer			(const gltfModel &_model, 	const gltfBufferView	&_bufferView)	noexcept
{ return _model.buffers			[_bufferView.buffer]; }