#include "Renderer/Base.h"

namespace renderer
{
	void Base::init(Window::WindowData &_winData) noexcept
	{
		Window::WindowResize windowResize;
		auto &deviceData = m_device->getData();

		glfwSetWindowUserPointer(m_window, &windowResize);

		const auto &surfaceCreateCallback = [&]()
		{
			return glfwCreateWindowSurface(
				deviceData.vkInstance, m_window,
				nullptr, &deviceData.surface
			);
		};
		const auto &surfaceExtensions = getSurfaceExtensions();

		m_device->createSurface(
			surfaceCreateCallback,
			surfaceExtensions,
			_winData.width, _winData.height
		);
		m_device->createDevice();
		m_device->createSwapchainData(deviceData.swapchainData);

		initCommands();
		initSyncObjects();
		initGraphicsQueueSubmitInfo();
		setupRenderPass();
		setupFramebuffer();
		setupPipeline();

		setupCommands();

		glfwSetFramebufferSizeCallback(m_window, Window::WindowResize::resize);

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

	void Base::setupRenderPass() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &renderPassData = m_screenData.renderPassData;
		auto &framebufferData = renderPassData.framebufferData;
		auto &attachments = framebufferData.attachments;
		auto &dependencies = renderPassData.deps;

		attachments.formats = {
			swapchainData.format,
			deviceData.depthFormat
		};

		vk::Framebuffer::createAttachments<ScreenData::s_fbAttCount>(
			deviceData.logicalDevice, deviceData.physicalDevice,
			swapchainData.extent, deviceData.memProps,
			attachments
		);

		vk::RenderPass::createSubpasses<
		  ScreenData::s_fbAttCount,
			ScreenData::s_spDescCount,
			ScreenData::s_spDepCount
		>(
			attachments.attSpMaps,
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
		dependencies[1].dstStageMask 		= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask 	= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask		= VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags	= VK_DEPENDENCY_BY_REGION_BIT;

		vk::RenderPass::create<
		  ScreenData::s_fbAttCount,
			ScreenData::s_spDescCount,
			ScreenData::s_spDepCount
		>(
			deviceData.logicalDevice,
			attachments.descs,
			renderPassData.subpasses,
			renderPassData.deps,
			framebufferData.renderPass
		);
	}

	void Base::setupFramebuffer() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &framebufferData = m_screenData.renderPassData.framebufferData;
		auto &framebuffers = swapchainData.framebuffers;
		const auto &framebuffersSize = swapchainData.size;

		std::array<VkImageView, ScreenData::s_fbAttCount> imageViews = {}; // framebuffer attachments

		auto fbInfo = vk::Framebuffer::setFramebufferInfo<ScreenData::s_fbAttCount>(
			framebufferData.renderPass,
			swapchainData.extent,
			imageViews
		);

		for(auto j = 1; j < ScreenData::s_fbAttCount; j++)
		{
			imageViews[j] = framebufferData.attachments.imageViews[j];
		}

		framebuffers.resize(framebuffersSize);
		for(auto i = 0u; i < framebuffersSize; i++)
		{
			imageViews[0] = swapchainData.imageViews[i];

//			fbInfo.pAttachments = imageViews.data();

			vk::Framebuffer::create(
				deviceData.logicalDevice,
				fbInfo,
				framebuffers[i]
			);
		}
	}

	void Base::setupPipeline() noexcept
	{

	}

	void Base::setupCommands() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &renderPassData = m_screenData.renderPassData;
		auto &framebufferData = renderPassData.framebufferData;
		auto &cmdData = deviceData.cmdData;
		auto &swapchainExtent = swapchainData.extent;

		const auto &rpCallback = [&](const VkCommandBuffer &_cmdBuffer)
		{ setupRenderPassCommands(swapchainExtent, _cmdBuffer); };

		const auto &recordCallback = [&](const VkCommandBuffer &_cmdBuffer)
		{
			vk::Command::recordRenderPassCommands<ScreenData::s_fbAttCount>(
				_cmdBuffer,
				swapchainExtent,
				framebufferData,
				rpCallback
			);
		};

		auto &cmdBuffers = cmdData.drawCmdBuffers;
		for(auto i = 0u; i < cmdBuffers.size(); i++)
		{
			framebufferData.framebuffer = swapchainData.framebuffers[i];

			vk::Command::recordCmdBuffer(cmdBuffers[i], recordCallback);
		}
	}

	void Base::setupRenderPassCommands(
		const VkExtent2D			&_swapchainExtent,
		const VkCommandBuffer	&_cmdBuffer
	) noexcept
	{
		auto &pipelineData = m_screenData.pipelineData;

		vk::Command::setViewport(_cmdBuffer,	_swapchainExtent);
		vk::Command::setScissor(_cmdBuffer,		_swapchainExtent);

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