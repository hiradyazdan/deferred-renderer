#include "Renderer/Deferred.h"

namespace renderer
{
	void Deferred::init(Window::WindowData &_winData) noexcept
	{
		Base::init(_winData);

		loadAssets();
		initCmdBuffer();
		initSyncObject();

		setupRenderPass();
		setupFramebuffer();
		setupUBOs();
		setupDescriptors();
		setupPipelines();

		setupCommands();
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
			&m_offscreenData.cmdBuffer,
			1
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
		using AttData = vk::Attachment::Data<OffScreenData::s_fbAttCount>;
		using AttType = AttData::Type;
		using AttTag = vk::Attachment::Tag;

		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &renderPassData = m_offscreenData.renderPassData;
		auto &framebufferData = renderPassData.framebufferData;
		auto &attachmentsData = framebufferData.attachments;
		auto &dependencies = renderPassData.deps;

		auto &attCount		= OffScreenData::s_fbAttCount;
		auto &spCount 		= OffScreenData::s_subpassCount;
		auto &spDepCount	= OffScreenData::s_spDepCount;

		auto POSITION	= static_cast<int>(AttTag::Color::POSITION);	// 0
		auto NORMAL		= static_cast<int>(AttTag::Color::NORMAL); 		// 1
		auto ALBEDO		= static_cast<int>(AttTag::Color::ALBEDO);		// 2
		auto DEPTH		= 3;

		attachmentsData.formats[POSITION]	= VK_FORMAT_R16G16B16A16_SFLOAT;
		attachmentsData.formats[NORMAL]		= VK_FORMAT_R16G16B16A16_SFLOAT;
		attachmentsData.formats[ALBEDO]		= VK_FORMAT_R8G8B8A8_UNORM;
		attachmentsData.formats[DEPTH] = deviceData.depthFormat;

		const auto &subpassIndices = std::vector<uint16_t>({ 0 });
		const AttData::AttSubpassMap &colorAttSpMap = { AttType::COLOR,	subpassIndices };

		attachmentsData.attSpMaps[POSITION]	= colorAttSpMap;
		attachmentsData.attSpMaps[NORMAL]		= colorAttSpMap;
		attachmentsData.attSpMaps[ALBEDO]		= colorAttSpMap;
		attachmentsData.attSpMaps[DEPTH] = { AttType::DEPTH,	subpassIndices };

		vk::Framebuffer::createAttachments<attCount>(
			deviceData.logicalDevice, deviceData.physicalDevice,
			deviceData.memProps,
			attachmentsData
		);

		vk::RenderPass::createSubpasses<attCount, spCount, spDepCount>(
			attachmentsData.attSpMaps,
			renderPassData.subpasses
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
			renderPassData.subpasses,
			renderPassData.deps,
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
		using BufferType	= vk::Buffer::Type;

		const auto uboCount = OffScreenData::s_uboCount;
		const auto COMPOSITION = static_cast<int>(BufferType::COMPOSITION);
		const auto OFFSCREEN = static_cast<int>(BufferType::OFFSCREEN);

		auto &bufferData = m_offscreenData.bufferData;
		auto &cameraData = m_offscreenData.camera.getData();
		auto tempData = vk::Buffer::Data<uboCount>::Temp();
		auto &ubos = bufferData.ubos;
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

		ubos	[COMPOSITION] = &compositionUBO;
		sizes	[COMPOSITION]	= sizeof(compositionUBO);

		ubos	[OFFSCREEN]		= &offscreenUBO;
		sizes	[OFFSCREEN]		= sizeof(offscreenUBO);

		vk::Buffer::create<uboCount>(
			m_device,
			tempData,
			bufferData
		);
	}

	void Deferred::setupDescriptors() noexcept
	{
		using Desc				= vk::Descriptor;
		using DescType		= vk::descriptor::Type;
		using BufferType	= vk::Buffer::Type;
		using StageFlag		= vk::shader::StageFlag;
		using AttTag			= vk::Attachment::Tag;

		auto tempData = Desc::Data<OffScreenData::s_descSetLayoutCount>::Temp();

		auto &logicalDevice = m_device->getData().logicalDevice;
		auto &descriptorData = m_offscreenData.descriptorData;
		auto &bufferData = m_offscreenData.bufferData;
		auto &modelData = m_offscreenData.model.getData();
		auto &bufferInfos = bufferData.descInfos;
		auto &descSets = descriptorData.sets;
		auto &layoutBindings = tempData.setLayoutBindings;
		auto &writeSets = tempData.writeSets;

		const auto VS_UBO = 0;
		const auto POSITION	= static_cast<int>(AttTag::Color::POSITION) + 1; // 1
		const auto NORMAL		= static_cast<int>(AttTag::Color::NORMAL)		+ 1; // 2
		const auto ALBEDO		= static_cast<int>(AttTag::Color::ALBEDO)		+ 1; // 3
		const auto FS_UBO = 4;

		const auto COMPOSITION = static_cast<int>(BufferType::COMPOSITION);
		const auto OFFSCREEN = static_cast<int>(BufferType::OFFSCREEN);

		descSets.resize(modelData.meshes.size() + 1);
		layoutBindings.resize(5);

		layoutBindings[VS_UBO]		= Desc::createSetLayoutBinding(DescType::UNIFORM_BUFFER, StageFlag::VERTEX, VS_UBO); // VS uniform buffer
		layoutBindings[POSITION]	= Desc::createSetLayoutBinding(DescType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT, POSITION); // Position / Color map
		layoutBindings[NORMAL]		= Desc::createSetLayoutBinding(DescType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT, NORMAL); 	// Normals  / Normal Map
		layoutBindings[ALBEDO]		= Desc::createSetLayoutBinding(DescType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT, ALBEDO); 	// Albedo
		layoutBindings[FS_UBO]		= Desc::createSetLayoutBinding(DescType::UNIFORM_BUFFER, StageFlag::FRAGMENT, FS_UBO); // FS uniform buffer

		setDescPool();
		setDescSetLayout(layoutBindings);

		// COMPOSITION Sets
		{
			auto &set = descSets[0];

			vk::Descriptor::allocSets(
				logicalDevice,
				descriptorData.pool,
				descriptorData.setLayouts.data(),
				&set
			);

			writeSets = {
//				Desc::createWriteSet(set, layoutBindings[POSITION], {}),
//				Desc::createWriteSet(set, layoutBindings[NORMAL], {}),
//				Desc::createWriteSet(set, layoutBindings[ALBEDO], {}),
				Desc::createWriteSet(set, layoutBindings[FS_UBO], &bufferInfos[COMPOSITION])
			};

			vk::Descriptor::updateSets(
				logicalDevice,
				writeSets
			);
		}

		// Models Sets (Offscreen)
		{
			auto descSetsSize = descSets.size();

			if(descSetsSize < 2) return;

			for(auto i = 1u; i <= descSetsSize; i++)
			{
				auto &set = descSets[i];

				vk::Descriptor::allocSets(
					logicalDevice,
					descriptorData.pool,
					descriptorData.setLayouts.data(),
					&set
				);

				writeSets = {
					Desc::createWriteSet(set, layoutBindings[VS_UBO], &bufferInfos[OFFSCREEN]),
//					Desc::createWriteSet(set, layoutBindings[POSITION], {}), // Color map
//					Desc::createWriteSet(set, layoutBindings[NORMAL], {})    // Normal map
				};

				vk::Descriptor::updateSets(
					logicalDevice,
					writeSets
				);
			}
		}
	}

	void Deferred::setDescPool() noexcept
	{
		using DescType = vk::descriptor::Type;

		auto &logicalDevice = m_device->getData().logicalDevice;
		auto &descriptorData = m_offscreenData.descriptorData;
		const auto &poolSizes = {
			vk::Descriptor::createPoolSize(DescType::UNIFORM_BUFFER, 8),
			vk::Descriptor::createPoolSize(DescType::COMBINED_IMAGE_SAMPLER, 9)
		};

		vk::Descriptor::createPool(
			logicalDevice,
			poolSizes,
			3,
			descriptorData.pool
	 	);
	}

	void Deferred::setDescSetLayout(
		std::vector<VkDescriptorSetLayoutBinding>	&_layoutBindings,
		uint32_t																	_index
	) noexcept
	{
		auto &logicalDevice = m_device->getData().logicalDevice;
		auto &descriptorData = m_offscreenData.descriptorData;

		vk::Descriptor::createSetLayout(
			logicalDevice,
			_layoutBindings,
			descriptorData.setLayouts[_index]
		);
	}

	void Deferred::setupPipelines() noexcept
	{
		namespace lightingPassShader	= constants::shaders::lightingPass;
		namespace geometryPassShader	= constants::shaders::geometryPass;
		using ShaderStage							= vk::Shader::Stage;
		using PipelineType						= vk::Pipeline::Type;

		auto &deviceData = m_device->getData();
		auto &logicalDevice = deviceData.logicalDevice;
		auto &pipelineData = m_offscreenData.pipelineData;
		auto &descriptorData = m_offscreenData.descriptorData;
		auto &shaderStageCount = OffScreenData::s_shaderStageCount;
		auto psoData = vk::Pipeline::PSO();
		auto shaderData = vk::Shader::Data();
		std::array<VkPipelineShaderStageCreateInfo, shaderStageCount> shaderStages = {};

		const auto VERTEX = static_cast<int>(ShaderStage::VERTEX);
		const auto FRAGMENT = static_cast<int>(ShaderStage::FRAGMENT);

		vk::Pipeline::createCache(logicalDevice, pipelineData.cache);
		vk::Pipeline::createLayout<OffScreenData::s_descSetLayoutCount>(
			logicalDevice,
			descriptorData.setLayouts,
			pipelineData.layouts[0]
		);

		// Deferred (Lighting) Pass Pipeline

		shaderStages[VERTEX] 		= setShader<ShaderStage::VERTEX>(lightingPassShader::vert, shaderData).stageInfo;
		shaderStages[FRAGMENT]	= setShader<ShaderStage::FRAGMENT>(lightingPassShader::frag, shaderData).stageInfo;

		psoData.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		setPipeline<PipelineType::COMPOSITION, shaderStageCount>(psoData, shaderStages);

		// Geometry (G-buffer) Pass Pipeline

		shaderStages[VERTEX]		= setShader<ShaderStage::VERTEX>(geometryPassShader::vert, shaderData).stageInfo;
		shaderStages[FRAGMENT]	= setShader<ShaderStage::FRAGMENT>(geometryPassShader::frag, shaderData).stageInfo;

		psoData.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		psoData.vertexInputState = {

		};
		psoData.colorBlendState.attachments = {

		};
		setPipeline<PipelineType::OFFSCREEN, shaderStageCount>(psoData, shaderStages, false);
	}

	void Deferred::setupCommands() noexcept
	{
		const auto &deviceData = m_device->getData();
		const auto &renderPassData = m_offscreenData.renderPassData;
		const auto &framebufferData = renderPassData.framebufferData;
		const auto &swapchainExtent = deviceData.swapchainData.extent;
		const auto &pipelineData = m_offscreenData.pipelineData;
		const auto &descriptorData = m_offscreenData.descriptorData;

		const auto COMPOSITION = static_cast<int>(vk::Pipeline::Type::COMPOSITION);

		Base::setupCommands(
			pipelineData.pipelines[COMPOSITION],
			pipelineData.layouts[0],
			&descriptorData.sets[COMPOSITION]
		);

		const auto &rpCallback = [&](const VkCommandBuffer &_cmdBuffer)
		{ setupRenderPassCommands(swapchainExtent, _cmdBuffer); };

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

	void Deferred::setupRenderPassCommands(
		const VkExtent2D					&_swapchainExtent,
		const VkCommandBuffer			&_cmdBuffer
	) noexcept
	{
		const auto &pipelineData = m_offscreenData.pipelineData;

		vk::Command::setViewport(_cmdBuffer,	_swapchainExtent);
		vk::Command::setScissor(_cmdBuffer,		_swapchainExtent);

		m_offscreenData.model.draw(
			_cmdBuffer,
			pipelineData.pipelines[static_cast<int>(vk::Pipeline::Type::OFFSCREEN)],
			pipelineData.layouts[0]
		);
	}

	void Deferred::submitOffscreenQueue() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &submitInfo = deviceData.cmdData.submitInfo;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &semaphores = deviceData.syncData.semaphores;

		submitInfo.pWaitSemaphores		= &semaphores.presentComplete;
		submitInfo.pSignalSemaphores	= &m_offscreenData.semaphore;

		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &m_offscreenData.cmdBuffer;

		vk::Command::submitQueue(graphicsQueue, submitInfo, "Offscreen");
	}

	void Deferred::submitSceneQueue() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &cmdData = deviceData.cmdData;
		auto &submitInfo = cmdData.submitInfo;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &semaphores = deviceData.syncData.semaphores;
		auto &currentBuffer = deviceData.swapchainData.currentBuffer;

		submitInfo.pWaitSemaphores		= &m_offscreenData.semaphore;
		submitInfo.pSignalSemaphores	= &semaphores.renderComplete;

		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &cmdData.drawCmdBuffers[currentBuffer];

		vk::Command::submitQueue(graphicsQueue, submitInfo, "Scene Composition");
	}

	void Deferred::draw() noexcept
	{
		beginFrame();

		submitOffscreenQueue(); // geometry pass (g-buffer)
		submitSceneQueue();			// lighting pass (deferred)

		endFrame();
	}
}