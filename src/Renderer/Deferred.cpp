#include "Renderer/Deferred.h"

namespace renderer
{
	void Deferred::init() noexcept
	{
		Base::init();

		loadAssets();
		initCmdBuffer();
		initSyncPrimitive();

		setupRenderPass();
		setupFramebuffer();
		setupUBOs();
		setupDescriptors();
		setupPipelines();

		setupCommands();

		m_screenData.isInited = true;
	}

	void Deferred::loadAssets() noexcept
	{
		using Model = vk::Model;

		m_screenData.modelsData.resize(Model::s_modelCount);

		loadAsset<Model::ID::SPONZA, Model::RenderingMode::PER_PRIMITIVE>(m_offscreenData.bufferData);
	}

	void Deferred::initCmdBuffer() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &logicalDevice	= deviceData.logicalDevice;
		auto &cmdData = deviceData.cmdData;
		auto &cmdPool = cmdData.cmdPool;

		vk::Command::allocateCmdBuffers(
			logicalDevice,
			cmdPool,
			&m_offscreenData.cmdBuffer
		);
	}

	void Deferred::initSyncPrimitive() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &logicalDevice	= deviceData.logicalDevice;

		vk::Sync::createSemaphore(
			logicalDevice,
			m_offscreenData.semaphore
		);
	}

	void Deferred::setupRenderPass() noexcept
	{
		using Att				= vk::Attachment;
		using AttType 	= Att::Type;
		using AttTag		= Att::Tag;
		using AttColor	= AttTag::Color;

		auto &deviceData			= m_device->getData();
		auto &swapchainData		= deviceData.swapchainData;
		auto &framebufferData = m_offscreenData.framebufferData;
		auto &attachmentsData = framebufferData.attachments;
		auto &formats					= attachmentsData.formats;

		auto &attCount		= OffScreenData::s_fbAttCount;
		auto &spCount 		= vk::RenderPass::s_subpassCount;
		auto &spDepCount	= vk::RenderPass::s_spDepCount;

		auto tempRPData			= OffScreenData::RenderPassData::create();
		auto &attSpMaps			= tempRPData.attSpMaps;
		auto &dependencies	= tempRPData.deps;
		auto &subpasses			= tempRPData.subpasses;

		const auto DEPTH		= attCount - 1; // 3

		attachmentsData.extent = swapchainData.extent;

		formats[AttColor::POSITION]	= VK_FORMAT_R16G16B16A16_SFLOAT;
		formats[AttColor::NORMAL]		= VK_FORMAT_R16G16B16A16_SFLOAT;
		formats[AttColor::ALBEDO]		= VK_FORMAT_R8G8B8A8_UNORM;
		formats[DEPTH]							= deviceData.depthFormat;

		const Att::AttSubpassMap &colorAttSpMap		= { AttType::COLOR,	{ 0 } };

		attSpMaps[AttColor::POSITION]	= colorAttSpMap;
		attSpMaps[AttColor::NORMAL]		= colorAttSpMap;
		attSpMaps[AttColor::ALBEDO]		= colorAttSpMap;
		attSpMaps[DEPTH]							= { AttType::DEPTH,	{ 0 } };

		// TODO
		dependencies[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass			= 0;
		dependencies[0].srcStageMask		= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask		= VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags	= VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass			= 0;
		dependencies[1].dstSubpass			= VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask		= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask		= VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags	= VK_DEPENDENCY_BY_REGION_BIT;

		vk::Framebuffer::createAttachments<attCount>(
			deviceData.logicalDevice,
			deviceData.memProps,
			attSpMaps,
			attachmentsData
		);

		vk::RenderPass::createSubpasses<attCount, spCount>(
			attSpMaps,
			subpasses
		);

		vk::RenderPass::create<attCount, spCount, spDepCount>(
			deviceData.logicalDevice,
			attachmentsData.descs,
			subpasses,
			dependencies,
			framebufferData.renderPass
		);
	}

	void Deferred::setupFramebuffer() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &framebufferData = getFbData(m_offscreenData);
		auto &attachmentsData = framebufferData.attachments;
		auto samplerInfo = vk::Image::Data::SamplerInfo::create();

		auto fbInfo = vk::Framebuffer::setFramebufferInfo(
			framebufferData.renderPass,
			attachmentsData.extent,
			attachmentsData.imageViews
		);

		vk::Framebuffer::create(
			deviceData.logicalDevice,
			fbInfo,
			framebufferData.framebuffer
		);

		attachmentsData.samplers.resize(1);
		vk::Image::createSampler(deviceData.logicalDevice, samplerInfo, attachmentsData.samplers[0]);
	}

	void Deferred::setupUBOs() noexcept
	{
		using BufferCategory	= vk::Buffer::Category;
		using BufferType			= vk::Buffer::Type;

		const auto &deviceData = m_device->getData();
		auto &bufferData = m_offscreenData.bufferData;
		auto tempData = vk::Buffer::TempData<BufferType::UNIFORM, vk::Buffer::s_ubcCount>::create();
		auto &sizes = tempData.sizes;

		// TODO: Parameterize this data through the UI

		CompositionUBO compositionUBO = {};
		OffScreenUBO offscreenUBO = {};

		setCompositionUBOData(compositionUBO);
		setOffscreenUBOData(offscreenUBO);

		sizes	[BufferCategory::COMPOSITION]	= sizeof(compositionUBO);
		sizes	[BufferCategory::OFFSCREEN]		= sizeof(offscreenUBO);

		vk::Buffer::create(
			deviceData.logicalDevice, deviceData.memProps,
			tempData,
			bufferData
		);

		updateOffscreenUBO();
		updateCompositionUBO();
	}

	void Deferred::setOffscreenUBOData(OffScreenUBO &_data) noexcept
	{
		const auto &cameraData = m_screenData.camera.getData();
		const auto &matrices = cameraData.matrices;

		_data = {
			matrices.perspective,
			matrices.view
		};
	}

	void Deferred::setCompositionUBOData(CompositionUBO &_data) noexcept
	{
		// TODO: Fix light values
		const auto &cameraData = m_screenData.camera.getData();
		_data = {
			{
				{
					glm::vec4(0.0f, 2.5f, 0.0f, 1.0f),
					glm::vec3(1.0f, 0.7f, 0.3f),
					25.0f
				},
//				{
//					glm::vec4((1.f - 2.5f) * 50.0f, -10.0f, 0.0f, 1.0f),
//					glm::vec3(1.0f, 0.7f, 0.7f),
//					120.0f
//				},
//				{
//					glm::vec4((2.f - 2.5f) * 50.0f, -10.0f, 0.0f, 1.0f),
//					glm::vec3(1.0f, 0.0f, 0.0f),
//					120.0f
//				},
//				{
//					glm::vec4((3.f - 2.5f) * 50.0f, -10.0f, 0.0f, 1.0f),
//					glm::vec3(0.0f, 0.0f, 1.0f),
//					120.0f
//				},
//				{
//					glm::vec4((4.f - 2.5f) * 50.0f, -10.0f, 0.0f, 1.0f),
//					glm::vec3(1.0f, 0.0f, 0.0f),
//					120.0f
//				}
			},
			// Current View Position
			glm::vec4(cameraData.pos, 1.0f)// * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f)
		};

	}

	void Deferred::updateOffscreenUBO() noexcept
	{
		using BufferCategory	= vk::Buffer::Category;

		auto &ubos = m_offscreenData.bufferData.entries;

		OffScreenUBO offscreenUBO = {};
		setOffscreenUBOData(offscreenUBO);

		memcpy(ubos[BufferCategory::OFFSCREEN], &offscreenUBO, sizeof(offscreenUBO));
	}

	void Deferred::updateCompositionUBO() noexcept
	{
		using BufferCategory	= vk::Buffer::Category;

		auto &ubos = m_offscreenData.bufferData.entries;

		CompositionUBO compositionUBO = {};
		setCompositionUBOData(compositionUBO);

		memcpy(ubos[BufferCategory::COMPOSITION], &compositionUBO, sizeof(compositionUBO));
	}

	void Deferred::setupDescPool(
		uint32_t _materialCount,
		uint32_t _maxSetCount
	) noexcept
	{
		using DescType				= vk::descriptor::Type;
		using Desc						= vk::Descriptor;
		using TextureMap			= vk::Texture::Sampler;

		auto &logicalDevice		= m_device->getData().logicalDevice;
		auto &descriptorData	= m_offscreenData.descriptorData;
		auto texMapCount = vk::toInt(TextureMap::_count_);

		const auto &poolSizes = {
			Desc::createPoolSize(DescType::UNIFORM_BUFFER, 2),
			Desc::createPoolSize(DescType::COMBINED_IMAGE_SAMPLER, _materialCount * texMapCount) // @todo: texture maps per material
		};

		Desc::createPool(
			logicalDevice,
			poolSizes,
			_maxSetCount,
			descriptorData.pool
		);
	}

	void Deferred::setupDescriptors() noexcept
	{
		using DescType				= vk::descriptor::Type;
		using BufferCategory	= vk::Buffer::Category;
		using TextureParam		= vk::Material::TexParam;
		using Att							= vk::Attachment;
		using Desc						= vk::Descriptor;

		using AttTag					= Att::Tag;
		using AttColor				= AttTag::Color;

		auto &logicalDevice			= m_device->getData().logicalDevice;
		auto &attachmentData		= m_offscreenData.framebufferData.attachments;
		auto &descriptorData		= m_offscreenData.descriptorData;
		auto &setLayouts				= descriptorData.setLayouts;
		auto &bufferData				= m_offscreenData.bufferData;
		auto &modelsData				= m_screenData.modelsData;
		auto &bufferInfos				= bufferData.descriptors;
		auto &descSets					= descriptorData.sets;

		auto tempData = OffScreenData::DescriptorData::Temp::create();
		auto &descriptors					= tempData.descriptors;

		for(const auto &model : modelsData)
		{ tempData.maxSetCount += model.imageCount; tempData.materialCount += model.materials.size(); }

		setupDescPool(tempData.materialCount, tempData.maxSetCount);

		const auto &dsLayoutBindings = GET_DESC_SET_LAYOUT_BINDINGS(DEFERRED_SHADING)

		const uint16_t GEOM_VS_UBO			= 0;
		uint16_t COLOR;
		const uint16_t POSITION = COLOR	= 1;
		const uint16_t NORMAL						= 2;
		const uint16_t ALBEDO						= 3;
		const uint16_t LIGHT_FS_UBO			= 4;

		Desc::createSetLayout(
			logicalDevice,
			dsLayoutBindings,
			setLayouts[Desc::LayoutCategory::DEFERRED_SHADING]
		);

		descSets.resize(tempData.materialCount + 1 + 1); // @todo: 1 set for OFFSCREEN buffers + 1 set for COMPOSITION buffers + 1 per model images/textures

		auto setIndex = 0;

		// COMPOSITION Set (Deferred)
		{
			auto &set = descSets[setIndex];

			Desc::allocSets(
				logicalDevice,
				descriptorData.pool,
				&setLayouts[Desc::LayoutCategory::DEFERRED_SHADING],
				&set
			);

			auto attTempData = Att::Data<OffScreenData::s_fbAttCount>::Temp::create();
			auto colorAttCount = vk::toInt(AttTag::Color::_count_);
			auto &imageInfos = attTempData.imageDescs;

			for(auto i = 0u; i < colorAttCount; ++i)
			{
				auto &imageDescriptor = imageInfos[i];

				imageDescriptor.sampler			= attachmentData.samplers[0];
				imageDescriptor.imageView		= attachmentData.imageViews[i];
				imageDescriptor.imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			descriptors = {
				Desc::createDescriptor(set, dsLayoutBindings[POSITION],			&imageInfos	[AttColor::POSITION]),
				Desc::createDescriptor(set, dsLayoutBindings[NORMAL],				&imageInfos	[AttColor::NORMAL]),
				Desc::createDescriptor(set, dsLayoutBindings[ALBEDO],				&imageInfos	[AttColor::ALBEDO]),
				Desc::createDescriptor(set, dsLayoutBindings[LIGHT_FS_UBO],	&bufferInfos[BufferCategory::COMPOSITION])
			};

			Desc::updateSets(logicalDevice, descriptors);

			setIndex += 1;
		}

		// Camera Matrices Set + Models Matrices + Models Materials Sets (Offscreen)
		{
			for(auto i = 0u; i < vk::Model::s_modelCount; ++i)
			{
				const auto &materials = modelsData[i].materials;

				for(const auto &material : materials)
				{
					auto &set = descSets[setIndex];

					Desc::allocSets(
						logicalDevice,
						descriptorData.pool,
						&setLayouts[Desc::LayoutCategory::DEFERRED_SHADING],
						&set
					);

					const auto &imageInfos = material.descriptors;

					descriptors = {
						Desc::createDescriptor(set, dsLayoutBindings[GEOM_VS_UBO],	&bufferInfos[BufferCategory::OFFSCREEN]),
						Desc::createDescriptor(set, dsLayoutBindings[COLOR],				&imageInfos	[TextureParam::BASE_COLOR_TEXTURE]),
						Desc::createDescriptor(set, dsLayoutBindings[NORMAL],				&imageInfos	[TextureParam::NORMAL_TEXTURE])
					};

					Desc::updateSets(logicalDevice, descriptors);

					setIndex++;
				}
			}
		}
	}

	void Deferred::setupPipelines() noexcept
	{
		namespace lightingPassShader	= constants::shaders::lightingPass;
		namespace geometryPassShader	= constants::shaders::geometryPass;
		using ShaderStage							= vk::Shader::Stage;
		using PipelineType						= vk::Pipeline::Type;
		using Vertex									= vk::Model::Vertex;

		auto psoData					= vk::Pipeline::PSO::create();
		auto shaderData				= vk::Shader::Data<vk::toInt(ShaderStage::_count_)>();

		auto &deviceData			= m_device->getData();
		auto &logicalDevice		= deviceData.logicalDevice;
		auto &modelsData			= m_screenData.modelsData;
		auto &pipelineData		= m_offscreenData.pipelineData;
		auto &descriptorData	= m_offscreenData.descriptorData;
		auto &shaderStages		= shaderData.stages;

		// composition pipeline + per material pipeline (all descriptor sets - offscreen ubo set)
		pipelineData.pipelines.resize(descriptorData.sets.size() - 1);

		vk::Pipeline::createCache(logicalDevice, pipelineData.cache);

		VkPushConstantRange pushConstRange = {};

		pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstRange.offset			= 0;
		pushConstRange.size				= sizeof(vk::Model::Node::matrix);

		vk::Array<VkPushConstantRange, 1> pcRanges = {
			pushConstRange
		};

		vk::Pipeline::createLayout(
			logicalDevice,
			pcRanges,
			descriptorData.setLayouts, // single pipeline layout used for all desc set layouts
			pipelineData.layouts[0]
		);

		// Deferred (Lighting) Pass Pipeline

		setShader<ShaderStage::VERTEX>(lightingPassShader::vert, shaderData);
		setShader<ShaderStage::FRAGMENT>(lightingPassShader::frag, shaderData);

		psoData.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		psoData.rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		setPipeline<PipelineType::COMPOSITION>(psoData, shaderStages);

		// Geometry (MRT) Pass / Material Pipeline(s)

		setShader<ShaderStage::VERTEX>(geometryPassShader::vert, shaderData);
		setShader<ShaderStage::FRAGMENT>(geometryPassShader::frag, shaderData);

		psoData.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		psoData.vertexInputState.vertexBindingDescs = {
			{ 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }
		};
		psoData.vertexInputState.vertexAttrDescs = {
			{ 0, 0, vk::FormatType::R32G32B32_SFLOAT,			(uint32_t) offsetof(Vertex, position)	}, // Position
			{ 1, 0, vk::FormatType::R32G32_SFLOAT,				(uint32_t) offsetof(Vertex, texCoord)	}, // UV
			{ 2, 0, vk::FormatType::R32G32B32A32_SFLOAT,	(uint32_t) offsetof(Vertex, color)		}, // Color
			{ 3, 0, vk::FormatType::R32G32B32_SFLOAT,			(uint32_t) offsetof(Vertex, normal)		}, // Normal
			{ 4, 0, vk::FormatType::R32G32B32A32_SFLOAT,	(uint32_t) offsetof(Vertex, tangent)	}  // Tangent
		};
		psoData.colorBlendState.attachments = {
			vk::Pipeline::setColorBlendAttachment(), 	// POSITION
			vk::Pipeline::setColorBlendAttachment(),	// NORMAL
			vk::Pipeline::setColorBlendAttachment()		// ALBEDO
		};

		for(auto i = 0u; i < vk::Model::s_modelCount; ++i)
		{
			const auto &materials = modelsData[i].materials;
			const auto materialCount = materials.size();

			for(auto j = 0u; j < materialCount; ++j)
			{
				const auto &material = materials[j];

				struct SpecData
				{
					VkBool32	alphaMask;
					float 		alphaMaskCutoff;
				} specData
				{
					material.alphaMode == vk::Material::alphaModes[vk::Material::AlphaMode::MASK_],
					material.alphaCutoff
				};

				const VkSpecializationMapEntry specMapEntries[2] = {
					vk::Shader::setSpecializationMapEntry(0, offsetof(SpecData, alphaMask), sizeof(SpecData::alphaMask)),
					vk::Shader::setSpecializationMapEntry(1, offsetof(SpecData, alphaMaskCutoff), sizeof(SpecData::alphaMaskCutoff))
				};
				const auto specInfo = vk::Shader::setSpecializationInfo(
					&specData, sizeof(specData),
					specMapEntries, sizeof(specMapEntries) / sizeof(VkSpecializationMapEntry)
				);

				shaderStages[ShaderStage::FRAGMENT].pSpecializationInfo = &specInfo;
				psoData.rasterizationState.cullMode = material.doubleSided ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

				setPipeline<PipelineType::OFFSCREEN>(psoData, shaderStages, j + 1);
			}
		}
	}

	void Deferred::setupCommands() noexcept
	{
		using PipelineType = vk::Pipeline::Type;

		const auto &deviceData = m_device->getData();
		const auto &swapchainExtent = deviceData.swapchainData.extent;
		const auto &pipelineData = m_offscreenData.pipelineData;
		const auto &descriptorData = m_offscreenData.descriptorData;
		const auto &framebufferData = m_offscreenData.framebufferData;

		Base::setupCommands(
			pipelineData.pipelines[PipelineType::COMPOSITION],
			pipelineData.layouts	[0],
			&descriptorData.sets	[PipelineType::COMPOSITION]
		);

		const auto &rpCallback = [&](const VkCommandBuffer &_cmdBuffer)
		{ setupRenderPassCommands(_cmdBuffer); };

		const auto &recordCallback = [&](const VkCommandBuffer &_cmdBuffer)
		{
			vk::Command::recordRenderPassCommands(
				_cmdBuffer,
				swapchainExtent,
				framebufferData,
				rpCallback
			);
		};

		vk::Command::record(m_offscreenData.cmdBuffer, recordCallback);
	}

	void Deferred::setupRenderPassCommands(const VkCommandBuffer &_offScreenCmdBuffer) noexcept
	{
		const auto &deviceData = m_device->getData();
		const auto &swapchainExtent = deviceData.swapchainData.extent;
		const auto &descSets = m_offscreenData.descriptorData.sets;
		const auto &pipelineData = m_offscreenData.pipelineData;

		vk::Command::setViewport(_offScreenCmdBuffer,	swapchainExtent);
		vk::Command::setScissor (_offScreenCmdBuffer,	swapchainExtent);

		vk::Model::draw(
			m_screenData.modelsData,
			m_offscreenData.bufferData,
			_offScreenCmdBuffer,
			pipelineData.layouts[0],
			descSets,
			pipelineData.pipelines,
			1,	// 0: composition ubo set,	1+: offscreen ubo set + materials sets
			1		// 0: composition pipeline, 1+: offscreen material pipelines
		);
	}

	void Deferred::submitOffscreenToQueue() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &semaphores = deviceData.syncData.semaphores;

		VkSubmitInfo submitInfo = {};
		vk::Command::setSubmitInfo(
			&semaphores.presentComplete,
			&m_offscreenData.semaphore,
			&m_offscreenData.cmdBuffer,
			submitInfo
		);
		vk::Command::submitToQueue(
			graphicsQueue, submitInfo,
			"Offscreen"
		);
	}

	void Deferred::submitSceneToQueue() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &cmdData = deviceData.cmdData;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &semaphores = deviceData.syncData.semaphores;
		auto &activeFbIndex = deviceData.swapchainData.activeFbIndex;

		VkSubmitInfo submitInfo = {};
		vk::Command::setSubmitInfo(
			&m_offscreenData.semaphore,
			&semaphores.renderComplete,
			&cmdData.drawCmdBuffers[activeFbIndex],
			submitInfo
		);
		vk::Command::submitToQueue(
			graphicsQueue, submitInfo,
			"Scene Composition"
		);
	}

	void Deferred::draw() noexcept
	{
		beginFrame();

		submitOffscreenToQueue();	// geometry/offscreen pass (g-buffer)
		submitSceneToQueue();			// lighting/composition pass (deferred)

		endFrame();
	}

	void Deferred::render() noexcept
	{
		if(!m_screenData.isInited) return;

		draw();

		if(!m_screenData.isPaused)
		{
			updateCompositionUBO();
		}

		if(m_screenData.camera.getData().isUpdated)
		{
			updateOffscreenUBO();
		}
	}
}