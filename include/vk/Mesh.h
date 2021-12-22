#pragma once

#include "Material.h"

namespace vk
{
	struct Mesh
	{
		VkBuffer									vertexBuffer;
		VkDeviceMemory						vertexMemory;

		VkBuffer									indexBuffer;
		VkDeviceMemory						indexMemory;

		std::shared_ptr<Material>	material;
	};
}