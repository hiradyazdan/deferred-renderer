#include "Renderer/Deferred.h"

namespace renderer
{
	void Deferred::init(Window::WindowData &_winData) noexcept
	{
		Base::init(_winData);

		loadAssets();
		initCmdBuffer();
		initSyncObject();

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

	void Deferred::setupDescriptors() noexcept
	{
		using DescType = vk::descriptor::Type;
		using StageFlag = vk::shader::StageFlag;

		auto setLayoutBindings = {
			vk::Descriptor::createSetLayoutBinding(DescType::UNIFORM_BUFFER, StageFlag::VERTEX, 0),
			vk::Descriptor::createSetLayoutBinding(DescType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT, 1),
			vk::Descriptor::createSetLayoutBinding(DescType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT, 2),
			vk::Descriptor::createSetLayoutBinding(DescType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT, 3),
			vk::Descriptor::createSetLayoutBinding(DescType::UNIFORM_BUFFER, StageFlag::FRAGMENT, 4)
		};

		setDescPool();
		setDescSetLayout(setLayoutBindings);
		setDescSet();
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
		std::initializer_list<VkDescriptorSetLayoutBinding> &_layoutBindings,
		uint32_t																						_index
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

	void Deferred::setDescSet() noexcept
	{

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

		vk::Pipeline::createCache(logicalDevice, pipelineData.cache);
		vk::Pipeline::createLayout<OffScreenData::s_descSetLayoutCount>(
			logicalDevice,
			descriptorData.setLayouts,
			pipelineData.layouts[0]
		);

		// Deferred (Lighting) Pass Pipeline

		shaderStages[0] = setShader<ShaderStage::VERTEX>(lightingPassShader::vert, shaderData).stageInfo;
		shaderStages[1] = setShader<ShaderStage::FRAGMENT>(lightingPassShader::frag, shaderData).stageInfo;

		psoData.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		setPipeline<PipelineType::COMPOSITION, shaderStageCount>(psoData, shaderStages);

		// Geometry (G-buffer) Pass Pipeline

		shaderStages[0] = setShader<ShaderStage::VERTEX>(geometryPassShader::vert, shaderData).stageInfo;
		shaderStages[1] = setShader<ShaderStage::FRAGMENT>(geometryPassShader::frag, shaderData).stageInfo;

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

		Base::setupCommands(
			pipelineData.pipelines[static_cast<int>(vk::Pipeline::Type::COMPOSITION)],
			pipelineData.layouts[0]
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