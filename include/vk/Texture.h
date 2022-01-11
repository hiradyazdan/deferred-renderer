#pragma once

#include "utils.h"

#include <ktx.h>
#include <ktxvulkan.h>

namespace vk
{
	class Texture
	{
		public:
			static const uint16_t s_samplerCount;

		public:
			enum class Sampler : uint16_t;

			template<uint16_t samplerCount>
			struct Data
			{
				Data()
				{
					ASSERT_ENUMS(Sampler);
				}

				Array<VkSampler,							samplerCount>	samplers;
				Array<VkDeviceMemory,					samplerCount>	memories;
				Array<VkDescriptorImageInfo,	samplerCount>	imageInfos;

				uint16_t																		width;
				uint16_t																		height;
				uint16_t																		mipLevels;
				uint16_t																		layerCount;
			};

		public:
			static bool isKtx(const std::string &_uri) noexcept;

		public:
			static void load(
				const std::string				&_fileName,
				const VkFormat					&_format,
				const VkDevice					&_logicalDevice,
				const VkQueue						&_copyQueue,
				const VkImageUsageFlags	&_imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
				const VkImageLayout			&_imageLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				bool               			_isLinearTiling		= false
			) noexcept;
			static void load(

			) noexcept;

		private:
			static ktxResult load(
				const std::string &_fileName,
				ktxTexture **_texture
			) noexcept;
	};
}