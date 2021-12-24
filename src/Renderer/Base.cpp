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

		setupCommands();

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
		auto &submitInfo = deviceData.cmdData.submitInfo;
		auto &semaphores = deviceData.syncData.semaphores;

		submitInfo.sType								= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext                = nullptr;
		submitInfo.pWaitDstStageMask		= &m_screenData.pipelineData.pipelineStages;
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

	void Base::setupCommands() noexcept
	{
		auto &rpBeginInfo = m_screenData.renderPassData.beginInfo;
		auto &deviceData = m_device->getData();
		const auto &cmdData = deviceData.cmdData;

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

		// TODO: temporary to run. remove once implemented create framebuffer
		deviceData.swapchainData.framebuffers.resize(deviceData.swapchainData.size);

		auto &cmdBuffers = cmdData.drawCmdBuffers;
		for(auto i = 0u; i < cmdBuffers.size(); i++)
		{
			rpBeginInfo.framebuffer = deviceData.swapchainData.framebuffers[i];

			vk::Command::recordCmdBuffer(cmdBuffers[i], recordCallback);
		}
	}

	void Base::setupRenderPassCommands(
		const vk::RenderPass::Data::BeginInfo &_beginInfo,
		const VkCommandBuffer									&_cmdBuffer
	) noexcept
	{
		auto &swapchainExtent = _beginInfo.swapchainExtent;
		auto &pipelineData = m_screenData.pipelineData;

		vk::Command::setViewport(_cmdBuffer,	swapchainExtent);
		vk::Command::setScissor(_cmdBuffer,		swapchainExtent);

		vk::Command::bindPipeline(
			_cmdBuffer,
			pipelineData.pipeline
		);
		vk::Command::bindDescSets(
			_cmdBuffer,
			m_screenData.material->descriptorSets,
			nullptr, 0,
			pipelineData.pipelineLayout
		);
		vk::Command::draw(
			_cmdBuffer,
			3,
			1
		);
	}

	void Base::loadAssets() noexcept
	{
		m_screenData.model.load("sponza.obj");
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
		auto &deviceData = m_device->getData();
		auto &cmdData = deviceData.cmdData;
		auto &submitInfo = cmdData.submitInfo;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &semaphores = deviceData.syncData.semaphores;
		auto &currentBuffer = deviceData.swapchainData.currentBuffer;

		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &cmdData.drawCmdBuffers[currentBuffer];

		vk::Command::submitQueue(graphicsQueue, submitInfo, "Scene Composition");
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