#pragma once

#include "utils.h"
#include "Device.h"

namespace vk
{
	class Buffer
	{
		public:
			static const uint16_t s_uboCount;

		public:
			enum class Category : uint16_t;

			template<uint16_t uboCount = 1>
			struct Data
			{
				Data()
				{
					debug::isEnumDefined<Category>();
					assert(s_uboCount && "s_uboCount should be explicitly defined/initialized.");
				}

				struct Temp
				{
					STACK_ONLY(Temp);

					Array<VkDeviceSize,					uboCount>	sizes;
					Array<VkDeviceSize,					uboCount>	alignments;
				};

				Array<VkBuffer,								uboCount>	buffers;
				Array<VkDeviceMemory,					uboCount>	memories;
				Array<VkDescriptorBufferInfo,	uboCount>	descInfos;
				Array<void*,									uboCount>	ubos; // TODO: should be temp data?
			};

			template<uint16_t mbtCount = 2>
			struct MeshData
			{
				struct Temp
				{
					STACK_ONLY(Temp);

					Array<VkDeviceSize,					mbtCount>	sizes;
					Array<void*,								mbtCount>	data;
				};

				Array<VkBuffer,								mbtCount>	buffers;
				Array<VkDeviceMemory,					mbtCount>	memories;
			};

		public:
			static void allocMemory(
				const std::unique_ptr<vk::Device>	&_device,
				const VkBufferUsageFlags					&_usageFlags,
				const VkMemoryPropertyFlags				&_memProps,
				const VkBuffer										&_buffer,
				VkDeviceSize 											&_bufferAlignment,
				VkDeviceMemory										&_memory
			)	noexcept;

			static void create(
				const VkDevice							&_logicalDevice,
				VkDeviceSize								_size,
				const VkBufferUsageFlags		&_usageFlags,
				VkBuffer										&_buffer,
				const VkSharingMode					&_sharingMode = VK_SHARING_MODE_EXCLUSIVE
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
				const auto &logicalDevice = deviceData.logicalDevice;

				auto &buffers			= _bufferData.buffers;
				auto &memories		= _bufferData.memories;
				auto &descriptors	= _bufferData.descInfos;
				auto &ubos 				= _bufferData.ubos;

				auto &sizes				= _tempData.sizes;
				auto &alignments	= _tempData.alignments;

				for(auto i = 0u; i < uboCount; i++)
				{
					auto &uboDst			= ubos				[i];// = nullptr;
					auto &buffer 			= buffers			[i] = VK_NULL_HANDLE;
					auto &memory			= memories		[i] = VK_NULL_HANDLE;
					auto &descriptor	= descriptors	[i];
					auto &size				= sizes				[i];
					auto &alignment		= alignments	[i] = 0; // TODO: decide for alignment

					create(logicalDevice, size, _usageFlags, buffer);
					allocMemory(
						_device, _usageFlags, _memProps,
						buffer, alignment, memory
					);

					descriptor.offset	= 0;
					descriptor.buffer	= buffer;
					descriptor.range	= VK_WHOLE_SIZE;

					bindMemory(logicalDevice, buffer, memory);
					mapMemory(logicalDevice, memory, &uboDst);
				}
			}

			template<uint16_t mbtCount>
			static void create(
				const std::unique_ptr<vk::Device>	&_device,
				typename MeshData<mbtCount>::Temp	&_tempData,
				MeshData<mbtCount>								&_meshBufferData,
				const VkBufferUsageFlags					&_usageFlags	= VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				const VkMemoryPropertyFlags				&_memProps		=	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
																													VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			) noexcept
			{
				const auto &deviceData = _device->getData();
				const auto &logicalDevice = deviceData.logicalDevice;

				auto &meshBuffers	= _meshBufferData.buffers;
				auto &mbMemories	= _meshBufferData.memories;

				auto &mbSizes			= _tempData.sizes;
				auto &mbData 			= _tempData.data;

				Array<VkDeviceSize, mbtCount> alignments; // TODO: temporary

				for(auto i = 0u; i < mbtCount; i++)
				{
					auto &buffer		= meshBuffers	[i];
					auto &memory		= mbMemories	[i];
					auto &size			= mbSizes			[i];
					auto &dataSrc		= mbData			[i];
					auto &alignment = alignments	[i] = 0;

					ASSERT(dataSrc != nullptr, "Buffer data source is NULL!");

					create(logicalDevice, size, _usageFlags, buffer);
					allocMemory(
						_device, _usageFlags, _memProps,
						buffer, alignment, memory
					);

					void *dataDst;

					mapMemory(logicalDevice, memory, &dataDst);
						memcpy(dataDst, dataSrc, size);

						if((_memProps & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
						{
							VkMappedMemoryRange mappedRange = {};
							mappedRange.sType		= VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
							mappedRange.memory	= memory;
							mappedRange.offset	= 0;
							mappedRange.size		= size;
							vkFlushMappedMemoryRanges(logicalDevice, 1, &mappedRange);
						}
					unmapMemory(logicalDevice, memory);
					bindMemory(logicalDevice, buffer, memory);
				}
			}

			template<uint16_t mbtCount>
			static void create(
				const std::unique_ptr<vk::Device>	&_device,
				Array<VkDeviceSize, mbtCount>			&_sizes,
				MeshData<mbtCount>								&_meshBufferData,
				const VkBufferUsageFlags					&_usageFlags	= VK_BUFFER_USAGE_TRANSFER_DST_BIT | 0,
				const VkMemoryPropertyFlags				&_memProps		=	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			) noexcept
			{
				const auto &deviceData = _device->getData();
				const auto &logicalDevice = deviceData.logicalDevice;

				auto &meshBuffers	= _meshBufferData.buffers;
				auto &mbMemories	= _meshBufferData.memories;

				Array<VkDeviceSize,				mbtCount> alignments; // TODO: temporary
				Array<VkBufferUsageFlags,	mbtCount> usageFlags;

				usageFlags[0] = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				usageFlags[1] = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

				for(auto i = 0u; i < mbtCount; i++)
				{
					auto &buffer		= meshBuffers	[i];
					auto &memory		= mbMemories	[i];
					auto &size			= _sizes			[i];
					auto &alignment = alignments	[i] = 0;

					create(logicalDevice, size, usageFlags[i] | _usageFlags, buffer);
					allocMemory(
						_device, _usageFlags, _memProps,
						buffer, alignment, memory
					);

					bindMemory(logicalDevice, buffer, memory);
				}
			}

			static void destroy(
				const VkDevice	&_logicalDevice,
				const VkBuffer	&_buffer
			) noexcept;

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
			static void unmapMemory(
				const VkDevice	&_logicalDevice,
				VkDeviceMemory	&_memory
			)	noexcept;
	};
}