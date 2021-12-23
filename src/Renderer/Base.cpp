#include "Renderer/Base.h"

namespace renderer
{
	void Base::init(Window::WindowData &_winData) noexcept
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
		initCommands();
		initSyncObjects();
		initGraphicsQueueSubmitInfo();

		glfwSetFramebufferSizeCallback(m_window, Window::WindowResize::resize);

	}

	void Base::initSwapchain() noexcept
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

	void Base::initSyncObjects() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &syncData = deviceData.syncData;
		auto &logicalDevice = deviceData.logicalDevice;
		auto &semaphores = syncData.semaphores;
		auto &fences = syncData.waitFences;

		vk::Sync::createSemaphore(logicalDevice, semaphores.presentComplete);
		vk::Sync::createSemaphore(logicalDevice, semaphores.renderComplete);

		fences.resize(deviceData.cmdData.drawCmdBuffers.size());
		for(auto &fence : fences)
		{
			vk::Sync::createFence(logicalDevice, fence, VK_FENCE_CREATE_SIGNALED_BIT);
		}
	}

	void Base::initGraphicsQueueSubmitInfo() noexcept
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

	void Base::initCommands() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &logicalDevice	= deviceData.logicalDevice;
		auto &queueFamilyIndex = deviceData.queueFamilyIndices.graphicsFamily;
		auto &cmdData = deviceData.cmdData;
		auto &cmdPool = cmdData.cmdPool;
		auto &drawCmdBuffers = cmdData.drawCmdBuffers;

		drawCmdBuffers.resize(deviceData.swapchainData.size);

		vk::Command::createPool(
			logicalDevice,
			queueFamilyIndex,
			cmdPool
		);
		vk::Command::allocateCmdBuffers(
			logicalDevice,
			cmdPool,
			drawCmdBuffers.data(),
			static_cast<uint32_t>(drawCmdBuffers.size())
		);
	}

	void Base::loadAssets() noexcept
	{
		m_model.load("sponza.obj");
	}

	void Base::render() noexcept
	{
		draw();
	}

	void Base::draw() noexcept
	{
		beginFrame();

		submitSceneQueue();

		endFrame();
	}

	void Base::submitSceneQueue() noexcept
	{

	}

	void Base::beginFrame() noexcept
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

	void Base::endFrame() noexcept
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

	std::vector<const char*> Base::getSurfaceExtensions() noexcept
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
}