#include "Renderer/Deferred.h"

namespace renderer
{
	void Deferred::init(Window::WindowData &_winData) noexcept
	{
		Base::init(_winData);

		loadAssets();
		initCmdBuffer();
		initSyncObject();

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

	void Deferred::setupCommands() noexcept
	{
		const auto &rpBeginInfo = m_offscreenData.renderPassData.beginInfo;
		const auto &rpCallback = [=](const VkCommandBuffer &_cmdBuffer)
		{ setupRenderPassCommands(rpBeginInfo, _cmdBuffer); };

		const auto &recordCallback = [=](const VkCommandBuffer &_cmdBuffer)
		{
			vk::Command::recordRenderPassCommands(
				_cmdBuffer,
				rpBeginInfo,
				rpCallback
			);
		};

		vk::Command::recordCmdBuffer(m_offscreenData.cmdBuffer, recordCallback);
	}

	void Deferred::setupRenderPassCommands(
		const vk::RenderPass::Data::BeginInfo	&_beginInfo,
		const VkCommandBuffer									&_cmdBuffer
	) noexcept
	{
		auto &swapchainExtent = _beginInfo.swapchainExtent;

		vk::Command::setViewport(_cmdBuffer,	swapchainExtent);
		vk::Command::setScissor(_cmdBuffer,		swapchainExtent);

		m_offscreenData.model.draw(_cmdBuffer);
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

		submitOffscreenQueue();
		submitSceneQueue();

		endFrame();
	}
}