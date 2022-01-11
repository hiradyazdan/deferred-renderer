#pragma once

#include <glm/gtc/type_ptr.hpp>

#include "Buffer.h"
#include "Material.h"

namespace vk
{
	class Model
	{
		public:
			static const uint16_t s_modelCount;

		public:
			enum class ID : uint16_t;
			enum class RenderingMode : uint16_t
			{
				PER_PRIMITIVE = 0,
				PER_MODEL			= 1
			};

		public:
			struct Node;
			struct Mesh;
			using NodePtr = std::shared_ptr<Node>;
			using MeshPtr = std::unique_ptr<Mesh>;

			struct Vertex
			{
				enum class Attributes : uint16_t
				{
					POS			= 0,
					NORMAL	= 1,
					UV			= 2,
					COLOR		= 3,
					TANGENT	= 4,
					JOINTS	= 5,
					WEIGHTS	= 6,

					_count_ = 7
				};

				static constexpr const Array<const char*, toInt(Attributes::_count_)> vtxAttrs = {
					"POSITION",
					"NORMAL",
					"TEXCOORD_0",
					"COLOR_0",
					"TANGENT",
					"JOINTS_0",
					"WEIGHTS_0"
				};

				glm::vec3 position;
				glm::vec3 normal;
				glm::vec2 texCoord;
				glm::vec3 color;
				glm::vec4 tangent;
				glm::vec4 joint0;
				glm::vec4 weight0;
			};

			struct IndexParams
			{
				uint32_t 	firstIndex		= 0;
				int				vtxOffset			= 0;
				uint32_t 	instanceCount	= 1;
				uint32_t 	firstInstance	= 0;
			};

			struct Primitive
			{
				IndexParams 		indexParams;
//		uint32_t	firstVertex;

				uint32_t	idxCount = 0;
//		uint32_t	vtxCount;

				int				matIndex = 0;
			};

			struct Mesh
			{
				std::string_view				name;
				std::vector<Primitive>	primitives;
			};

			struct Node
			{
				NodePtr								parent;
				MeshPtr								mesh;
				uint32_t							index;
				std::vector<NodePtr>	children;
				glm::mat4 						matrix;
				std::string_view			name;
				int										skinIndex = -1;
				glm::vec3							translation;
				glm::vec3							scale { 1.0f };
				glm::quat 						rotation;
			};

			using VertexAttr = Vertex::Attributes;

			struct Data
			{
				struct Temp
				{
					STACK_ONLY(Temp);

						Array<const float*, toInt(VertexAttr::_count_)> buffers = {};
				};

				std::vector<NodePtr>	nodes;
				std::vector<NodePtr>	linearNodes;
				std::vector<Material>	materials;
				std::vector<Vertex>		vertices;
				std::vector<uint32_t>	indices;

				RenderingMode					renderMode;
				IndexParams 					indexParams;

				uint32_t imageSamplerCount	= 0;
				uint32_t meshCount					= 0;
			};

		public:
			template<RenderingMode renderMode, Buffer::Type type, uint16_t bufferCount>
			inline static void setup(
				const std::unique_ptr<Device>		&_device,
				Data 														&_data,
				Buffer::Data<type, bufferCount>	&_bufferData,
				uint32_t												_instanceCount	= 1,
				uint32_t												_firstInstance	= 0,
				uint32_t												_firstIndex			= 0,
				int															_vtxOffset			= 0
			) noexcept
			{
				Buffer::assertModelBuffers<type, bufferCount>();

				_data.renderMode	= renderMode;
				_data.indexParams	= {
					_firstIndex, _vtxOffset,
					_instanceCount, _firstInstance
				};

				setupBuffers(_device, _data.vertices, _data.indices, _bufferData);
				// setup descriptors
			}

