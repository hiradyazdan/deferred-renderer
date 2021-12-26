#pragma once

#include "utils.h"
#include "Framebuffer.h"

namespace vk
{
	class RenderPass
	{
		public:
			template<uint16_t framebufferAttCount>
			struct Data
			{
				Framebuffer::Data<framebufferAttCount>	framebufferData;
			};

		public:
			static void create() noexcept;
	};
}