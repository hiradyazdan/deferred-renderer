#pragma once

#include "utils.h"

namespace vk
{
	class Descriptor
	{
		public:
			static const uint16_t s_setLayoutCount;

		public:
			enum class LayoutCategory : uint16_t;

			template<uint32_t setLayoutCount	= 1>
			struct Data
			{
				Data() { ASSERT_ENUMS(LayoutCategory); }

				struct Temp
				{
					STACK_ONLY(Temp);

					std::initializer_list<VkWriteDescriptorSet>	descriptors;

					uint32_t																		materialCount	= 0;
					uint32_t																		maxSetCount		= 1;
				};

				Array<VkDescriptorSetLayout,	setLayoutCount>	setLayouts;
				Vector<VkDescriptorSet>												sets;

				VkDescriptorPool															pool = VK_NULL_HANDLE;
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

			template<uint32_t binding>
			static VkDescriptorSetLayoutBinding createSetLayoutBinding(
				const VkDescriptorType		&_type,
				const VkShaderStageFlags	&_stageFlags,
				uint32_t									_descCount					= 1,
				const VkSampler*					_immutableSamplers	= nullptr
			) noexcept
			{
				VkDescriptorSetLayoutBinding setLayoutBinding = {};
				setLayoutBinding.binding				 		= binding;
				setLayoutBinding.descriptorType			= _type;
				setLayoutBinding.descriptorCount		= _descCount;
				setLayoutBinding.stageFlags					= _stageFlags;
				setLayoutBinding.pImmutableSamplers = _immutableSamplers;

				return setLayoutBinding;
			}
			template<size_t bindingCount>
			inline static void createSetLayout(
				const VkDevice																					&_logicalDevice,
				const Array<VkDescriptorSetLayoutBinding, bindingCount>	&_bindings,
				VkDescriptorSetLayout																		&_setLayout
			) noexcept
			{
				VkDescriptorSetLayoutCreateInfo layoutInfo = {};
				layoutInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.pNext				= nullptr;
				layoutInfo.flags				= 0;
				layoutInfo.bindingCount	= static_cast<uint32_t>(bindingCount);
				layoutInfo.pBindings		= _bindings.data();

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
				const std::initializer_list<VkWriteDescriptorSet>	&_descriptors,
				uint16_t																					_maxDescCount = 4,
				const VkCopyDescriptorSet													*_copies			= nullptr,
				uint32_t																					_copyCount		= 0
			) noexcept;
			static VkWriteDescriptorSet createDescriptor(
				const VkDescriptorSet								&_set,
				const VkDescriptorSetLayoutBinding	&_layoutBinding,
				const VkDescriptorImageInfo					*_pImageInfo
			) noexcept;
			static VkWriteDescriptorSet createDescriptor(
				const VkDescriptorSet								&_set,
				const VkDescriptorSetLayoutBinding	&_layoutBinding,
				const VkDescriptorBufferInfo				*_pBufferInfo
			) noexcept;

		private:
			static VkWriteDescriptorSet createDescriptor(
				const VkDescriptorSet								&_set,
				const VkDescriptorSetLayoutBinding	&_layoutBinding
			) noexcept;
	};
}