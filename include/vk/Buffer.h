#pragma once

#include "utils.h"

namespace vk
{
	class Buffer
	{
		public:
			enum class Category : uint16_t;

			// @todo: work out if the enum values swap position, it should not affect the end result
			// the issue can rise if they're used as subscript index on different types of containers
			enum class Type : uint16_t
			{
				VERTEX	= 0,
				INDEX		= 1,

				UNIFORM = 2,

				// TODO: add other buffers (e.g. storage buffers)

				TEXTURE = 10,	// Staging (CPU) ONLY for Optimal Tiling

				ANY			= 11	// excl. TEXTURE
			};

		public:
			inline static const uint16_t s_mbtCount = 2;	// model buffer type: vtx & idx
			static const				uint16_t s_ubcCount;			// uniform buffer category
			static const 				uint16_t s_bufferCount;

		public:
			template<Type type, uint16_t count>
			struct Data
			{
				// @todo: move all enum asserts to main function
//				Data() { ASSERT_ENUMS(Category); }

				struct Temp
				{
					STACK_ONLY(Temp);

					Array<VkDeviceSize,					count>			sizes;
					Array<VkDeviceSize,					count>			alignments; 	// @todo: where to use?
					Array<void*,								count>			entries;			// ONLY for Staging Buffers (Model, Texture, etc.)
				};

				Array<void*,									count>			entries;			// ONLY for Uniform Buffers
				Array<VkBuffer,								count>			buffers;
				Array<VkDeviceMemory,					count>			memories;
				Array<VkDescriptorBufferInfo,	count>			descriptors;	// ONLY for UBCs (Uniform Buffer Categories)
				Array<uint32_t,								s_mbtCount>	entryCounts;	// ONLY for MBTs (Model Buffer Types)
			};

			template<Type type, uint16_t count>
			using TempData = typename Data<type, count>::Temp;

		public:
			template<Type type, uint16_t count>
			static void assertUniformBuffers() noexcept
			{
				static_assert(
					(type == Type::UNIFORM || type == Type::ANY)	&&
					type != Type::TEXTURE													&&
					(count == s_bufferCount || count == s_ubcCount),
					"This function only takes Uniform Buffers"
				);
			}

			template<Type type, uint16_t count>
			static void assertStagingBuffers() noexcept
			{
				static_assert(
					type != Type::UNIFORM &&
					(count == s_mbtCount || count == 1),
					"This function only takes Vertex, Index & Texture Buffers"
				);
			}

			template<Type type, uint16_t count>
			static void assertModelBuffers() noexcept
			{
				static_assert(
					type != Type::UNIFORM &&
					type != Type::TEXTURE &&
					(count == s_bufferCount || count == s_mbtCount),
					"This function only takes Vertex & Index Buffers"
				);
			}

		public:
			static void createMemory(
				const VkDevice													&_logicalDevice,
				const VkBufferUsageFlags								&_usageFlags,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkMemoryPropertyFlags							&_propFlags,
				const VkBuffer													&_buffer,
				VkDeviceSize 														&_bufferAlignment,
				VkDeviceMemory													&_memory,
				VkDeviceSize														_offset = 0
			)	noexcept;

			static void createMemory(
				const VkDevice													&_logicalDevice,
				const VkBufferUsageFlags								&_usageFlags,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkMemoryPropertyFlags							&_propFlags,
				const VkBuffer													&_buffer,
				VkDeviceSize 														&_bufferAlignment,
				VkDeviceMemory													&_memory,
				VkDeviceSize														&_memReqsSize,
				VkDeviceSize														_offset = 0
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
				const VkDevice																						&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties										&_memProps,
				TempData<Type::UNIFORM, s_ubcCount ? s_ubcCount : count>	&_inData,
				Data<type, count>																					&_outData
			) noexcept
			{
				INFO_LOG("Creating Uniform Buffer(s) (UBO)...");

				assertUniformBuffers<type, count>();

				const auto &usageFlags		=	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				const auto &memPropFlags	=	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
																		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				auto &sizes				= _inData.sizes;
				auto &alignments	= _inData.alignments;

				auto &entries			= _outData.entries;
				auto &buffers			= _outData.buffers;
				auto &memories		= _outData.memories;
				auto &descriptors	= _outData.descriptors;

				auto startIdx = count == s_ubcCount ? 0u : toInt(Type::UNIFORM);
				for(auto i = startIdx; i < startIdx + s_ubcCount; ++i)
				{
					auto ubcIdx = i - startIdx;

					auto &size				= sizes				[ubcIdx];
					auto &alignment		= alignments	[ubcIdx];
					auto &uboDst			= entries			[ubcIdx]	= nullptr;
					auto &buffer 			= buffers			[i]				= VK_NULL_HANDLE;
					auto &memory			= memories		[i]				= VK_NULL_HANDLE;
					auto &descriptor	= descriptors	[ubcIdx]	= {};

					create(_logicalDevice, size, usageFlags, buffer);
					createMemory(
						_logicalDevice, usageFlags,
						_memProps, memPropFlags,
						buffer, alignment, memory
					);
					Device::mapMemory(_logicalDevice, memory, &uboDst);

					descriptor.offset	= 0;
					descriptor.buffer	= buffer;
					descriptor.range	= VK_WHOLE_SIZE;
				}
			}

