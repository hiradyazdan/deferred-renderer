#include "vk/Descriptor.h"

namespace vk
{
	VkDescriptorPoolSize Descriptor::createPoolSize(
		const VkDescriptorType	&_type,
		uint32_t								_descCount
	) noexcept
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type							= _type;
		poolSize.descriptorCount	= _descCount;

		return poolSize;
	}

	void Descriptor::createPool(
		const VkDevice																		&_logicalDevice,
		const std::initializer_list<VkDescriptorPoolSize>	&_poolSizes,
		uint32_t																					_maxSets,
		VkDescriptorPool																	&_pool
	) noexcept
	{
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.maxSets				= _maxSets;
		poolInfo.poolSizeCount	= _poolSizes.size();
		poolInfo.pPoolSizes			= data(_poolSizes);

		auto result = vkCreateDescriptorPool(
			_logicalDevice,
			&poolInfo,
			nullptr,
			&_pool
		);
		ASSERT_VK(result, "Failed to create Descriptor Pool");
	}

	VkDescriptorSetLayoutBinding Descriptor::createSetLayoutBinding(
		const VkDescriptorType		&_type,
		const VkShaderStageFlags	&_stageFlags,
		uint32_t									_binding,
		uint32_t									_descCount
	) noexcept
	{
		VkDescriptorSetLayoutBinding setLayoutBinding = {};
		setLayoutBinding.descriptorType		= _type;
		setLayoutBinding.stageFlags				= _stageFlags;
		setLayoutBinding.binding				 	= _binding;
		setLayoutBinding.descriptorCount	= _descCount;

		return setLayoutBinding;
	}

	void Descriptor::createSetLayout(
		const VkDevice																						&_logicalDevice,
		const std::initializer_list<VkDescriptorSetLayoutBinding>	&_setLayoutBindings,
		VkDescriptorSetLayout																			&_setLayout
	) noexcept
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext				= nullptr;
		layoutInfo.flags				= 0;
		layoutInfo.bindingCount	= _setLayoutBindings.size();
		layoutInfo.pBindings		= data(_setLayoutBindings);

		const auto &result = vkCreateDescriptorSetLayout(
			_logicalDevice,
			&layoutInfo,
			nullptr,
			&_setLayout
		);
		ASSERT_VK(result, "Failed to create DescriptorSet Layout");
	}
}