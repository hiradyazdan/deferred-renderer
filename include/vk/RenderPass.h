#pragma once

#include "utils.h"

namespace vk
{
	class RenderPass
	{
		public:
			struct Data
			{
				struct Attachment
				{
					VkImage image				= VK_NULL_HANDLE;
					VkDeviceMemory mem	= VK_NULL_HANDLE;
				};

				struct BeginInfo
				{
					VkRenderPass							renderPass	= VK_NULL_HANDLE;
					VkFramebuffer							framebuffer	= VK_NULL_HANDLE;
					std::vector<VkClearValue> clearValues;
					VkExtent2D								swapchainExtent;
				} beginInfo;
			};
	};
}