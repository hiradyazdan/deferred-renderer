#pragma once

#include "utils.h"

namespace vk
{
	class CmdBuffer
	{
		friend class Device;
		struct Data
		{
			VkCommandBuffer offscreenCmdBuffer = VK_NULL_HANDLE;
			std::vector<VkCommandBuffer> drawCmdBuffers;
		};
	};
}