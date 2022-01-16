#ifndef APP_CONSTANTS_H
#define APP_CONSTANTS_H

namespace constants
{
	static constexpr const auto WINDOW_TITLE = "Deferred Renderer";
	static constexpr const auto WINDOW_WIDTH = 1280;
	static constexpr const auto WINDOW_HEIGHT = 720;

	static const std::string ASSET_PATH = "./assets/";
	static const auto SHADERS_PATH	= ASSET_PATH + "shaders/";
	static const auto MODELS_PATH		= ASSET_PATH + "models/";
	static const auto TEXTURES_PATH = ASSET_PATH + "textures/";

	// @todo: temporary - should implement custom initializer list (cross-compile)
	static constexpr const vk::Array<const char*, 1> models = {
		"sponza.gltf"
	};

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

		static constexpr const auto _count_ = 4;
	}
}

#endif
