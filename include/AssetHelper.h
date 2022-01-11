#pragma once

#include "vk/vk.h"
#include "_constants.h"

namespace tinygltf
{
	class 	TinyGLTF;
	class 	Model;
	class		Node;

	struct	Image;
	struct	Accessor;
	struct	BufferView;
	struct	Buffer;
}

class AssetHelper
{
	using gltfModel				= tinygltf::Model;
	using gltfNode				= tinygltf::Node;
	using gltfLoader			= tinygltf::TinyGLTF;
	using gltfImage				= tinygltf::Image;
	using gltfAccessor		= tinygltf::Accessor;
	using gltfBufferView	= tinygltf::BufferView;
	using gltfBuffer			= tinygltf::Buffer;

	using Vertex					= vk::Model::Vertex;
	using Primitive				= vk::Model::Primitive;
	using Mesh						= vk::Model::Mesh;
	using Node						= vk::Model::Node;
	using Texture					= vk::Texture;

	using VertexAttr			= Vertex::Attributes;

	using NodePtr					= std::shared_ptr<Node>;

	enum class IdxComponentType : int;

	public:
		static void load(
			const std::string	&_fileName,
			vk::Model::Data		&_data,
			float 						_scale = 1.0f
		) noexcept;

	private:
		static void loadModel(
			const std::string	&_fileName,
			gltfModel					&_model
		) noexcept;

	private:
		static void loadImages		(const gltfModel &_model)	noexcept;
		static void loadMaterials(const gltfModel &_model)	noexcept;
		static void loadTextures	(const gltfModel &_model)	noexcept;
		static void loadNode			(
			const gltfNode				&_node,
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
				attrCount == vk::toInt(VertexAttr::_count_),
				"_buffers attributes count should match vertex attributes count"
			);

			const auto attr = static_cast<VertexAttr>(counter);

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

		template<typename TComponentType>
		static void getIndices(
			const gltfBuffer 			&_buffer,
			const size_t					_bufferIndex,
			const uint32_t				_firstVtx,
			const uint32_t				_idxCount,
			std::vector<uint32_t> &_indices
		) noexcept
		{
			const auto &buffer = getBufferData<TComponentType>(_buffer, _bufferIndex);

			_indices.resize(_idxCount);

			for(auto i = 0; i < _idxCount; i++)
			{
				_indices[i] = buffer[i] + _firstVtx;
			}
		}

		template<typename TComponentType>
		static auto getBufferData(const gltfBuffer &_buffer, const std::size_t _index) noexcept
		{ return reinterpret_cast<const TComponentType*>(&(_buffer.data[_index])); }

	private:
		template<VertexAttr attr>
		static auto hasAttribute			(const std::map<std::string, int> &_attrs) noexcept
		{ return getAttribute<attr>(_attrs) != _attrs.end(); }

		template<VertexAttr attr>
		static auto getAttribute			(const std::map<std::string, int> &_attrs) noexcept
		{ return _attrs.find(Vertex::vtxAttrs[attr]); }

		template<VertexAttr attr>
		static auto &getAccessorIndex	(const std::map<std::string, int> &_attrs) noexcept
		{ return getAttribute<attr>(_attrs)->second; }

		template<VertexAttr attr>
		static auto &getAccessor			(const gltfModel &_model, const std::map<std::string, int> &_attrs) noexcept
		{ return getAccessor(_model,		getAccessorIndex<attr>(_attrs)); }

		template<VertexAttr attr>
		static auto &getBufferView		(const gltfModel &_model, const std::map<std::string, int> &_attrs) noexcept
		{ return getBufferView(_model,	getAccessor<attr>(_model, _attrs)); }

		template<VertexAttr attr>
		static auto &getBuffer				(const gltfModel &_model, const std::map<std::string, int> &_attrs) noexcept
		{ return getBuffer(_model,			getBufferView<attr>(_model, _attrs)); }
};