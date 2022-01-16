#pragma once

#include <tiny_gltf.h>

#include "vk/vk.h"
#include "_constants.h"

class AssetHelper
{
	using gltfModel				= tinygltf::Model;
	using gltfNode				= tinygltf::Node;
	using gltfLoader			= tinygltf::TinyGLTF;
	using gltfImage				= tinygltf::Image;
	using gltfAccessor		= tinygltf::Accessor;
	using gltfBufferView	= tinygltf::BufferView;
	using gltfBuffer			= tinygltf::Buffer;
	using gltfParamMap		= tinygltf::ParameterMap;

	using Material				= vk::Material;
	using Vertex					= vk::Model::Vertex;
	using Primitive				= vk::Model::Primitive;
	using Mesh						= vk::Model::Mesh;
	using Node						= vk::Model::Node;
	using DevicePtr				= std::unique_ptr<vk::Device>;

	using MatParam				= Material::Param;
	using VertexAttr			= Vertex::Attribute;

	using NodePtr					= std::shared_ptr<Node>;

	enum class IdxComponentType : int;

	public:
		static void load(
			const DevicePtr		&_device,
			const std::string	&_fileName,
			vk::Model::Data		&_data,
			float 						_scale				= 1.0f,
			const std::string &_textureDir	= constants::TEXTURES_PATH
		) noexcept;

	private:
		static void loadModel(
			const std::string	&_fileName,
			gltfModel					&_model
		) noexcept;

		static void loadTextures	(
			const DevicePtr		&_device,
			const gltfModel		&_model,
			const std::string	&_textureDir,
			vk::Texture::Data	&_textureData
		)	noexcept;

		static void loadMaterials	(
			gltfModel								&_model,
			const vk::Texture::Data	&_textureData,
			vk::Model::Data					&_data
		)	noexcept;
		static void loadNodes(

		) noexcept;
		static void loadNode(
			const gltfNode 				&_node,
			const gltfModel				&_model,
			float 								_scale,
			vk::Model::Data				&_data,
			const NodePtr					&_parent = nullptr
		) noexcept;

	private:
		static const gltfAccessor		&getAccessor	(const gltfModel	&_model, 	int										_index)				noexcept;
		static const gltfBufferView &getBufferView(const gltfModel	&_model, 	const gltfAccessor		&_accessor)		noexcept;
		static const gltfBuffer			&getBuffer		(const gltfModel	&_model,	const gltfBufferView	&_bufferView)	noexcept;

	private:
		template<uint16_t counter = 0, std::size_t attrCount>
		static typename std::enable_if<counter < attrCount, void>::type getVtxBuffers(
			const gltfModel											&_model,
			const std::map<std::string, int>		&_attrs,
			vk::Array<const float*, attrCount>	&_buffers
		) noexcept
		{
			static_assert(
				attrCount == toInt(VertexAttr::_count_),
				"_buffers attributes count should match vertex attributes count"
			);

			const auto &attr = static_cast<VertexAttr>(counter);

			if(hasAttribute<attr>(_attrs))
			{
				const auto &accessor	= getAccessor		<attr>(_model, _attrs);
				const auto &view			= getBufferView				(_model, accessor);
				const auto &buffer		= getBuffer						(_model, view);

				_buffers[attr] = getBufferData<float>(buffer, accessor.byteOffset + view.byteOffset);
			}

			getVtxBuffers<counter + 1>(_model, _attrs, _buffers);
		}

		template<uint16_t counter = 0, std::size_t attrCount>
		static typename std::enable_if<counter >= attrCount, void>::type getVtxBuffers(
			const gltfModel&,
			const std::map<std::string, int>&,
			vk::Array<const float*, attrCount>&
		) noexcept {}

