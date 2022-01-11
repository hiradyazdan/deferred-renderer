#pragma once

#include "utils.h"

namespace vk
{
	class Descriptor
	{
		template<uint16_t setLayoutCount, uint16_t layoutBindingCount>
		using LayoutBindingArray = Array<Array<VkDescriptorSetLayoutBinding, layoutBindingCount>, setLayoutCount>;

		public:
			static const uint16_t s_layoutBindingCount;
			static const uint16_t s_setLayoutCount;

		public:
			enum class LayoutCategory : uint16_t;
			enum class LayoutBinding : uint16_t;

			template<
			  uint16_t layoutBindingCount,
				uint16_t setLayoutCount	= 1
			>
			struct Data
			{
				Data() { ASSERT_ENUMS(LayoutCategory, LayoutBinding); }

				struct Temp
				{
					STACK_ONLY(Temp);

					std::initializer_list<VkWriteDescriptorSet>							writeSets;
					LayoutBindingArray<setLayoutCount,	layoutBindingCount>	setLayoutBindings;
				};

				Array<VkDescriptorSetLayout,							setLayoutCount>	setLayouts;
				Vector<VkDescriptorSet>																		sets;

				VkDescriptorPool																					pool = VK_NULL_HANDLE;
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

			template<LayoutBinding binding>
			static VkDescriptorSetLayoutBinding createSetLayoutBinding(
				const VkDescriptorType		&_type,
				const VkShaderStageFlags	&_stageFlags,
				uint32_t									_descCount = 1
			) noexcept
			{
				VkDescriptorSetLayoutBinding setLayoutBinding = {};
				setLayoutBinding.descriptorType		= _type;
				setLayoutBinding.stageFlags				= _stageFlags;
				setLayoutBinding.binding				 	= toInt(binding);
				setLayoutBinding.descriptorCount	= _descCount;

				return setLayoutBinding;
			}

			template<uint16_t layoutBindingCount>
			static void createSetLayout(
				const VkDevice																								&_logicalDevice,
				const Array<VkDescriptorSetLayoutBinding, layoutBindingCount>	&_setLayoutBindings,
				VkDescriptorSetLayout																					&_setLayout
			) noexcept
			{
				VkDescriptorSetLayoutCreateInfo layoutInfo = {};
				layoutInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.pNext				= nullptr;
				layoutInfo.flags				= 0;
				layoutInfo.bindingCount	= _setLayoutBindings.size();
				layoutInfo.pBindings		= _setLayoutBindings.data();

				const auto &result = vkCreateDescriptorSetLayout(
					_logicalDevice,
					&layoutInfo,
					nullptr,
					&_setLayout
				);
				ASSERT_VK(result, "Failed to create DescriptorSet Layout");
			}
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
				uint16_t																					_maxDescCount = 4,
				const VkCopyDescriptorSet													*_copies			= nullptr,
				uint32_t																					_copyCount		= 0
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