#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
//#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif

#include "AssetHelper.h"

void AssetHelper::load(
	const DevicePtr		&_device,
	const std::string	&_fileName,
	vk::Model::Data		&_modelData,
	vk::Texture::Data &_textureData,
	float 						_scale,
	const std::string &_textureDir
) noexcept
{
	gltfModel	model;

	loadModel			(_fileName, model);
	loadTextures	(_device, model, _textureDir, _textureData);
	loadMaterials	(model, _textureData, _modelData);

	_modelData.textureCount = _textureData.size();

	const auto &scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
	auto &nodes = scene.nodes;
	auto meshCount = model.meshes.size();

	for(const auto &node : nodes)
	{
		const auto &modelNode = model.nodes[node];

		loadNode(modelNode, model, _scale, _modelData);
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
	// todo
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

void AssetHelper::loadMaterials(
	gltfModel								&_model,
	const vk::Texture::Data	&_textureData,
	vk::Model::Data					&_data
)	noexcept
{
	using TextureParam	= Material::TextureParam;
	using TexCoordSet		= Material::TexCoordSet;
	using ColorFactor		= Material::ColorFactorParam;

	auto 				&materials = _data.materials;
	auto				&gltfMaterials = _model.materials;
	const auto  &texImageInfos = _textureData.imageInfos;
	const auto	materialCount = gltfMaterials.size();
	const auto	&matParams = Material::paramKeys;
	const auto	&matTexParams_1 = Material::textureParamKeys;
	const auto	&matTexParams_2 = Material::additionalTextureParamKeys;

	const auto	&matFacParams_1 = Material::factorParamKeys;
	const auto	&matFacParams_2 = Material::colorFactorParamKeys;
	const auto  &matAlphaModes = Material::alphaModes;

	materials.resize(materialCount);

	for(auto mat = 0u; mat < materialCount; ++mat)
	{
		auto &gltfMat					= gltfMaterials[mat];
		auto &gltfMatVals			= gltfMat.values;
		auto &gltfMatAddVals	= gltfMat.additionalValues;

		auto &material = materials[mat];

		material.alphaMode		= gltfMat.alphaMode;
		material.alphaCutoff	= (float) gltfMat.alphaCutoff;
		material.doubleSided	= gltfMat.doubleSided;

		if(!texImageInfos.empty())
		{
			for(auto p = 0; p < matTexParams_1.size(); ++p)
			{
				const auto &param = gltfMatVals[matTexParams_1[p]];
				auto texIndex = param.TextureIndex();

				if(texIndex < 0) { texIndex = 0; }

				material.descriptors[p] = texImageInfos[texIndex];
				material.texCoordSets[p] = param.TextureTexCoord();
			}

			for(auto p = 0; p < matTexParams_2.size(); ++p)
			{
				const auto &param = gltfMatAddVals[matTexParams_2[p]];
				auto texIndex = param.TextureIndex();

				if(texIndex < 0) { texIndex = 0; }

				material.descriptors[p + toInt(TextureParam::_count_)] = texImageInfos[texIndex];
				material.texCoordSets[p + toInt(TextureParam::_count_)] = param.TextureTexCoord();
			}
		}

		for(auto p = 0; p < matFacParams_1.size(); ++p)
		{
			const auto &param = gltfMatVals[matFacParams_1[p]];

			material.factors[p] = static_cast<float>(param.Factor());
		}

		for(auto p = 0; p < matFacParams_2.size(); ++p)
		{
			const auto &param = gltfMatVals[matFacParams_2[p]];

			const auto &colorFactor = !param.number_array.empty()
				? glm::make_vec3(param.ColorFactor().data())
				: glm::vec<3, double>(0.0f, 0.0f, 0.0f);
			material.colorFactors[p] = glm::vec4(colorFactor, 1.0f);
		}

		const auto &emissiveFactorParam = gltfMatAddVals[matFacParams_2[ColorFactor::EMISSIVE_FACTOR]];
		material.colorFactors[ColorFactor::EMISSIVE_FACTOR] = !emissiveFactorParam.number_array.empty()
			? glm::vec4(glm::make_vec3(emissiveFactorParam.ColorFactor().data()), 1.0f)
			: glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

//		if(hasKey<MatParam::BASE_COLOR_TEXTURE>(gltfMatVals))
//		{
//			const auto &param = gltfMatVals[matParams[MatParam::BASE_COLOR_TEXTURE]];
//
//			material.baseColorTexture = _data.textures[param.TextureIndex()];
//			material.texCoordSets[TexCoordSet::BASE_COLOR] = param.TextureTexCoord();
//		}
	}

	// @todo work out if default material is required?
//	materials[materialCount] = Material();
}

void AssetHelper::loadTextures(
	const DevicePtr		&_device,
	const gltfModel		&_model,
	const std::string	&_textureDir,
	vk::Texture::Data	&_textureData
)	noexcept
{
	const auto textureCount = _model.textures.size();

	// Empty texture
	if(textureCount < 1)
	{
		_textureData.resize(1);
		vk::Texture::create(_device, VK_FORMAT_R8G8B8A8_UNORM, _textureData);

		return;
	}

	_textureData.resize(textureCount);
	for(auto tex = 0u; tex < textureCount; ++tex)
	{
		auto &gltfTexture = _model.textures[tex];
		auto &gltfImage = _model.images[gltfTexture.source];

		vk::Texture::load(
			_device,
			_textureDir + gltfImage.uri,
			VK_FORMAT_R8G8B8A8_UNORM,
			tex, _textureData
		);
	}
}

std::shared_ptr<AssetHelper::Node> AssetHelper::createNode(
	const gltfNode	&_node,
	const NodePtr		&_parent,
	float 					_scale
) noexcept
{
	auto newNode = std::make_shared<Node>();

	newNode->name		= _node.name;
	newNode->parent	= _parent;

	newNode->matrix	= glm::mat4(1.0f); // identity
	auto &nodeTrans = _node.translation;
	auto &nodeRot = _node.rotation;
	auto &nodeScale = _node.scale;
	auto &nodeMtx = _node.matrix;

	if(nodeTrans.size() == 3) // vec3 -> mat4
	{
		newNode->matrix = glm::translate(newNode->matrix, glm::vec3(glm::make_vec3(nodeTrans.data())));
	}
	if(nodeRot.size() == 4) // vec4 (quat) -> mat4
	{
		newNode->matrix *= glm::mat4(static_cast<glm::quat>(glm::make_quat(nodeRot.data())));
	}
	if(nodeScale.size() == 3) // vec3 -> mat4
	{
		newNode->matrix = glm::scale(newNode->matrix, _scale * glm::vec3(glm::make_vec3(nodeScale.data())));
	}
	if(nodeMtx.size() == 16) // mat4 -> mat4
	{
		newNode->matrix = glm::make_mat4x4(nodeMtx.data());
	}

	return newNode;
}

void AssetHelper::loadNode(
	const gltfNode 				&_node,
	const gltfModel				&_model,
	float 								_scale,
	vk::Model::Data				&_data,
	const NodePtr					&_parent
) noexcept
{
	auto newNode = createNode(_node, _parent, _scale);

	const auto &nodeChildren = _node.children;
	if(!nodeChildren.empty())
	{
		const auto &nodeChildrenCount = nodeChildren.size();
		for(auto n = 0u; n < nodeChildrenCount; ++n)
		{
//			INFO_LOG("node child index: %d", n);

			loadNode(_model.nodes[nodeChildren[n]], _model, _scale, _data, newNode);
		}
	}

	if(_node.mesh > -1)
	{
		const auto &mesh = _model.meshes[_node.mesh];

//		std::unique_ptr<vk::Model::Mesh> newMesh;

//		newMesh.name = mesh.name;

		const auto &primitives			= mesh.primitives;
		const auto &primitiveCount	= primitives.size();
		auto &newPrimitives					= newNode->mesh.primitives;

		newPrimitives.resize(primitiveCount);

		for(auto p = 0u; p < primitiveCount; ++p)
		{
			const auto &primitive = primitives[p];
			auto &indices		= _data.indices;
			auto &vertices	= _data.vertices;

			if(primitive.indices < 0) { continue; }

			switch(primitive.mode)
			{
				case TINYGLTF_MODE_TRIANGLES:
					break;
				case TINYGLTF_MODE_TRIANGLE_STRIP:
					break;
				case TINYGLTF_MODE_TRIANGLE_FAN:
					break;
				case TINYGLTF_MODE_LINE:
					break;
				case TINYGLTF_MODE_LINE_STRIP:
					break;
				case TINYGLTF_MODE_LINE_LOOP:
					break;
				case TINYGLTF_MODE_POINTS:
					break;
			}

			auto firstIdx = static_cast<uint32_t>(indices.size());
			auto firstVtx = static_cast<uint32_t>(vertices.size());

			uint32_t idxCount = 0;
			uint32_t vtxCount = 0;

			glm::vec3	posMin;
			glm::vec3 posMax;

			setVertices(
				_model, primitive.attributes,
				firstVtx,
				vtxCount, vertices
			);
			setIndices(
				_model, primitive.indices,
				firstVtx, firstIdx,
				idxCount, indices
			);

			Primitive newPrimitive;
			newPrimitive.indexParams.firstIndex = firstIdx;
			newPrimitive.idxCount								= idxCount;
			newPrimitive.vtxCount								= vtxCount;
			newPrimitive.matIndex								= primitive.material;

			newPrimitives[p] = newPrimitive;
		}
	}

	auto &nodes = _parent ? _parent->children : _data.nodes;

	nodes.push_back(newNode);
}

const AssetHelper::gltfAccessor &AssetHelper::getAccessor(
	const gltfModel &_model, int _index
) noexcept
{ return _model.accessors[_index]; }

const AssetHelper::gltfBufferView	&AssetHelper::getBufferView(
	const gltfModel &_model, const gltfAccessor &_accessor
) noexcept
{ return _model.bufferViews[_accessor.bufferView]; }

const AssetHelper::gltfBuffer &AssetHelper::getBuffer(
	const gltfModel &_model, const gltfBufferView &_bufferView
) noexcept
{ return _model.buffers[_bufferView.buffer]; }