			template<Buffer::Type type, uint16_t bufferCount>
			inline static void draw(
				const Vector<Data>							&_modelsData,
				Buffer::Data<type, bufferCount>	&_bufferData,
				const Vector<VkDescriptorSet> 	&_modelSets,
				const VkCommandBuffer						&_cmdBuffer,
				const VkPipeline								&_pipeline,
				const VkPipelineLayout					&_pipelineLayout,
				uint16_t												_modelSetsStartIndex = 0
			) noexcept
			{
				using BufferType = Buffer::Type;

				Buffer::assertModelBuffers<type, bufferCount>();

				auto &buffers = _bufferData.buffers;
				auto idxCount = _bufferData.entryCounts[BufferType::INDEX];

				Command::bindPipeline(_cmdBuffer, _pipeline);

				// @todo: implement both bind per primitive & per model
				Command::bindDescSets(
					_cmdBuffer,
					_modelSets.data(),
					_modelSets.size(),
					nullptr, 0,
					_pipelineLayout,
					_modelSetsStartIndex
				);

				Command::bindVtxBuffers(_cmdBuffer, buffers[BufferType::VERTEX], 0);
				Command::bindIdxBuffer(_cmdBuffer, buffers[BufferType::INDEX], 0);

				for(const auto &modelData : _modelsData)
				{
					switch(modelData.renderMode)
					{
						case RenderingMode::PER_PRIMITIVE:
						{
							for(auto &node : modelData.nodes)
							{ drawNode(_cmdBuffer, node); }
						}
							break;
						case RenderingMode::PER_MODEL:
						{
							const auto &indexParams = modelData.indexParams;

							Command::drawIndexed(
								_cmdBuffer, idxCount,
								indexParams.firstIndex,			indexParams.vtxOffset,
								indexParams.instanceCount,	indexParams.firstInstance
							);
						}
							break;
					}
				}
			}

		private:
			inline static void drawNode(
				const VkCommandBuffer	&_cmdBuffer,
				const NodePtr &_node
			) noexcept
			{
				const auto &mesh = _node->mesh;

				if(mesh)
				{
					for(const auto &primitive : mesh->primitives)
					{
						const auto &primIdxParams = primitive.indexParams;

						Command::drawIndexed(
							_cmdBuffer,
							primitive.idxCount,
							primIdxParams.firstIndex,			primIdxParams.vtxOffset,
							primIdxParams.instanceCount,	primIdxParams.firstInstance
						);
					}
				}

				for(const auto &child : _node->children)
				{
					drawNode(_cmdBuffer, child);
				}
			}

		private:
			template<Buffer::Type type, uint16_t bufferCount>
			static void setupBuffers(
				const std::unique_ptr<Device>		&_device,
				std::vector<Vertex>							&_vertices,
				std::vector<uint32_t>						&_indices,
				Buffer::Data<type, bufferCount>	&_bufferData
			) noexcept
			{
				VkCommandBuffer copyCmd;
				VkFence fence;

				auto &deviceData		= _device->getData();
				auto &logicalDevice	= deviceData.logicalDevice;
				auto &cmdPool = deviceData.cmdData.cmdPool;
				auto inData = Buffer::Data<type, Buffer::s_mbtCount>::Temp::create();
				auto stagingBufferData = Buffer::Data<type, Buffer::s_mbtCount>();

				auto &sizes				= inData.sizes;

				auto &cpuBuffers	= stagingBufferData.buffers;
				auto &cpuMemories	= stagingBufferData.memories;

				auto &gpuBuffers	= _bufferData.buffers;

				createBuffers(
					_device,
					_vertices, _indices,
					inData, stagingBufferData, _bufferData
				);
				setupBuffersCopyCmd(
					logicalDevice,
					sizes,
					cpuBuffers, gpuBuffers,
					cmdPool, copyCmd
				);
				submitBuffersCopyCmd(
					logicalDevice,
					fence, copyCmd,
					deviceData.graphicsQueue
				);
				cleanUpBuffers(
					logicalDevice,
					fence, copyCmd, cmdPool,
					cpuBuffers, cpuMemories
				);
			}

