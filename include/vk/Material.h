#pragma once

#include "utils.h"
#include "Pipeline.h"

namespace vk
{
	struct Material
	{
		std::vector<VkDescriptorSet>	descriptorSets;
	};
}