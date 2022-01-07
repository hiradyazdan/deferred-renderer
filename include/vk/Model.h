#pragma once

#include "utils.h"
#include "Buffer.h"
#include "Mesh.h"

namespace vk
{
	class Model
	{
		template<uint16_t mbtCount>
		using MeshData = Buffer::MeshData<mbtCount>;

		template<uint16_t mbtCount>
		using TempMeshData = typename MeshData<mbtCount>::Temp;

		public:
			static const uint16_t s_modelCount;

		public:
			enum class Name : uint16_t;

		public:
			struct Data
			{
				std::vector<Mesh>	meshes;
			};

		public:
			inline static void load(
				const std::unique_ptr<Device>	&_device,
				std::vector<Mesh>							&_meshes
			) noexcept
			{
				for(auto &mesh : _meshes)
				{
					setupMeshBuffers(_device, mesh);
					// setup descriptors
				}
			}

			inline static void draw(
				const std::vector<Mesh>				&_meshes,
				const Vector<VkDescriptorSet> &_meshSets,
				const VkCommandBuffer					&_cmdBuffer,
				const VkPipeline							&_pipeline,
				const VkPipelineLayout				&_pipelineLayout,
				uint16_t											_meshSetsStartIndex = 0
			) noexcept
			{
				Command::bindPipeline(
					_cmdBuffer,
					_pipeline
				);

				for(const auto &mesh : _meshes)
				{
					auto &meshBuffers = mesh.bufferData.buffers;

					Command::bindDescSets(
						_cmdBuffer,
						&_meshSets[_meshSetsStartIndex],
						1,
						nullptr, 0,
						_pipelineLayout
					);
					Command::bindVtxBuffers(
						_cmdBuffer, meshBuffers[Mesh::BufferType::VERTEX], 0
					);
					Command::bindIdxBuffer(
						_cmdBuffer, meshBuffers[Mesh::BufferType::INDEX], 0
					);
					Command::drawIndexed(
						_cmdBuffer,
						mesh.indices.size(),
						1
					);

					_meshSetsStartIndex++;
				}
			}

		private:
			template<uint16_t mbtCount = Mesh::s_bufferTypeCount>
			static void setupMeshBuffers(
				const std::unique_ptr<vk::Device>	&_device,
				Mesh &_mesh
			) noexcept
			{
				VkCommandBuffer copyCmd;
				VkFence fence;

				auto &deviceData		= _device->getData();
				auto &logicalDevice	= deviceData.logicalDevice;
				auto &cmdPool = deviceData.cmdData.cmdPool;
				auto &meshBufferData = _mesh.bufferData;
				auto stagingBufferData = MeshData<mbtCount>();
				auto tempData = TempMeshData<mbtCount>::create();

				createMeshBuffers<mbtCount>(
					_device,
					_mesh.vertices, _mesh.indices,
					tempData, stagingBufferData, meshBufferData
				);
				setupMeshBuffersCopyCmd<mbtCount>(
					logicalDevice,
					tempData.sizes,
					stagingBufferData.buffers, meshBufferData.buffers,
					cmdPool, copyCmd
				);
				submitMeshBuffersCopyCmd(
					logicalDevice,
					fence, copyCmd,
					deviceData.graphicsQueue
				);
				cleanUpBuffers<mbtCount>(
					logicalDevice,
					fence, copyCmd, cmdPool,
					stagingBufferData.buffers, stagingBufferData.memories
				);
			}

			template<uint16_t mbtCount>
			static void createMeshBuffers(
				const std::unique_ptr<vk::Device>	&_device,
				std::vector<Mesh::Vertex>					&_vertices,
				std::vector<uint32_t>							&_indices,
				TempMeshData<mbtCount>						&_tempBufferData,
				MeshData<mbtCount>								&_stagingBufferData,
				MeshData<mbtCount>								&_meshBufferData
			) noexcept
			{
				auto &bufferSizes		= _tempBufferData.sizes;

				bufferSizes[Mesh::BufferType::VERTEX]	= _vertices.size() * sizeof(Mesh::Vertex);
				bufferSizes[Mesh::BufferType::INDEX]	= _indices.size() * sizeof(uint32_t);

				_tempBufferData.data[Mesh::BufferType::VERTEX]	= _vertices.data();
				_tempBufferData.data[Mesh::BufferType::INDEX]		= _indices.data();

				Buffer::create<mbtCount>(_device, _tempBufferData, _stagingBufferData);
				Buffer::create<mbtCount>(_device, bufferSizes, _meshBufferData);
			}

			template<uint16_t mbtCount>
			static void setupMeshBuffersCopyCmd(
				const VkDevice											&_logicalDevice,
				const Array<VkDeviceSize,	mbtCount>	&_bufferSizes,
				const Array<VkBuffer,			mbtCount>	&_stageBuffers,
				const Array<VkBuffer,			mbtCount>	&_meshBuffers,
				const VkCommandPool									&_cmdPool,
				VkCommandBuffer											&_copyCmd
			) noexcept
			{
				const auto &recordCallback = [&](const VkCommandBuffer &_cmdBuffer)
				{
					VkBufferCopy region = {};

					for(auto i = 0u; i < mbtCount; i++)
					{
						region.size = _bufferSizes[i];
						vk::Command::copyBuffer(
							_cmdBuffer,
							_stageBuffers[i], _meshBuffers[i],
							&region
						);
					}
				};

				vk::Command::allocateCmdBuffers	(_logicalDevice, _cmdPool, &_copyCmd);
				vk::Command::recordCmdBuffer		(_copyCmd, recordCallback);
			}

			inline static void submitMeshBuffersCopyCmd(
				const VkDevice				&_logicalDevice,
				VkFence								&_fence,
				const VkCommandBuffer	&_cmdBuffer,
				const VkQueue					&_queue
			) noexcept
			{
				VkSubmitInfo submitInfo = {};

				vk::Sync		::createFence		(_logicalDevice, _fence);
				vk::Command	::setSubmitInfo	(&_cmdBuffer, submitInfo);
				vk::Command	::submitQueue		(_queue, submitInfo, "Mesh Buffers Copy Over", _fence);
				vk::Sync		::waitForFences	(_logicalDevice, &_fence);
			}

			template<uint16_t mbtCount>
			static void cleanUpBuffers(
				const VkDevice												&_logicalDevice,
				VkFence																&_fence,
				VkCommandBuffer												&_cmdBuffer,
				const VkCommandPool										&_cmdPool,
				const Array<VkBuffer,				mbtCount>	&_buffers,
				const Array<VkDeviceMemory,	mbtCount>	&_memories
			) noexcept
			{
				vk::Sync		::destroyFence			(_logicalDevice, _fence);
				vk::Command	::destroyCmdBuffers	(_logicalDevice, _cmdPool, &_cmdBuffer);

				for(auto i = 0u; i < mbtCount; i++)
				{
					vk::Buffer::destroy		(_logicalDevice, _buffers[i]);
					vk::Device::freeMemory(_logicalDevice, _memories[i]);
				}
			}
	};
}