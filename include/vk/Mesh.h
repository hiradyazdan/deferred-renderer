#pragma once

#include "Material.h"

namespace vk
{
	struct Mesh
	{
		enum class BufferType
		{
			VERTEX	= 0,
			INDEX		= 1,
			_count_	= 2
		};

		inline static const uint16_t s_bufferTypeCount = toInt(BufferType::_count_);

		struct Vertex
		{
			glm::vec3 position;
			glm::vec2 texCoord;
			glm::vec3 color;
			glm::vec3 normal;
			glm::vec4 tangent;
		};

		// @todo: should these be temporary data?
		std::vector<uint32_t>					indices;
		std::vector<Vertex>						vertices;

		Buffer::MeshData<s_bufferTypeCount> bufferData;

//		std::shared_ptr<Material>			material;
	};
}