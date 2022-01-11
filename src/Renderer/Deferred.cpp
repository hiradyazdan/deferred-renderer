#include "Renderer/Deferred.h"

namespace renderer
{
	void Deferred::init() noexcept
	{
		Base::init();

		loadAssets();
		initCmdBuffer();
		initSyncObject();

		setupRenderPass();
		setupFramebuffer();
		setupUBOs();
		setupDescriptors();
		setupPipelines();

		setupCommands();
		setupDeferredCommands();

		m_screenData.isInited = true;
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

	void Deferred::initSyncObject() noexcept
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
		using AttTag		= Att::Tag;
		using AttColor	= AttTag::Color;
		using AttData 	= Att::Data<OffScreenData::s_fbAttCount>;
		using AttType 	= AttData::Type;

		auto &deviceData			= m_device->getData();
		auto &swapchainData		= deviceData.swapchainData;
		auto &renderPassData	= m_offscreenData.renderPassData;
		auto &framebufferData = renderPassData.framebufferData;
		auto &attachmentsData = framebufferData.attachments;

		auto &attCount		= OffScreenData::s_fbAttCount;
		auto &spCount 		= vk::RenderPass::s_subpassCount;
		auto &spDepCount	= vk::RenderPass::s_spDepCount;

		auto tempRPData			= OffScreenData::RenderPassData::Temp::create();
		auto &dependencies	= tempRPData.deps;
		auto &subpasses			= tempRPData.subpasses;

		const auto DEPTH		= attCount - 1; // 3

		attachmentsData.extent = swapchainData.extent;

		attachmentsData.formats[AttColor::POSITION]	= VK_FORMAT_R16G16B16A16_SFLOAT;
		attachmentsData.formats[AttColor::NORMAL]		= VK_FORMAT_R16G16B16A16_SFLOAT;
		attachmentsData.formats[AttColor::ALBEDO]		= VK_FORMAT_R8G8B8A8_UNORM;
		attachmentsData.formats[DEPTH]							= deviceData.depthFormat;

		const auto &subpassIndices = std::vector<uint16_t>({ 0 });
		const AttData::AttSubpassMap &colorAttSpMap = { AttType::COLOR,	subpassIndices };

		attachmentsData.attSpMaps[AttColor::POSITION]	= colorAttSpMap;
		attachmentsData.attSpMaps[AttColor::NORMAL]		= colorAttSpMap;
		attachmentsData.attSpMaps[AttColor::ALBEDO]		= colorAttSpMap;
		attachmentsData.attSpMaps[DEPTH]							= { AttType::DEPTH,	subpassIndices };

		vk::Framebuffer::createAttachments<attCount>(
			deviceData.logicalDevice, deviceData.physicalDevice,
			deviceData.memProps,
			attachmentsData
		);

		vk::RenderPass::createSubpasses<attCount, spCount, spDepCount>(
			attachmentsData.attSpMaps,
			subpasses
		);

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
		auto &framebufferData = m_offscreenData.renderPassData.framebufferData;
		auto &attachmentsData = framebufferData.attachments;

		auto fbInfo = vk::Framebuffer::setFramebufferInfo<OffScreenData::s_fbAttCount>(
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
		vk::Image::createSampler(deviceData.logicalDevice, attachmentsData.samplers[0]);
	}

	void Deferred::setupUBOs() noexcept
	{
		using BufferCategory	= vk::Buffer::Category;
		using BufferType			= vk::Buffer::Type;

		auto &bufferData = m_offscreenData.bufferData;
		auto &cameraData = m_offscreenData.camera.getData();
		auto tempData = vk::Buffer::TempData<BufferType::UNIFORM, vk::Buffer::s_ubcCount>::create();
		auto &ubos = tempData.entries;
		auto &sizes = tempData.sizes;

		// TODO: Parameterize this data through the UI

		struct CompositionUBO
		{
			struct Light
			{
				glm::vec4	position;
				glm::vec3	color;
				float			radius;
			};

			Light			lights[6];
			glm::vec4	viewPos;
		};

		struct OffScreenUBO
		{
			glm::mat4 projection;
			glm::mat4 model;
			glm::mat4 view;
		};

		CompositionUBO compositionUBO = {
			{
				{
					glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
					glm::vec3(1.5f),
					15.0f * 0.25f
				},
				{
					glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f),
					glm::vec3(1.0f, 0.0f, 0.0f),
					15.0f
				},
				{
					glm::vec4(2.0f, -1.0f, 0.0f, 0.0f),
					glm::vec3(0.0f, 0.0f, 2.5f),
					5.0f
				},
				{
					glm::vec4(0.0f, -0.9f, 0.5f, 0.0f),
					glm::vec3(1.0f, 1.0f, 0.0f),
					2.0f
				},
				{
					glm::vec4(0.0f, -0.5f, 0.0f, 0.0f),
					glm::vec3(0.0f, 1.0f, 0.2f),
					5.0f
				},
				{
					glm::vec4(0.0f, -1.0f, 0.0f, 0.0f),
					glm::vec3(1.0f, 0.7f, 0.3f),
					25.0f
				}
			},
			glm::vec4(cameraData.pos, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f)
		};

		OffScreenUBO offscreenUBO = {
			cameraData.matrices.perspective,
			glm::mat4(1.0f),
			cameraData.matrices.view
		};

		ubos	[BufferCategory::COMPOSITION] = &compositionUBO;
		sizes	[BufferCategory::COMPOSITION]	= sizeof(compositionUBO);

		ubos	[BufferCategory::OFFSCREEN]		= &offscreenUBO;
		sizes	[BufferCategory::OFFSCREEN]		= sizeof(offscreenUBO);

		vk::Buffer::create(
			m_device,
			tempData,
			bufferData
		);
	}

