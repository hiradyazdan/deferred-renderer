#pragma once

#include "utils.h"

#include <ktx.h>
#include <ktxvulkan.h>

namespace vk
{
	class Device;
	class Texture
	{
		using DevicePtr = std::unique_ptr<Device>;

		public:
			enum class Sampler : uint16_t;

			struct Data
			{
				struct Temp
				{
					STACK_ONLY(Temp)

					Vector<VkBufferImageCopy> bufferCopyRegions = {}; // ONLY for buffer staging (optimal tiling)

					uint32_t									mipLevelCount		= 0;
					VkFormat 									format					= VK_FORMAT_UNDEFINED;
					VkImageLayout							imageLayout			= VK_IMAGE_LAYOUT_UNDEFINED;
					VkImageUsageFlags					imageUsageFlags = 0;
					VkExtent2D								imageExtent			= {};
					VkDeviceSize							size						= 0;
					void											*entry					= nullptr;
				};

				Vector<VkSampler>							samplers;
				Vector<VkImageView>						imageViews;
				Vector<VkImage>								images;
				Vector<VkDeviceMemory>				memories;

				Vector<VkImageLayout>					imageLayouts;
				Vector<VkDescriptorImageInfo>	imageInfos;

				size_t size() const noexcept { return textureCount; }
				void resize(size_t _textureCount) noexcept
				{
					samplers.resize			(_textureCount);
					imageViews.resize		(_textureCount);
					images.resize				(_textureCount);
					memories.resize			(_textureCount);

					imageLayouts.resize	(_textureCount);
					imageInfos.resize		(_textureCount);

					textureCount = _textureCount;
				}

				private:
					size_t											textureCount = 0;
			};

		public:
			static bool isKtx(const std::string &_uri) noexcept;

		public:
			static void destroy(
				const VkDevice					&_logicalDevice,
				const Data							&_data
			) noexcept;

		public:
			static void load(
				const DevicePtr							&_device,
				const std::string						&_fileName,
				const VkFormat							&_format,
				uint16_t										_index,
				Data												&_data,
				bool               					_isLinearTiling		= false,
				const VkAllocationCallbacks	*_pAllocator			= nullptr,
				const VkImageUsageFlags			&_imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
				const VkImageLayout					&_imageLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			) noexcept;
			static void load() noexcept;

		private:
			static ktxResult load(
				const std::string &_fileName,
				ktxTexture **_texture
			) noexcept;

			static void recordStagingBufferCommands(
				const VkDevice					&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties &_memProps,
				const VkCommandBuffer		&_cmdBuffer,
				const Data::Temp				&_inData,
				const VkBuffer 					&_buffer,
				VkDeviceMemory					&_memory,
				VkImage									&_image
			) noexcept;

			static void recordStagingImageCommands(
				const VkDevice					&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties &_memProps,
				const VkCommandBuffer		&_cmdBuffer,
				Data::Temp							&_inData,
				VkDeviceMemory					&_memory,
				VkImage									&_image
			) noexcept;

			template<typename TTexture>
			inline static auto getImageOffset(TTexture *_texture, uint32_t _mipLevel) noexcept
			{
				ktx_size_t offset;
				auto result = ktxTexture_GetImageOffset(_texture, _mipLevel, 0, 0, &offset);

				ASSERT(result == KTX_SUCCESS, "Failed to get mip level offset of the texture!");

				return offset;
			}

			static void updateDescriptor(uint16_t _index, Data &_data) noexcept;
	};
}