#include "Renderer.h"

void Renderer::init(Window::WindowData &_winData) noexcept
{
	Window::WindowResize windowResize;

	glfwSetWindowUserPointer(m_window, &windowResize);

	const auto &surfaceCreateCallback = [=]()
	{
		return glfwCreateWindowSurface(
			m_device->getData().vkInstance, m_window,
			nullptr, &m_device->getData().surface
		);
	};
	const auto &surfaceExtensions = getSurfaceExtensions();

	m_device->createSurface(
		surfaceCreateCallback,
		surfaceExtensions,
		_winData.width, _winData.height
	);
	m_device->createDevice();

	initSwapchain();
	initSyncObjects();
	initGraphicsQueueSubmitInfo();

	glfwSetFramebufferSizeCallback(m_window, Window::WindowResize::resize);

}

void Renderer::initSwapchain() noexcept
{
	auto &deviceData = m_device->getData();
	auto &swapchainData   = deviceData.swapchainData;
	auto &swapchainImages = swapchainData.images;

	m_device->createSwapchainData(swapchainData);

	const auto &swapchainSize = swapchainImages.size();
	for(auto i = 0u; i < swapchainSize; i++)
	{
		vk::Image::createImageView(
			deviceData.logicalDevice,
			swapchainImages[i],
			swapchainData.format,
			swapchainData.imageViews[i]
		);
	}
}

void Renderer::initSyncObjects() noexcept
{
	auto &deviceData = m_device->getData();
	auto &device = deviceData.logicalDevice;
	auto &semaphores = deviceData.syncData.semaphores;

	vk::Sync::createSemaphore(device, semaphores.presentComplete);
	vk::Sync::createSemaphore(device, semaphores.renderComplete);
}

void Renderer::initGraphicsQueueSubmitInfo() noexcept
{
	auto &deviceData = m_device->getData();
	auto &submitInfo = deviceData.submitInfo;
	auto &semaphores = deviceData.syncData.semaphores;

	submitInfo.sType								= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask		= &deviceData.submitPipelineStages;
	submitInfo.waitSemaphoreCount		= 1;
	submitInfo.pWaitSemaphores			= &semaphores.presentComplete;
	submitInfo.signalSemaphoreCount	= 1;
	submitInfo.pSignalSemaphores		= &semaphores.renderComplete;
}

void Renderer::prepare() noexcept
{
	loadAssets();
}

void Renderer::render() noexcept
{
	draw();
}

void Renderer::loadAssets() noexcept
{
	m_model.load("sponza.obj");
}

void Renderer::submitOffscreenQueue() noexcept
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

void Renderer::submitSceneQueue() noexcept
{
	auto &deviceData = m_device->getData();
	auto &submitInfo = deviceData.submitInfo;
	auto &graphicsQueue = deviceData.graphicsQueue;
	auto &semaphores = deviceData.syncData.semaphores;

	submitInfo.pWaitSemaphores = &semaphores.offscreen;
	submitInfo.pSignalSemaphores = &semaphores.renderComplete;

	submitInfo.pCommandBuffers = &deviceData.cmdData.drawCmdBuffers[deviceData.swapchainData.currentBuffer];

	auto result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	ASSERT_VK(result, "Failed to submit Scene Draw graphics queue!");
}

void Renderer::draw() noexcept
{
	beginFrame();

	submitOffscreenQueue();
	submitSceneQueue();

	endFrame();
}

void Renderer::beginFrame() noexcept
{
	auto &deviceData = m_device->getData();
	auto &swapchainData = deviceData.swapchainData;
	auto &device = deviceData.logicalDevice;
	auto &swapchain = swapchainData.swapchain;
	auto &semaphores = deviceData.syncData.semaphores;

	vk::Swapchain::acquireNextImage(
		device, swapchain,
		semaphores.presentComplete, &swapchainData.currentBuffer
	);

}

void Renderer::endFrame() noexcept
{
	auto &deviceData = m_device->getData();
	auto &swapchainData = deviceData.swapchainData;
	auto &device = deviceData.logicalDevice;
	auto &graphicsQueue = deviceData.graphicsQueue;
	auto &swapchain = swapchainData.swapchain;
	auto &semaphores = deviceData.syncData.semaphores;

	vk::Swapchain::queuePresentImage(
		device, swapchain, graphicsQueue,
		semaphores.renderComplete, swapchainData.currentBuffer
	);
	auto result = vkQueueWaitIdle(graphicsQueue);
	ASSERT_VK(result, "Failed to wait for the graphics queue!");
}

std::vector<const char*> Renderer::getSurfaceExtensions() noexcept
{
	uint32_t glfwExtensionCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(
		&glfwExtensionCount
	);

	return
	{
		glfwExtensions,
		glfwExtensions + glfwExtensionCount
	};
}