	void Deferred::setupDescriptors() noexcept
	{
		using DescType				= vk::descriptor::Type;
		using BufferCategory	= vk::Buffer::Category;
		using TextureMap			= vk::Texture::Sampler;
		using StageFlag				= vk::shader::StageFlag;
		using Att							= vk::Attachment;
		using Desc						= vk::Descriptor;

		using AttTag					= Att::Tag;
		using AttColor				= AttTag::Color;
		using LayoutBinding		= Desc::LayoutBinding;

		auto tempData = OffScreenData::DescriptorData::Temp::create();

		auto &logicalDevice		= m_device->getData().logicalDevice;
		auto &attachmentData	= m_offscreenData.renderPassData.framebufferData.attachments;
		auto &descriptorData	= m_offscreenData.descriptorData;
		auto &bufferData			= m_offscreenData.bufferData;
//		auto &modelData				= m_screenData.modelData;
		auto &bufferInfos			= bufferData.descriptors;
		auto &descSets				= descriptorData.sets;
		auto &layoutBindings	= tempData.setLayoutBindings;
		auto &writeSets				= tempData.writeSets;

		auto matCount = 1;//modelData.materials.size();
//		auto meshCount = modelData.meshCount;

		auto texMapCount = vk::toInt(TextureMap::_count_);

		descSets.resize(vk::Model::s_modelCount + 1); // @todo: 1 set for COMPOSITION buffers + 1 per model images/textures

		const auto &poolSizes = {
			Desc::createPoolSize(DescType::UNIFORM_BUFFER, 8),
			Desc::createPoolSize(DescType::COMBINED_IMAGE_SAMPLER, matCount * texMapCount) // @todo: texture maps per material
		};
		Desc::createPool(
			logicalDevice,
			poolSizes,
			descSets.size(),
			descriptorData.pool
		);

		auto &DSLayoutBindings = layoutBindings[Desc::LayoutCategory::DEFERRED_SHADING];

		DSLayoutBindings[LayoutBinding::VS_UBO]		= Desc::createSetLayoutBinding<LayoutBinding::VS_UBO>(DescType::UNIFORM_BUFFER, StageFlag::VERTEX); // VS uniform buffer
		DSLayoutBindings[LayoutBinding::POSITION]	= Desc::createSetLayoutBinding<LayoutBinding::POSITION>(DescType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT); // Position / Color map
		DSLayoutBindings[LayoutBinding::NORMAL]		= Desc::createSetLayoutBinding<LayoutBinding::NORMAL>(DescType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT);	// Normals  / Normal Map
		DSLayoutBindings[LayoutBinding::ALBEDO]		= Desc::createSetLayoutBinding<LayoutBinding::ALBEDO>(DescType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT);	// Albedo
		DSLayoutBindings[LayoutBinding::FS_UBO]		= Desc::createSetLayoutBinding<LayoutBinding::FS_UBO>(DescType::UNIFORM_BUFFER, StageFlag::FRAGMENT); // FS uniform buffer

		Desc::createSetLayout<Desc::s_layoutBindingCount>(
			logicalDevice,
			DSLayoutBindings,
			descriptorData.setLayouts[Desc::LayoutCategory::DEFERRED_SHADING]
		);

		// COMPOSITION Sets (Deferred)
		{
			auto &set = descSets[BufferCategory::COMPOSITION];

			Desc::allocSets(
				logicalDevice,
				descriptorData.pool,
				descriptorData.setLayouts.data(),
				&set
			);

			auto attTempData = Att::Data<OffScreenData::s_fbAttCount>::Temp::create();
			auto colorAttCount = vk::toInt(AttTag::Color::_count_);
			auto &imageInfos = attTempData.imageDescs;

			for(auto i = 0u; i < colorAttCount; i++)
			{
				auto &imageDescriptor = imageInfos[i];

				imageDescriptor.sampler			= attachmentData.samplers[0];
				imageDescriptor.imageView		= attachmentData.imageViews[i];
				imageDescriptor.imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			writeSets = {
				Desc::createWriteSet(set, DSLayoutBindings[LayoutBinding::POSITION],	&imageInfos	[AttColor::POSITION]),
				Desc::createWriteSet(set, DSLayoutBindings[LayoutBinding::NORMAL],		&imageInfos	[AttColor::NORMAL]),
				Desc::createWriteSet(set, DSLayoutBindings[LayoutBinding::ALBEDO],		&imageInfos	[AttColor::ALBEDO]),
				Desc::createWriteSet(set, DSLayoutBindings[LayoutBinding::FS_UBO],		&bufferInfos[BufferCategory::COMPOSITION])
			};

			Desc::updateSets(logicalDevice, writeSets);
		}

		// Models Materials Sets (Offscreen)
		{
			auto imageInfos = m_offscreenData.textureData.imageInfos;

			for(auto i = 1u; i < descSets.size(); i++)
			{
				auto &set = descSets[i];

				Desc::allocSets(
					logicalDevice,
					descriptorData.pool,
					descriptorData.setLayouts.data(),
					&set
				);

				writeSets = {
					Desc::createWriteSet(set, DSLayoutBindings[LayoutBinding::VS_UBO],		&bufferInfos[BufferCategory::OFFSCREEN]),
					Desc::createWriteSet(set, DSLayoutBindings[LayoutBinding::POSITION],	&imageInfos	[TextureMap::COLOR]),
					Desc::createWriteSet(set, DSLayoutBindings[LayoutBinding::NORMAL],		&imageInfos	[TextureMap::NORMAL])
				};

				Desc::updateSets(logicalDevice, writeSets);
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

		const auto shaderStageCount = vk::toInt(ShaderStage::_count_);
		auto psoData					= vk::Pipeline::PSO::create();
		auto shaderData				= vk::Shader::Data<shaderStageCount>();

		auto &deviceData			= m_device->getData();
		auto &logicalDevice		= deviceData.logicalDevice;
		auto &pipelineData		= m_offscreenData.pipelineData;
		auto &descriptorData	= m_offscreenData.descriptorData;
		auto &shaderStages		= shaderData.stages;

		vk::Pipeline::createCache(logicalDevice, pipelineData.cache);
		vk::Pipeline::createLayout<vk::Descriptor::s_setLayoutCount>(
			logicalDevice,
			descriptorData.setLayouts,
			pipelineData.layouts[0]
		);

		// Deferred (Lighting) Pass Pipeline

		setShader<ShaderStage::VERTEX>(lightingPassShader::vert, shaderData);
		setShader<ShaderStage::FRAGMENT>(lightingPassShader::frag, shaderData);

		psoData.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		setPipeline<PipelineType::COMPOSITION, shaderStageCount>(psoData, shaderStages);

		// Geometry (G-buffer) Pass Pipeline

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
		setPipeline<PipelineType::OFFSCREEN, shaderStageCount>(psoData, shaderStages);
	}

	void Deferred::setupCommands() noexcept
	{
		using PipelineType = vk::Pipeline::Type;

		const auto &pipelineData = m_offscreenData.pipelineData;
		const auto &descriptorData = m_offscreenData.descriptorData;

		Base::setupCommands(
			pipelineData.pipelines[PipelineType::COMPOSITION],
			pipelineData.layouts	[0],
			&descriptorData.sets	[PipelineType::COMPOSITION]
		);
	}

	void Deferred::setupDeferredCommands() noexcept
	{
		const auto &deviceData = m_device->getData();
		const auto &renderPassData = m_offscreenData.renderPassData;
		const auto &framebufferData = renderPassData.framebufferData;
		const auto &swapchainExtent = deviceData.swapchainData.extent;

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

		vk::Command::recordCmdBuffer(m_offscreenData.cmdBuffer, recordCallback);
	}

	void Deferred::setupRenderPassCommands(const VkCommandBuffer &_offScreenCmdBuffer) noexcept
	{
		using PipelineType = vk::Pipeline::Type;

		const auto &deviceData = m_device->getData();
		const auto &swapchainExtent = deviceData.swapchainData.extent;
		const auto &descSets = m_offscreenData.descriptorData.sets;
		const auto &pipelineData = m_offscreenData.pipelineData;

		vk::Command::setViewport(_offScreenCmdBuffer,	swapchainExtent);
		vk::Command::setScissor(_offScreenCmdBuffer,		swapchainExtent);

		vk::Model::draw(
			m_screenData.modelsData,
			m_offscreenData.bufferData,
			descSets,
			_offScreenCmdBuffer,
			pipelineData.pipelines[PipelineType::OFFSCREEN],
			pipelineData.layouts[0], 1
		);
	}

	void Deferred::loadAssets() noexcept
	{
		using Model = vk::Model;

		m_screenData.modelsData.resize(Model::s_modelCount);

		loadAsset<Model::ID::SPONZA, Model::RenderingMode::PER_PRIMITIVE>(
			m_offscreenData.bufferData
		);
	}

	void Deferred::submitOffscreenQueue() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &submitInfo = deviceData.cmdData.submitInfo;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &semaphores = deviceData.syncData.semaphores;

		vk::Command::setSubmitInfo(
			&semaphores.presentComplete,
			&m_offscreenData.semaphore,
			&m_offscreenData.cmdBuffer,
			submitInfo
		);
		vk::Command::submitQueue(
			graphicsQueue, submitInfo,
			"Offscreen"
		);
	}

	void Deferred::submitSceneQueue() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &cmdData = deviceData.cmdData;
		auto &submitInfo = cmdData.submitInfo;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &semaphores = deviceData.syncData.semaphores;
		auto &currentBuffer = deviceData.swapchainData.currentBuffer;

		vk::Command::setSubmitInfo(
			&m_offscreenData.semaphore,
			&semaphores.renderComplete,
			&cmdData.drawCmdBuffers[currentBuffer],
			submitInfo
		);
		vk::Command::submitQueue(
			graphicsQueue, submitInfo,
			"Scene Composition"
		);
	}

	void Deferred::draw() noexcept
	{
		beginFrame();

		submitOffscreenQueue(); // geometry pass (g-buffer)
		submitSceneQueue();			// lighting pass (deferred)

		endFrame();
	}

	void Deferred::render() noexcept
	{
		if(!m_screenData.isInited) return;

		draw();

		if(m_screenData.isPaused)
		{
			// TODO: update composition (deferred / lighting pass) ubos
		}

		if(m_screenData.camera.getData().isUpdated)
		{
			// TODO: update offscreen (geometry pass) ubos
		}
	}
}