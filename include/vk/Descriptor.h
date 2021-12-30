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
				std::array<VkDescriptorSetLayout, setLayoutCount>	setLayouts;

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
				const VkDevice																						&_logicalDevice,
				const std::initializer_list<VkDescriptorSetLayoutBinding>	&_setLayoutBindings,
				VkDescriptorSetLayout																			&_setLayout
			) noexcept;
	};
}