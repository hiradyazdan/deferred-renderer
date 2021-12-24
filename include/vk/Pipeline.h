#pragma once

#include "utils.h"

namespace vk
{
	class Pipeline
	{
		public:
			struct Data
			{
				VkPipeline				pipeline;
				VkPipelineLayout	pipelineLayout;
			};
	};
}