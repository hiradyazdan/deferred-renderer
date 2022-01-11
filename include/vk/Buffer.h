#pragma once

#include "Device.h"

namespace vk
{
	class Buffer
	{
		public:
			enum class Category : uint16_t;

			enum class Type : uint16_t
			{
				VERTEX	= 0,
				INDEX		= 1,

				UNIFORM = 2,

				ANY			= 3
			};

		public:
			inline static const uint16_t s_mbtCount = 2;	// model buffer type: vtx & idx
			static const				uint16_t s_ubcCount;			// uniform buffer category
			static const 				uint16_t s_bufferCount;

		public:
			template<Type type, uint16_t count>
			struct Data
			{
				Data() { ASSERT_ENUMS(Category); }

				struct Temp
				{
					STACK_ONLY(Temp);

					Array<VkDeviceSize,					count>	sizes;
					Array<VkDeviceSize,					count>	alignments; // @todo: where to use?
					Array<void*,								count>	entries; // @todo: should be temp data?
				};

				Array<VkBuffer,								count>	buffers;
				Array<VkDeviceMemory,					count>	memories;
				Array<VkDescriptorBufferInfo,	s_ubcCount ? s_ubcCount : count>	descriptors; // ONLY for UBCs
				Array<uint32_t,								s_mbtCount>	entryCounts; // ONLY for MBTs
			};

			template<Type type, uint16_t count>
			using TempData = typename Data<type, count>::Temp;

		public:
			template<Type type, uint16_t count>
			static void assertUniformBuffers() noexcept
			{
				static_assert(
					(type == Type::UNIFORM || type == Type::ANY) &&
					(count == s_bufferCount || count == s_ubcCount),
					"This function only takes Uniform Buffers"
				);
			}

			template<Type type, uint16_t count>
			static void assertModelBuffers() noexcept
			{
				static_assert(
					type != Type::UNIFORM &&
					(count == s_bufferCount || count == s_mbtCount),
					"This function only takes Vertex & Index Buffers"
				);
			}

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

			// UBO
			template<Type type, uint16_t count>
			inline static void create(
				const std::unique_ptr<vk::Device>													&_device,
				TempData<Type::UNIFORM, s_ubcCount ? s_ubcCount : count>	&_inData,
				Data<type, count>																					&_outData
			) noexcept
			{
				INFO_LOG("Creating Uniform Buffers (UBOs)...");

				assertUniformBuffers<type, count>();

				const auto &deviceData		= _device->getData();
				const auto &logicalDevice	= deviceData.logicalDevice;
				const auto &usageFlags		=	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

				auto &sizes				= _inData.sizes;
				auto &alignments	= _inData.alignments;
				auto &entries			= _inData.entries;

				auto &buffers			= _outData.buffers;
				auto &memories		= _outData.memories;
				auto &descriptors	= _outData.descriptors;

				auto startIdx = count == s_ubcCount ? 0u : toInt(Type::UNIFORM);
				for(auto i = startIdx; i < startIdx + s_ubcCount; i++)
				{
					auto ubcIdx = i - startIdx;

					auto &size				= sizes				[ubcIdx];
					auto &alignment		= alignments	[ubcIdx];
					auto &uboDst			= entries			[ubcIdx];
					auto &buffer 			= buffers			[i] = VK_NULL_HANDLE;
					auto &memory			= memories		[i] = VK_NULL_HANDLE;
					auto &descriptor	= descriptors	[ubcIdx] = {};

					create(logicalDevice, size, usageFlags, buffer);
					allocMemory(
						_device, usageFlags,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						buffer, alignment, memory
					);

					descriptor.offset	= 0;
					descriptor.buffer	= buffer;
					descriptor.range	= VK_WHOLE_SIZE;

					bindMemory(logicalDevice, buffer, memory);
					mapMemory(logicalDevice, memory, &uboDst);
				}
			}

			// CPU Host Staging Buffer (Model/Scene)
			template<Type type>
			static void create(
				const std::unique_ptr<vk::Device>	&_device,
				TempData<type,				s_mbtCount>	&_inData,
				Array<VkBuffer,				s_mbtCount>	&_buffers,
				Array<VkDeviceMemory,	s_mbtCount>	&_memories
			) noexcept
			{
				INFO_LOG("Creating Staging (CPU) Buffers...");

				assertModelBuffers<type, s_mbtCount>();

				const auto &deviceData		= _device->getData();
				const auto &logicalDevice	= deviceData.logicalDevice;
				const auto &usageFlags		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				const auto &memProps			= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
															 			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				auto &sizes				= _inData.sizes;
				auto &alignments	= _inData.alignments;
				auto &entries			= _inData.entries;

				for(auto i = 0u; i < s_mbtCount; i++)
				{
					auto &buffer		= _buffers		[i];
					auto &memory		= _memories		[i];
					auto &size			= sizes				[i];
					auto &dataSrc		= entries			[i];
					auto &alignment = alignments	[i];

					ASSERT(dataSrc != nullptr, "Buffer data source is NULL!");

					create(logicalDevice, size, usageFlags, buffer);
					allocMemory(
						_device, usageFlags, memProps,
						buffer, alignment, memory
					);

					void *dataDst;

					mapMemory(logicalDevice, memory, &dataDst);
						memcpy(dataDst, dataSrc, size);

						if((memProps & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
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

			// GPU Device Local Buffer (Model/Scene)
			template<Type type, uint16_t count>
			static void create(
				const std::unique_ptr<vk::Device>			&_device,
				const Array<VkDeviceSize,	s_mbtCount>	&_sizes,
				Array<VkDeviceSize, 			s_mbtCount>	&_alignments,
				Array<VkBuffer,						count>			&_buffers,
				Array<VkDeviceMemory,			count>			&_memories
			) noexcept
			{
				INFO_LOG("Creating Device Local (GPU) Buffers...");

				assertModelBuffers<type, count>();

				VkMemoryPropertyFlags mempProps = 0; // @todo Device Local?

				const auto &deviceData = _device->getData();
				const auto &logicalDevice = deviceData.logicalDevice;
				const auto &usageFlags	= VK_BUFFER_USAGE_TRANSFER_DST_BIT | mempProps;

				Array<VkBufferUsageFlags,	s_mbtCount> modelFlags;

				modelFlags[0] = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				modelFlags[1] = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

				auto startIdx = count == s_mbtCount ? 0u : toInt(Type::VERTEX);
				for(auto i = startIdx; i < startIdx + s_mbtCount; i++)
				{
					auto mbtIdx = i - startIdx;

					auto &size			= _sizes			[mbtIdx];
					auto &alignment = _alignments	[mbtIdx];
					auto &buffer		= _buffers		[i];
					auto &memory		= _memories		[i];

					create(logicalDevice, size, modelFlags[mbtIdx] | usageFlags, buffer);
					allocMemory(
						_device, usageFlags,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
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