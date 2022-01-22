#pragma once

#include "Texture.h"

namespace vk
{
	struct Material
	{
		enum class AlphaMode : uint16_t
		{
			OPAQUE_	= 0,
			MASK_		= 1,
			BLEND_	= 2,

			_count_ = 3
		};

		enum class TexParam : uint16_t
		{
			BASE_COLOR_TEXTURE					= 0,
			METALLIC_ROUGHNESS_TEXTURE	= 1,
			NORMAL_TEXTURE							= 2,
			EMISSIVE_TEXTURE						= 3,
			OCCLUSION_TEXTURE						= 4,

			_count_ = 5
		};

//		private:
			enum class TextureParam : uint16_t
			{
				BASE_COLOR_TEXTURE					= 0,
				METALLIC_ROUGHNESS_TEXTURE	= 1,

				_count_ = 2
			};

			enum class AdditionalTextureParam : uint16_t
			{
				NORMAL_TEXTURE							= 0,
				EMISSIVE_TEXTURE						= 1,
				OCCLUSION_TEXTURE						= 2,

				_count_ = 3
			};

			enum class FactorParam : uint16_t
			{
				ROUGHNESS_FACTOR						= 0,
				METALLIC_FACTOR							= 1,

				_count_ = 2
			};

			enum class ColorFactorParam : uint16_t
			{
				BASE_COLOR_FACTOR						= 0,
				ALPHA_CUTOFF								= 1,
				EMISSIVE_FACTOR							= 2,

				_count_ = 3
			};

		public:

		enum class Param : uint16_t
		{
			BASE_COLOR_TEXTURE					= 0,
			METALLIC_ROUGHNESS_TEXTURE	= 1,

			ROUGHNESS_FACTOR						= 2,
			METALLIC_FACTOR							= 3,
			BASE_COLOR_FACTOR						= 4,

			_count_ = 5
		};

		enum class AdditionalParam : uint16_t
		{
			NORMAL_TEXTURE		= 0,
			EMISSIVE_TEXTURE	= 1,
			OCCLUSION_TEXTURE	= 2,

			ALPHA_MODE				= 3,
			ALPHA_CUTOFF			= 4,

			EMISSIVE_FACTOR		= 5,

			_count_ = 6
		};

		enum class Extension : uint16_t
		{
			_count_ = 8
		};

		static inline const Array<std::string, toInt(AlphaMode::_count_)> alphaModes = {
			"OPAQUE",
			"MASK",
			"BLEND"
		};

		static constexpr const Array<const char*, toInt(TextureParam::_count_)> textureParamKeys = {
			"baseColorTexture",
			"metallicRoughnessTexture"
		};

		static constexpr const Array<const char*, toInt(AdditionalTextureParam::_count_)> additionalTextureParamKeys = {
			"normalTexture",
			"emissiveTexture",
			"occlusionTexture"
		};

		static constexpr const Array<const char*, toInt(FactorParam::_count_)> factorParamKeys = {
			"roughnessFactor",
			"metallicFactor"
		};

		static constexpr const Array<const char*, toInt(ColorFactorParam::_count_)> colorFactorParamKeys = {
			"baseColorFactor",
			"alphaCutoff",
			"emissiveFactor"
		};

		static constexpr const Array<const char*, toInt(Param::_count_)> paramKeys = {
			"baseColorTexture",
			"metallicRoughnessTexture",
			"roughnessFactor",
			"metallicFactor",
			"baseColorFactor"
		};

		static constexpr const Array<const char*, toInt(AdditionalParam::_count_)> additionalParamKeys = {
			"normalTexture",
			"emissiveTexture",
			"occlusionTexture",
			"alphaMode",
			"alphaCutoff",
			"emissiveFactor"
		};

		static constexpr const Array<const char*, toInt(Extension::_count_)> extensionKeys = {

		};

		std::string alphaMode = alphaModes[AlphaMode::OPAQUE_];
		float alphaCutoff = 1.0f;
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		glm::vec4 emissiveFactor = glm::vec4(1.0f);

		const static int descCount = toInt(TextureParam::_count_) + toInt(AdditionalTextureParam::_count_);

		Array<VkDescriptorImageInfo, descCount> descriptors;
//		Vector<VkDescriptorImageInfo> descriptors;

		Array<Texture::Data, toInt(TextureParam::_count_) + toInt(AdditionalTextureParam::_count_)> textures;
		Array<float, toInt(FactorParam::_count_)> factors;
		Array<glm::vec4, toInt(ColorFactorParam::_count_)> colorFactors;

//		Texture *baseColorTexture;
//		Texture *metallicRoughnessTexture;
//		Texture *normalTexture;
//		Texture *occlusionTexture;
//		Texture *emissiveTexture;

		bool doubleSided = false;

		enum class TexCoordSet : uint16_t
		{
			BASE_COLOR					= 0,
			METALLIC_ROUGHNESS	= 1,

			NORMAL							= 2,
			EMISSIVE						= 3,
			OCCLUSION						= 4,

			SPECULAR_GLOSSINESS	= 5, // Extension

			_count_ = 6
		};

		Array<int, toInt(TexCoordSet::_count_)> texCoordSets;

//		struct {
//			uint8_t baseColor = 0;
//			uint8_t metallicRoughness = 0;
//			uint8_t specularGlossiness = 0;
//			uint8_t normal = 0;
//			uint8_t occlusion = 0;
//			uint8_t emissive = 0;
//		} texCoordSets;
		struct
		{
			Texture *specularGlossinessTexture;
			Texture *diffuseTexture;
			glm::vec4 diffuseFactor = glm::vec4(1.0f);
			glm::vec3 specularFactor = glm::vec3(0.0f);
		} extension;
		struct PbrWorkflows
		{
			bool metallicRoughness = true;
			bool specularGlossiness = false;
		} pbrWorkflows;
	};
}