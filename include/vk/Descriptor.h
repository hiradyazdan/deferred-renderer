#pragma once

#include "utils.h"

namespace vk
{
	class Descriptor
	{
		public:
			template<uint16_t setLayoutCount = 1>
			struct Data
			{
				struct Temp
				{
					std::initializer_list<VkWriteDescriptorSet>	writeSets;
					std::vector<VkDescriptorSetLayoutBinding>		setLayoutBindings;
				};

				std::array<VkDescriptorSetLayout, setLayoutCount>	setLayouts;
				std::vector<VkDescriptorSet>											sets;

				VkDescriptorPool																	pool = VK_NULL_HANDLE;
			};

		public:
			static VkDescriptorPoolSize createPoolSize(
				const VkDescriptorType	&_type,
				uint32_t								_descCount
			) noexcept;
			static void createPool(
				const VkDevice																		&_logicalDevice,
				const std::initializer_list<VkDescriptorPoolSize>	&_poolSizes,
				uint32_t																					_maxSets,
				VkDescriptorPool																	&_pool
			) noexcept;
			static VkDescriptorSetLayoutBinding createSetLayoutBinding(
				const VkDescriptorType		&_type,
				const VkShaderStageFlags	&_stageFlags,
				uint32_t									_binding,
				uint32_t									_descCount = 1
			) noexcept;
			static void createSetLayout(
				const VkDevice																	&_logicalDevice,
				const std::vector<VkDescriptorSetLayoutBinding>	&_setLayoutBindings,
				VkDescriptorSetLayout														&_setLayout
			) noexcept;
			static void allocSets(
				const VkDevice							&_logicalDevice,
				const VkDescriptorPool			&_pool,
				const VkDescriptorSetLayout	*_pSetLayouts,
				VkDescriptorSet							*_pSets,
				uint32_t										_setCount = 1
			) noexcept;
			static void updateSets(
				const VkDevice																		&_logicalDevice,
				const std::initializer_list<VkWriteDescriptorSet>	&_writeSets,
				const VkCopyDescriptorSet													*_copies = nullptr,
				uint32_t																					_copyCount = 0
			) noexcept;
			static VkWriteDescriptorSet createWriteSet(
				const VkDescriptorSet								&_set,
				const VkDescriptorSetLayoutBinding	&_layoutBinding,
				const VkDescriptorImageInfo					*_pImageInfo
			) noexcept;
			static VkWriteDescriptorSet createWriteSet(
				const VkDescriptorSet								&_set,
				const VkDescriptorSetLayoutBinding	&_layoutBinding,
				const VkDescriptorBufferInfo				*_pBufferInfo
			) noexcept;

		private:
			static VkWriteDescriptorSet createWriteSet(
				const VkDescriptorSet								&_set,
				const VkDescriptorSetLayoutBinding	&_layoutBinding
			) noexcept;
	};
}