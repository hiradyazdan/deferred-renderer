#pragma once

#include "utils.h"

namespace vk
{
	class Pipeline
	{
		public:
			struct Data
			{
				VkPipeline						pipeline				= VK_NULL_HANDLE;
				VkPipelineLayout			pipelineLayout	= VK_NULL_HANDLE;

				VkPipelineStageFlags	pipelineStages	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			};
	};
}