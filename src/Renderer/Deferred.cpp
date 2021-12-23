#include "Renderer/Deferred.h"

namespace renderer
{
	void Deferred::init(Window::WindowData &_winData) noexcept
	{
		Base::init(_winData);

		loadAssets();
		initCmdBuffer();
		initSyncObject();
	}

	void Deferred::initCmdBuffer() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &logicalDevice	= deviceData.logicalDevice;
		auto &cmdData = deviceData.cmdData;
		auto &cmdPool = cmdData.cmdPool;
		auto &offscreenCmdBuffer = cmdData.offscreenCmdBuffer;

		vk::Command::allocateCmdBuffers(
			logicalDevice,
			cmdPool,
			&offscreenCmdBuffer,
			1
		);
	}

	void Deferred::initSyncObject() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &logicalDevice	= deviceData.logicalDevice;
		auto &semaphores = deviceData.syncData.semaphores;

		vk::Sync::createSemaphore(
			logicalDevice,
			semaphores.offscreen
		);
	}

	void Deferred::submitOffscreenQueue() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &submitInfo = deviceData.submitInfo;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &semaphores = deviceData.syncData.semaphores;

		submitInfo.pWaitSemaphores = &semaphores.presentComplete;
		submitInfo.pSignalSemaphores = &semaphores.offscreen;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &deviceData.cmdData.offscreenCmdBuffer;

		auto result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		ASSERT_VK(result, "Failed to submit Offscreen graphics queue!");
	}

	void Deferred::submitSceneQueue() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &submitInfo = deviceData.submitInfo;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &semaphores = deviceData.syncData.semaphores;
		auto &currentBuffer = deviceData.swapchainData.currentBuffer;

		submitInfo.pWaitSemaphores = &semaphores.offscreen;
		submitInfo.pSignalSemaphores = &semaphores.renderComplete;

		submitInfo.pCommandBuffers = &deviceData.cmdData.drawCmdBuffers[currentBuffer];

		auto result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		ASSERT_VK(result, "Failed to submit Scene Draw graphics queue!");
	}

	void Deferred::draw() noexcept
	{
		beginFrame();

		submitOffscreenQueue();
		submitSceneQueue();

		endFrame();
	}
}