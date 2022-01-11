#ifndef VK_CONSTANTS_H
#define VK_CONSTANTS_H

namespace vk::constants
{
	static constexpr const float CLEAR_COLOR[4] =
	{
		0.0f, // r
		0.0f, // g
		0.0f, // b
		1.0f  // a
	};

	static constexpr const std::pair<float, uint32_t> CLEAR_DEPTH_STENCIL =
	{
		1.0f, // depth
		0     // stencil
	};

	struct NOOP
	{
		NOOP() = delete;
		NOOP(const NOOP&) = delete;
		NOOP& operator=(const NOOP&) = delete;
	};
}

#endif