			template<Buffer::Type type, uint16_t bufferCount>
			static void createBuffers(
				const std::unique_ptr<Device>													&_device,
				std::vector<Vertex>																		&_vertices,
				std::vector<uint32_t>																	&_indices,
				typename Buffer::Data<type, Buffer::s_mbtCount>::Temp	&_inData,
				Buffer::Data<type, Buffer::s_mbtCount>								&_cpuBufferData,
				Buffer::Data<type, bufferCount>												&_gpuBufferData
			) noexcept
			{
				using BufferType = Buffer::Type;

				auto &sizes				= _inData.sizes;
				auto &alignments	= _inData.alignments;
				auto &entries			= _inData.entries;

				auto &counts			= _gpuBufferData.entryCounts;

				auto vtxCount = _vertices.size();
				auto idxCount = _indices.size();

				sizes		[BufferType::VERTEX]	= vtxCount * sizeof(Vertex);
				sizes		[BufferType::INDEX]		= idxCount * sizeof(uint32_t);

				entries	[BufferType::VERTEX]	= _vertices.data();
				entries	[BufferType::INDEX]		= _indices.data();

				counts	[BufferType::VERTEX]	= static_cast<uint32_t>(vtxCount);
				counts	[BufferType::INDEX]		= static_cast<uint32_t>(idxCount);

				Buffer::create<type>(_device, _inData, _cpuBufferData.buffers, _cpuBufferData.memories);
				Buffer::create<type, bufferCount>(_device, sizes, alignments, _gpuBufferData.buffers, _gpuBufferData.memories);
			}

			template<uint16_t bufferCount>
			static void setupBuffersCopyCmd(
				const VkDevice																&_logicalDevice,
				const Array<VkDeviceSize,	Buffer::s_mbtCount>	&_bufferSizes,
				const Array<VkBuffer,			Buffer::s_mbtCount>	&_cpuBuffers,
				const Array<VkBuffer,			bufferCount>				&_gpuBuffers,
				const VkCommandPool														&_cmdPool,
				VkCommandBuffer																&_copyCmd
			) noexcept
			{
				const auto &recordCallback = [&](const VkCommandBuffer &_cmdBuffer)
				{
					VkBufferCopy region = {};

					for(auto i = 0; i < Buffer::s_mbtCount; i++)
					{
						region.size = _bufferSizes[i];
						Command::copyBuffer(
							_cmdBuffer,
							_cpuBuffers[i], _gpuBuffers[i + toInt(Buffer::Type::VERTEX)],
							&region
						);
					}
				};

				Command::allocateCmdBuffers	(_logicalDevice, _cmdPool, &_copyCmd);
				Command::recordCmdBuffer		(_copyCmd, recordCallback);
			}

			inline static void submitBuffersCopyCmd(
				const VkDevice				&_logicalDevice,
				VkFence								&_fence,
				const VkCommandBuffer	&_cmdBuffer,
				const VkQueue					&_queue
			) noexcept
			{
				VkSubmitInfo submitInfo = {};

				Sync		::createFence		(_logicalDevice, _fence);
				Command	::setSubmitInfo	(&_cmdBuffer, submitInfo);
				Command	::submitQueue		(_queue, submitInfo, "Model Buffers Copy Over", _fence);
				Sync		::waitForFences	(_logicalDevice, &_fence);
			}

			template<uint16_t bufferCount>
			static void cleanUpBuffers(
				const VkDevice														&_logicalDevice,
				VkFence																		&_fence,
				VkCommandBuffer														&_cmdBuffer,
				const VkCommandPool												&_cmdPool,
				const Array<VkBuffer,				bufferCount>	&_buffers,
				const Array<VkDeviceMemory,	bufferCount>	&_memories
			) noexcept
			{
				Sync		::destroyFence			(_logicalDevice, _fence);
				Command	::destroyCmdBuffers	(_logicalDevice, _cmdPool, &_cmdBuffer);

				for(auto i = 0; i < bufferCount; i++)
				{
					Buffer::destroy		(_logicalDevice, _buffers[i]);
					Device::freeMemory(_logicalDevice, _memories[i]);
				}
			}
	};
}