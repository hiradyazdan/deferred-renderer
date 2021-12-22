#pragma once

#include "utils.h"

namespace vk
{
	struct Material
	{
		VkPipeline			pipeline;
		VkDescriptorSet	descriptorSet;
	};
}