			// CPU Host Staging Buffer (Model & Texture)
			template<Type type, uint16_t count>
			static void create(
				const VkDevice													&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				TempData<type,				count>						&_inData,
				Array<VkBuffer,				count>						&_buffers,
				Array<VkDeviceMemory,	count>						&_memories
			) noexcept
			{
//				INFO_LOG("Creating Staging (CPU) Buffer(s) (type: %d)...", type);

				assertStagingBuffers<type, count>();

				const auto &usageFlags		=	VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				const auto &memPropFlags	=	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
																		 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				auto &sizes				= _inData.sizes;
				auto &alignments	= _inData.alignments;
				auto &entries			= _inData.entries;

				for(auto i = 0u; i < count; ++i)
				{
					auto &buffer		= _buffers		[i];
					auto &memory		= _memories		[i];
					auto &size			= sizes				[i];
					auto &dataSrc		= entries			[i];
					auto &alignment = alignments	[i];
					void *dataDst;

					ASSERT(dataSrc != nullptr, "Buffer data source is NULL!");

					create(_logicalDevice, size, usageFlags, buffer);
					createMemory(
						_logicalDevice, usageFlags,
						_memProps, memPropFlags,
						buffer, alignment, memory
					);
					Device::mapMemory(_logicalDevice, memory, &dataDst);
						memcpy(dataDst, dataSrc, size);

						if((memPropFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
						{
							VkMappedMemoryRange mappedRange = {};
							mappedRange.sType		= VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
							mappedRange.memory	= memory;
							mappedRange.offset	= 0;
							mappedRange.size		= size;
							vkFlushMappedMemoryRanges(_logicalDevice, 1, &mappedRange);
						}
					Device::unmapMemory(_logicalDevice, memory);
				}
			}

			// GPU Device Local Buffer (Model)
			template<Type type, uint16_t count>
			static void create(
				const VkDevice													&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				const Array<VkDeviceSize,	s_mbtCount>		&_sizes,
				Array<VkDeviceSize, 			s_mbtCount>		&_alignments,
				Array<VkBuffer,						count>				&_buffers,
				Array<VkDeviceMemory,			count>				&_memories
			) noexcept
			{
				INFO_LOG("Creating Device Local (GPU) Buffer(s) (Model)...");

				assertModelBuffers<type, count>();

				VkMemoryPropertyFlags mempPropFlags = 0; // @todo Device Local?

				const auto &usageFlags	= VK_BUFFER_USAGE_TRANSFER_DST_BIT | mempPropFlags;

				Array<VkBufferUsageFlags,	s_mbtCount> modelFlags;

				modelFlags[Type::VERTEX]	= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				modelFlags[Type::INDEX]		= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

				auto startIdx = count == s_mbtCount ? 0u : toInt(Type::VERTEX);
				for(auto i = startIdx; i < startIdx + s_mbtCount; ++i)
				{
					auto mbtIdx = i - startIdx;

					auto &size			= _sizes			[mbtIdx];
					auto &alignment = _alignments	[mbtIdx];
					auto &buffer		= _buffers		[i];
					auto &memory		= _memories		[i];

					create(_logicalDevice, size, modelFlags[mbtIdx] | usageFlags, buffer);
					createMemory(
						_logicalDevice, usageFlags,
						_memProps, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						buffer, alignment, memory
					);
				}
			}

			static void destroy(
				const VkDevice							&_logicalDevice,
				const VkBuffer							&_buffer,
				const VkAllocationCallbacks	*_pAllocator = nullptr
			) noexcept;

			template<Type type, uint16_t count>
			static void destroy(
				const VkDevice							&_logicalDevice,
				const Data<type, count>			&_data,
				const VkAllocationCallbacks	*_pAllocator = nullptr
			) noexcept
			{
//				INFO_LOG("Cleaning Buffer(s) & Memory(s) (type: %d)", type);

				for(auto b = 0u; b < count; ++b)
				{
					destroy						(_logicalDevice, _data.buffers	[b], _pAllocator);
					Device::freeMemory(_logicalDevice, _data.memories	[b], _pAllocator);
				}
			}

		private:
			static void createMemory(
				const VkDevice													&_logicalDevice,
				const VkBufferUsageFlags								&_usageFlags,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkMemoryPropertyFlags							&_propFlags,
				const VkBuffer													&_buffer,
				VkMemoryRequirements										&_memReqs,
				VkDeviceMemory													&_memory,
				VkDeviceSize														_offset = 0
			)	noexcept;
			static void bindMemory(
				const VkDevice				&_logicalDevice,
				const VkBuffer				&_buffer,
				const VkDeviceMemory	&_memory,
				VkDeviceSize					_offset = 0
			) noexcept;
	};
}