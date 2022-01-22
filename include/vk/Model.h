#pragma once

#include <glm/gtc/type_ptr.hpp>

#include "Buffer.h"
#include "Pipeline.h"
#include "Material.h"

namespace vk
{
	class Device;
	class Model
	{
		public:
			static const uint16_t s_modelCount;

		public:
			enum class ID							: uint16_t;
			enum class RenderingMode	: uint16_t
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
				enum class Attribute : uint16_t
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

				static constexpr const Array<const char*, toInt(Attribute::_count_)> attrKeys = {
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
				uint32_t	vtxCount = 0;

				int				matIndex = 0;
			};

			struct Mesh
			{
				std::string_view				name;
				std::vector<Primitive>	primitives;
			};

			struct Node
			{
				std::string_view			name;
				NodePtr								parent;
				Mesh									mesh;
				std::vector<NodePtr>	children;
				glm::mat4 						matrix;
				glm::vec3							translation;
				glm::vec3							scale { 1.0f };
				glm::quat 						rotation;

				uint32_t							index;
				int										skinIndex = -1;
			};

			using VertexAttr = Vertex::Attribute;

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

				uint32_t imageSamplerCount	= 0;
				uint32_t meshCount					= 0;
				uint32_t textureCount				= 0;
			};

		public:
			template<Buffer::Type type, uint16_t bufferCount>
			inline static void setup(
				const std::unique_ptr<Device>		&_device,
				Data 														&_data,
				Buffer::Data<type, bufferCount>	&_bufferData
			) noexcept
			{
				Buffer::assertModelBuffers<type, bufferCount>();

				setupBuffers(_device, _data.vertices, _data.indices, _bufferData);
				// setup descriptors
			}

