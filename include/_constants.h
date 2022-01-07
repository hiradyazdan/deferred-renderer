#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constants
{
	static constexpr const auto WINDOW_TITLE = "Deferred Renderer";
	static constexpr const auto WINDOW_WIDTH = 1280;
	static constexpr const auto WINDOW_HEIGHT = 720;

	static const std::string ASSET_PATH = "./assets/";
	static const auto SHADERS_PATH	= ASSET_PATH + "shaders/";
	static const auto MODELS_PATH		= ASSET_PATH + "models/";
	static const auto TEXTURES_PATH = ASSET_PATH + "textures/";

	namespace models
	{
		static constexpr const auto sponza = "sponza.gltf";
	}

	namespace shaders
	{
		// Offscreen (G-buffer)
		namespace geometryPass
		{
			static constexpr const auto vert = "geometry_pass.vert";
			static constexpr const auto frag = "geometry_pass.frag";
		}

		// Composition (Deferred)
		namespace lightingPass
		{
			static constexpr const auto vert = "lighting_pass.vert";
			static constexpr const auto frag = "lighting_pass.frag";
		}
	}

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
