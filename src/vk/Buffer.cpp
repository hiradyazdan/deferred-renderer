#include "vk/Buffer.h"

namespace vk
{
	void Buffer::allocMemory(
		const std::unique_ptr<vk::Device>	&_device,
		const VkBufferUsageFlags					&_usageFlags,
		const VkMemoryPropertyFlags				&_memProps,
		const VkBuffer										&_buffer,
		VkDeviceSize 											&_memAlignment,
		VkDeviceMemory										&_memory
	) noexcept
	{
		const auto &deviceData = _device->getData();
		const auto &logicalDevice = deviceData.logicalDevice;

		VkMemoryRequirements memReqs;

		vkGetBufferMemoryRequirements(logicalDevice, _buffer, &memReqs);

		_memAlignment = memReqs.alignment;

		VkMemoryAllocateInfo allocInfo	= {};
		allocInfo.sType									= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize				= memReqs.size;
		allocInfo.memoryTypeIndex				= _device->getMemoryType(memReqs.memoryTypeBits, _memProps);

		VkMemoryAllocateFlagsInfoKHR allocFlags = {};

		if(_usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
		{
			allocFlags.sType	= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			allocFlags.flags	= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

			allocInfo.pNext		= &allocFlags;
		}

		const auto &result = vkAllocateMemory(
			logicalDevice,
			&allocInfo,
			nullptr,
			&_memory
		);
		ASSERT_VK(result, "Failed to allocate buffer memory!");
	}

	void Buffer::create(
		const VkDevice							&_logicalDevice,
		VkDeviceSize								_size,
		const VkBufferUsageFlags		&_usageFlags,
		VkBuffer										&_buffer
	) noexcept
	{
		VkBufferCreateInfo bufferInfo = {};

		bufferInfo.sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage	= _usageFlags;
		bufferInfo.size		= _size;

		const auto &result = vkCreateBuffer(
			_logicalDevice,
			&bufferInfo,
			nullptr,
			&_buffer
		);
		ASSERT_VK(result, "Failed to create buffer!");
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
		ASSERT_VK(result, "Failed to bind memory block to buffer");
	}

	void Buffer::mapMemory(
		const VkDevice				&_logicalDevice,
		const VkDeviceMemory	&_memory,
		void									**_pData,
		VkDeviceSize					_size,
		VkDeviceSize					_offset
	) noexcept
	{
		const auto &result = vkMapMemory(
			_logicalDevice,
			_memory,
			_offset,
			_size,
			0,
			_pData
		);
		ASSERT_VK(result, "Failed to map data to gpu memory");
	}
}