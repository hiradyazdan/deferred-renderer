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
		poolInfo.pPoolSizes			= std::data(_poolSizes);

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
		const VkDevice																	&_logicalDevice,
		const std::vector<VkDescriptorSetLayoutBinding>	&_setLayoutBindings,
		VkDescriptorSetLayout														&_setLayout
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

	void Descriptor::allocSets(
		const VkDevice							&_logicalDevice,
		const VkDescriptorPool			&_pool,
		const VkDescriptorSetLayout	*_pSetLayouts,
		VkDescriptorSet							*_pSets,
		uint32_t										_setCount
	) noexcept
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType								= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool			= _pool;
		allocInfo.pSetLayouts					= _pSetLayouts;
		allocInfo.descriptorSetCount	= _setCount;

		const auto &result = vkAllocateDescriptorSets(
			_logicalDevice,
			&allocInfo,
			_pSets
		);
		ASSERT_VK(result, "Failed to allocate Descriptor Sets");
	}

	void Descriptor::updateSets(
		const VkDevice																		&_logicalDevice,
		const std::initializer_list<VkWriteDescriptorSet>	&_writeSets,
		const VkCopyDescriptorSet													*_copies,
		uint32_t																					_copyCount
	) noexcept
	{
		vkUpdateDescriptorSets(
			_logicalDevice,
			_writeSets.size(),
			data(_writeSets),
			_copyCount,
			_copies
		);
	}

	VkWriteDescriptorSet Descriptor::createWriteSet(
		const VkDescriptorSet								&_set,
		const VkDescriptorSetLayoutBinding	&_layoutBinding,
		const VkDescriptorBufferInfo				*_pBufferInfo
	) noexcept
	{
		VkWriteDescriptorSet descriptor	= createWriteSet(_set, _layoutBinding);
		descriptor.pBufferInfo          = _pBufferInfo;

		return descriptor;
	}

	VkWriteDescriptorSet Descriptor::createWriteSet(
		const VkDescriptorSet								&_set,
		const VkDescriptorSetLayoutBinding	&_layoutBinding,
		const VkDescriptorImageInfo					*_pImageInfo
	) noexcept
	{
		VkWriteDescriptorSet descriptor	= createWriteSet(_set, _layoutBinding);
		descriptor.pImageInfo          	= _pImageInfo;

		return descriptor;
	}

	VkWriteDescriptorSet Descriptor::createWriteSet(
		const VkDescriptorSet								&_set,
		const VkDescriptorSetLayoutBinding	&_layoutBinding
	) noexcept
	{
		VkWriteDescriptorSet descriptor	= {};

		descriptor.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor.dstSet               = _set;
		descriptor.dstBinding           = _layoutBinding.binding;
		descriptor.descriptorCount      = _layoutBinding.descriptorCount;
		descriptor.descriptorType       = _layoutBinding.descriptorType;

		return descriptor;
	}
}