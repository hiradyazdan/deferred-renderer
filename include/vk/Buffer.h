#pragma once

#include "utils.h"
#include "Device.h"

namespace vk
{
	class Buffer
	{
		public:
			enum class Type : uint16_t;

			template<uint16_t uboCount>
			struct Data
			{
				struct Temp
				{
					std::array<VkDeviceSize,					uboCount>	sizes;
					std::array<VkDeviceSize,					uboCount>	alignments;
				};

				std::array<VkBuffer,								uboCount>	buffers;
				std::array<VkDeviceMemory,					uboCount>	memories;
				std::array<VkDescriptorBufferInfo,	uboCount>	descInfos;
				std::array<void*,										uboCount>	ubos;
			};

		public:
			static void allocMemory(
				const std::unique_ptr<vk::Device>	&_device,
				const VkBufferUsageFlags					&_usageFlags,
				const VkMemoryPropertyFlags				&_memProps,
				const VkBuffer										&_buffer,
				VkDeviceSize 											&_memAlignment,
				VkDeviceMemory										&_memory
			)	noexcept;

			static void create(
				const VkDevice							&_logicalDevice,
				VkDeviceSize								_size,
				const VkBufferUsageFlags		&_usageFlags,
				VkBuffer										&_buffer
			) noexcept;

			template<uint16_t uboCount>
			static void create(
				const std::unique_ptr<vk::Device>	&_device,
				typename Data<uboCount>::Temp			&_tempData,
				Data<uboCount>										&_bufferData,
				const VkBufferUsageFlags					&_usageFlags	=	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				const VkMemoryPropertyFlags				&_memProps		=	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
																													VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			) noexcept
			{
				const auto &deviceData = _device->getData();

				auto &buffers			= _bufferData.buffers;
				auto &memories		= _bufferData.memories;
				auto &descriptors	= _bufferData.descInfos;
				auto &ubos 				= _bufferData.ubos;

				auto &sizes				= _tempData.sizes;
				auto &alignments	= _tempData.alignments;

				for(auto i = 0u; i < uboCount; i++)
				{
					auto &uboDst			= ubos				[i] = nullptr;
					auto &buffer 			= buffers			[i] = VK_NULL_HANDLE;
					auto &memory			= memories		[i] = VK_NULL_HANDLE;
					auto &descriptor	= descriptors	[i];
					auto &size				= sizes				[i];
					auto &alignment		= alignments	[i] = 0; // TODO: decide for alignment

					create(deviceData.logicalDevice, size, _usageFlags, buffer);
					allocMemory(
						_device, _usageFlags, _memProps,
						buffer, alignment, memory
					);

					descriptor.offset	= 0;
					descriptor.buffer	= buffer;
					descriptor.range	= VK_WHOLE_SIZE;

					bindMemory(deviceData.logicalDevice, buffer, memory);
					mapMemory(deviceData.logicalDevice, memory, &uboDst);
				}
			}

		private:
			static void bindMemory(
				const VkDevice				&_logicalDevice,
				const VkBuffer				&_buffer,
				const VkDeviceMemory	&_memory,
				VkDeviceSize					_offset = 0
			) noexcept;
			static void mapMemory(
				const VkDevice				&_logicalDevice,
				const VkDeviceMemory	&_memory,
				void									**_pData,
				VkDeviceSize					_size		= VK_WHOLE_SIZE,
				VkDeviceSize					_offset	= 0
			)	noexcept;
			static void unmapMemory()	noexcept;
	};
}