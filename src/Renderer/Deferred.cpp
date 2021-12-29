#include "Renderer/Deferred.h"

namespace renderer
{
	void Deferred::init(Window::WindowData &_winData) noexcept
	{
		Base::init(_winData);

		loadAssets();
		initCmdBuffer();
		initSyncObject();

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

	void Deferred::setupPipelines() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &pipelineData = m_offscreenData.pipelineData;
		auto psoData = vk::Pipeline::PSO();
		std::array<VkPipelineShaderStageCreateInfo, OffScreenData::s_shaderStageCount> shaderStages = {};

		vk::Pipeline::createCache(deviceData.logicalDevice, pipelineData.cache);

		// Deferred (Lighting) Pass Pipeline

		psoData.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		setPipeline<
			vk::Pipeline::Type::COMPOSITION,
			OffScreenData::s_shaderStageCount
		>(psoData, shaderStages);

		// Geometry (G-buffer) Pass Pipeline

		psoData.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		psoData.vertexInputState = {

		};
		psoData.colorBlendState.attachments = {

		};
		setPipeline<
		  vk::Pipeline::Type::OFFSCREEN,
			OffScreenData::s_shaderStageCount
		>(psoData, shaderStages, false);
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
			pipelineData.layout
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
			pipelineData.layout
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