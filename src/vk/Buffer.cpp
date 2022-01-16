#include "vk/Device.h"
#include "vk/Buffer.h"

namespace vk
{
	void Buffer::createMemory(
		const VkDevice													&_logicalDevice,
		const VkBufferUsageFlags								&_usageFlags,
		const VkPhysicalDeviceMemoryProperties	&_memProps,
		const VkMemoryPropertyFlags							&_propFlags,
		const VkBuffer													&_buffer,
		VkDeviceSize 														&_bufferAlignment,
		VkDeviceMemory													&_memory,
		VkDeviceSize														_offset
	) noexcept
	{
		VkMemoryRequirements memReqs;

		createMemory(
			_logicalDevice, _usageFlags,
			_memProps, _propFlags, _buffer,
			memReqs, _memory, _offset
		);

		_bufferAlignment	= memReqs.alignment;
	}

	void Buffer::createMemory(
		const VkDevice													&_logicalDevice,
		const VkBufferUsageFlags								&_usageFlags,
		const VkPhysicalDeviceMemoryProperties	&_memProps,
		const VkMemoryPropertyFlags							&_propFlags,
		const VkBuffer													&_buffer,
		VkDeviceSize 														&_bufferAlignment,
		VkDeviceMemory													&_memory,
		VkDeviceSize														&_memReqsSize,
		VkDeviceSize														_offset
	) noexcept
	{
		VkMemoryRequirements memReqs;

		createMemory(
			_logicalDevice, _usageFlags,
			_memProps, _propFlags, _buffer,
			memReqs, _memory, _offset
		);

		_bufferAlignment	= memReqs.alignment;
		_memReqsSize			= memReqs.size;
	}

	void Buffer::createMemory(
		const VkDevice													&_logicalDevice,
		const VkBufferUsageFlags								&_usageFlags,
		const VkPhysicalDeviceMemoryProperties	&_memProps,
		const VkMemoryPropertyFlags							&_propFlags,
		const VkBuffer													&_buffer,
		VkMemoryRequirements										&_memReqs,
		VkDeviceMemory													&_memory,
		VkDeviceSize														_offset
	) noexcept
	{
		vkGetBufferMemoryRequirements(_logicalDevice, _buffer, &_memReqs);

		VkMemoryAllocateFlagsInfoKHR allocFlags = {};

		allocFlags.sType	= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
		allocFlags.flags	= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		Device::allocMemory(
			_logicalDevice,
			_memReqs.size,
			Device::getMemoryType(_memReqs.memoryTypeBits, _memProps, _propFlags),
			_memory,
			_usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ? &allocFlags : nullptr
		);
		bindMemory(_logicalDevice, _buffer, _memory, _offset);
	}

	void Buffer::create(
		const VkDevice							&_logicalDevice,
		VkDeviceSize								_size,
		const VkBufferUsageFlags		&_usageFlags,
		VkBuffer										&_buffer,
		const VkSharingMode					&_sharingMode
	) noexcept
	{
		VkBufferCreateInfo bufferInfo = {};

		bufferInfo.sType				= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage				= _usageFlags;
		bufferInfo.size					= _size;
		bufferInfo.sharingMode	= _sharingMode;

		const auto &result = vkCreateBuffer(
			_logicalDevice,
			&bufferInfo,
			nullptr,
			&_buffer
		);
		ASSERT_VK(result, "Failed to create buffer!");
	}

	void Buffer::destroy(
		const VkDevice	&_logicalDevice,
		const VkBuffer	&_buffer
	) noexcept
	{
		vkDestroyBuffer(
			_logicalDevice,
			_buffer,
			nullptr
		);
	}

	void Buffer::bindMemory(
		const VkDevice				&_logicalDevice,
		const VkBuffer				&_buffer,
		const VkDeviceMemory	&_memory,
		VkDeviceSize					_offset
	) noexcept
	{
		const auto &result = vkBindBufferMemory(
			_logicalDevice,
			_buffer,
			_memory,
			_offset
		);
		ASSERT_VK(result, "Failed to bind GPU memory block to buffer");
	}
}