			// @todo make this implementation macro-based instead of generic draw function for custom api configuration
			template<
				RenderingMode	renderMode,
				Buffer::Type type, uint16_t bufferCount,
				uint16_t pipelineLayoutCount = 1, uint16_t pushConstCount = 0, uint16_t shaderModCount = 0
			>
			inline static void draw(
				const VkCommandBuffer																											&_cmdBuffer,
				const Vector<Data>																												&_modelsData,
				Buffer::Data<type, bufferCount>																						&_bufferData,
				const Pipeline::Data<shaderModCount, pipelineLayoutCount, pushConstCount>	&_pipelineData,
				const Vector<VkDescriptorSet> 																						&_descSets,
				uint16_t																																	_matFirstSetIdx		= 0,
				uint16_t																																	_matFirstPipeIdx	= 0,
				const IndexParams																													&_indexParams			= { 0, 0, 1, 0 }
			) noexcept
			{
				using BufferType = Buffer::Type;

				Buffer::assertModelBuffers<type, bufferCount>();

				auto &buffers = _bufferData.buffers;
				auto idxCount = _bufferData.entryCounts[BufferType::INDEX];

//				Command::bindPipeline(_cmdBuffer, _pipelines[1]);

				Command::bindVtxBuffers(_cmdBuffer, buffers[BufferType::VERTEX], 0);
				Command::bindIdxBuffer(_cmdBuffer, buffers[BufferType::INDEX], 0);

				for(const auto &modelData : _modelsData)
				{
					switch(renderMode)
					{
						case RenderingMode::PER_PRIMITIVE:
						{
							for(auto &node : modelData.nodes)
							{
								drawNode(
									_cmdBuffer, _descSets, _pipelineData,
									node, _matFirstSetIdx, _matFirstPipeIdx
								);
							}
						}
							break;
						case RenderingMode::PER_MODEL:
						{
							for(const auto &pipeLayoutDescSet : _pipelineData.pipeLayoutDescSets)
							{
								const auto &pipelineLayout = _pipelineData.layouts[pipeLayoutDescSet.layoutIndex];
//								const auto &descSets = _descSets

								Command::bindDescSets(
									_cmdBuffer,
									_descSets.data(),
									_descSets.size(),
									nullptr, 0,
									pipelineLayout,
									_matFirstSetIdx
								);
							}

							Command::drawIndexed(
								_cmdBuffer, idxCount,
								_indexParams.firstIndex,		_indexParams.vtxOffset,
								_indexParams.instanceCount,	_indexParams.firstInstance
							);
						}
							break;
					}
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
				using BufferData = Buffer::Data<type, Buffer::s_mbtCount>;

				BufferData			stagingBufferData;
				VkCommandBuffer	copyCmd;

				auto &deviceData		= _device->getData();
				auto &logicalDevice	= deviceData.logicalDevice;
				auto &cmdPool = deviceData.cmdData.cmdPool;
				auto inData = BufferData::Temp::create();

				auto &sizes				= inData.sizes;

				auto &cpuBuffers	= stagingBufferData.buffers;
				auto &gpuBuffers	= _bufferData.buffers;

				createBuffers(
					logicalDevice, deviceData.memProps,
					_vertices, _indices,
					inData, stagingBufferData, _bufferData
				);
				setupBuffersCopyCmd(
					logicalDevice,
					sizes,
					cpuBuffers, gpuBuffers,
					cmdPool, copyCmd
				);
				Command::submitStagingCopyCommand(
					logicalDevice,
					copyCmd,
					cmdPool, deviceData.graphicsQueue,
					"Model Buffers Copy"
				);
				Buffer::destroy(logicalDevice, stagingBufferData);
			}

			template<Buffer::Type type, uint16_t bufferCount>
			static void createBuffers(
				const VkDevice																				&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties								&_memProps,
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

				auto vtxCount			= _vertices.size();
				auto idxCount			= _indices.size();

				sizes		[BufferType::VERTEX]	= vtxCount * sizeof(Vertex);
				sizes		[BufferType::INDEX]		= idxCount * sizeof(uint32_t);

				entries	[BufferType::VERTEX]	= _vertices.data();
				entries	[BufferType::INDEX]		= _indices.data();

				counts	[BufferType::VERTEX]	= static_cast<uint32_t>(vtxCount);
				counts	[BufferType::INDEX]		= static_cast<uint32_t>(idxCount);

				Buffer::create<type, Buffer::s_mbtCount>(_logicalDevice, _memProps, _inData, _cpuBufferData.buffers, _cpuBufferData.memories);
				Buffer::create<type, bufferCount>(_logicalDevice, _memProps, sizes, alignments, _gpuBufferData.buffers, _gpuBufferData.memories);
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

					for(auto i = 0; i < Buffer::s_mbtCount; ++i)
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
				Command::record							(_copyCmd, recordCallback);
			}

		private:
			template<uint16_t shaderModCount, uint16_t pipelineLayoutCount, uint16_t pushConstCount>
			static void drawNode(
				const VkCommandBuffer																											&_cmdBuffer,
				const Vector<VkDescriptorSet>																							&_descSets,
				const Pipeline::Data<shaderModCount, pipelineLayoutCount, pushConstCount> &_pipelineData,
				const NodePtr																															&_node,
				uint16_t																																	_matFirstSetIdx		= 0,
				uint16_t																																	_matFirstPipeIdx	= 0
			) noexcept
			{
				const auto &mesh	= _node->mesh;
				auto nodeMtx			= _node->matrix;
				auto curParent		= _node->parent;

				while(curParent)
				{
					nodeMtx		= curParent->matrix * nodeMtx;
					curParent	= curParent->parent;
				}

				if(!_pipelineData.pushConstRanges.empty())
				{
					Command::setPushConstants(
						_cmdBuffer, _pipelineData.layouts[0],
						&nodeMtx, _pipelineData.pushConstRanges[0]
					);
				}

				for(const auto &primitive : mesh.primitives)
				{
					const auto &primIdxParams = primitive.indexParams;
					const auto &matSet = _descSets[_matFirstSetIdx + primitive.matIndex];
					const auto &matPipeline = _pipelineData.pipelines[_matFirstPipeIdx + primitive.matIndex];

					if(primitive.idxCount == 0) { continue; }

//			DEBUG_LOG("idxCount: %d\nfirstIndex: %d", primitive.idxCount, primIdxParams.firstIndex);
					Command::bindPipeline(_cmdBuffer, matPipeline);

					Command::bindDescSets(
						_cmdBuffer,
						&matSet, 1,
						nullptr, 0,
						_pipelineData.layouts[0]//, _matFirstSetIdx
					);

					Command::drawIndexed(
						_cmdBuffer,
						primitive.idxCount,
						primIdxParams.firstIndex,			primIdxParams.vtxOffset,
						primIdxParams.instanceCount,	primIdxParams.firstInstance
					);
				}

				for(const auto &child : _node->children)
				{
					drawNode(
						_cmdBuffer, _descSets, _pipelineData,
						child, _matFirstSetIdx, _matFirstPipeIdx
					);
				}
			}
	};
}