		inline static void getVertices(
			const vk::Array<const float*, toInt(VertexAttr::_count_)> _buffers,
			const uint32_t				_firstVtx,
			const uint32_t				_vtxCount,
			std::vector<Vertex>		&_vertices,
			bool 									_isColorVec3 = false
		) noexcept
		{
			// @todo: can this reserve cause edge case issues?
			_vertices.reserve(_firstVtx + _vtxCount);

			const auto posBuff			= _buffers[VertexAttr::POS];
			const auto normalBuff		= _buffers[VertexAttr::NORMAL];
			const auto uvBuff				= _buffers[VertexAttr::UV];
			const auto colBuff			= _buffers[VertexAttr::COLOR];
			const auto tangentBuff	= _buffers[VertexAttr::TANGENT];
			const auto jointBuff		= reinterpret_cast<const uint16_t*>(_buffers[VertexAttr::JOINTS]);
			const auto weightBuff		= _buffers[VertexAttr::WEIGHTS];

			const auto vec3Zero = glm::vec3(0.0f);
			const auto vec4Zero = glm::vec4(0.0f);
			const auto vec4One	= glm::vec4(1.0f);

			const auto hasSkin	= _buffers[VertexAttr::JOINTS] && weightBuff;

			for(auto vtx = 0u; vtx < _vtxCount; ++vtx)
			{
				Vertex vertex;

				vertex.position	= glm::vec4(glm::make_vec3(&posBuff[vtx * 3]), 1.0f);
				vertex.normal		= glm::normalize(glm::vec3(normalBuff ? glm::make_vec3(&normalBuff[vtx * 3]) : vec3Zero));
				vertex.texCoord	= uvBuff ? glm::make_vec2(&uvBuff[vtx * 2]) : vec3Zero;
				vertex.color		= colBuff
					? (_isColorVec3 ? glm::vec4(glm::make_vec3(&colBuff[vtx * 3]), 1.0f) : glm::make_vec4(&colBuff[vtx * 4]))
					: vec4One;
				vertex.tangent	= tangentBuff	? glm::make_vec4(&tangentBuff[vtx * 4]) : vec4Zero;
				vertex.joint0		= hasSkin			? glm::vec4(glm::make_vec4(&jointBuff[vtx * 4])) : vec4Zero;
				vertex.weight0	= hasSkin			? glm::make_vec4(&weightBuff[vtx * 4]) : vec4Zero;

				_vertices.push_back(vertex);
			}
		}

		template<typename TComponentType>
		static void getIndices(
			const gltfBuffer 			&_buffer,
			const size_t					_bufferIndex,
			const uint32_t				_firstVtx,
			const uint32_t 				_firstIdx,
			const uint32_t				_idxCount,
			std::vector<uint32_t> &_indices
		) noexcept
		{
			const auto &buffer = getBufferData<TComponentType>(_buffer, _bufferIndex);

			// @todo: can this reserve cause edge case issues?
			_indices.reserve(_firstIdx + _idxCount);

			for(auto i = 0u; i < _idxCount; ++i)
			{
				_indices.push_back(buffer[i] + _firstVtx);
			}
		}

		template<typename TComponentType>
		static auto getBufferData(const gltfBuffer &_buffer, const std::size_t _index) noexcept
		{ return reinterpret_cast<const TComponentType*>(&(_buffer.data[_index])); }

	private:
		template<VertexAttr attrKey>
		static auto hasAttribute			(const std::map<std::string, int> &_attrs) noexcept
		{ return getAttribute<attrKey>(_attrs) != _attrs.end(); }

		template<VertexAttr attrKey>
		static auto getAttribute			(const std::map<std::string, int> &_attrs) noexcept
		{ return _attrs.find(Vertex::attrKeys[attrKey]); }

		template<VertexAttr attrKey>
		static auto &getAccessorIndex	(const std::map<std::string, int> &_attrs) noexcept
		{ return getAttribute<attrKey>(_attrs)->second; }

		template<VertexAttr attrKey>
		static auto &getAccessor			(const gltfModel &_model, const std::map<std::string, int> &_attrs) noexcept
		{ return getAccessor(_model,		getAccessorIndex<attrKey>(_attrs)); }

		template<VertexAttr attrKey>
		static auto &getBufferView		(const gltfModel &_model, const std::map<std::string, int> &_attrs) noexcept
		{ return getBufferView(_model,	getAccessor<attrKey>(_model, _attrs)); }

		template<VertexAttr attrKey>
		static auto &getBuffer				(const gltfModel &_model, const std::map<std::string, int> &_attrs) noexcept
		{ return getBuffer(_model,			getBufferView<attrKey>(_model, _attrs)); }

	private:
		template<MatParam paramKey>
		static auto hasKey(const gltfParamMap &_params) noexcept
		{ return getKey<paramKey>(_params) != _params.end(); }

		template<MatParam paramKey>
		static auto getKey(const gltfParamMap &_params) noexcept
		{ return _params.find(Material::paramKeys[paramKey]); }
};