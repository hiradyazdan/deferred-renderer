#include "Renderer/Base.h"

namespace renderer
{
	void Base::init() noexcept
	{
		initVk();
		initCamera();
		initCommands();
		initSyncObjects();
		initGraphicsQueueSubmitInfo();

		setupRenderPass();
		setupFramebuffers();
	}

	void Base::initVk() noexcept
	{
		int width, height;
		auto &deviceData = m_device->getData();

		glfwSetWindowUserPointer(m_window, this);
		glfwGetFramebufferSize(m_window, &width, &height);

		m_device->createSurface(
			getSurfaceExtensions(),
			[&]()
			{
				return glfwCreateWindowSurface(
					deviceData.vkInstance, m_window,
					nullptr, &deviceData.surface
				);
			}, width, height
		);
		m_device->createDevice();
		m_device->createSwapchainData(deviceData.swapchainData);

		glfwSetFramebufferSizeCallback(m_window, framebufferResize);
	}

	void Base::initCamera() noexcept
	{
		const auto &scExtent = m_device->getData().swapchainData.extent;

		m_screenData.camera.setPerspective(
			60.0f,
			(float) scExtent.width / (float) scExtent.height,
			0.1f, 256.0f
		);
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

		vk::Command::setSubmitInfo(
			&semaphores.presentComplete,
			&semaphores.renderComplete,
			m_screenData.m_submitPipelineStages,
			submitInfo
		);
	}

	void Base::setupRenderPass() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &renderPassData = m_screenData.m_renderPassData;
		auto &framebufferData = renderPassData.framebufferData;
		auto &attachmentsData = framebufferData.attachments;

		auto &attCount		= ScreenData::s_fbAttCount;
		auto &spCount			= ScreenData::s_subpassCount;
		auto &spDepCount	= ScreenData::s_spDepCount;

		auto tempRPData = vk::RenderPass::Data<attCount, spCount, spDepCount>::Temp();
		auto &dependencies = tempRPData.deps;
		auto &subpasses = tempRPData.subpasses;

		attachmentsData.extent = swapchainData.extent;

		attachmentsData.formats = {
			swapchainData.format,
			deviceData.depthFormat
		};

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

	void Base::setupFramebuffers() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &framebufferData = m_screenData.m_renderPassData.framebufferData;
		auto &attData = framebufferData.attachments;
		auto &framebuffers = swapchainData.framebuffers;
		const auto &framebuffersSize = swapchainData.size;
		const auto FRAMEBUFFER = 0;
		const auto DEPTH = attData.depthAttIndex;

		std::array<VkImageView, ScreenData::s_fbAttCount> attachments = {};

		attachments[DEPTH] = attData.imageViews[DEPTH];

		auto fbInfo = vk::Framebuffer::setFramebufferInfo<ScreenData::s_fbAttCount>(
			framebufferData.renderPass,
			swapchainData.extent,
			attachments
		);

		framebuffers.resize(framebuffersSize);
		for(auto i = 0u; i < framebuffersSize; i++)
		{
			attachments[FRAMEBUFFER] = swapchainData.buffers[i].imageView;

			vk::Framebuffer::create(
				deviceData.logicalDevice,
				fbInfo,
				framebuffers[i]
			);
		}
	}

	void Base::setupCommands(
		const VkPipeline				&_pipeline,
		const VkPipelineLayout	&_pipelineLayout,
		const VkDescriptorSet		*_descSets,
		uint32_t								_descSetCount
	) noexcept
	{
		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &swapchainExtent = swapchainData.extent;
		auto &renderPassData = m_screenData.m_renderPassData;
		auto &framebufferData = renderPassData.framebufferData;
		auto &cmdData = deviceData.cmdData;

		const auto &rpCallback = [&](const VkCommandBuffer &_cmdBuffer)
		{
			setupRenderPassCommands(
				_cmdBuffer,
				_pipeline, _pipelineLayout,
				_descSets, _descSetCount
			);
		};

		const auto &recordCallback = [&](const VkCommandBuffer &_cmdBuffer)
		{
			vk::Command::recordRenderPassCommands(
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
		const VkCommandBuffer		&_cmdBuffer,
		const VkPipeline				&_pipeline,
		const VkPipelineLayout	&_pipelineLayout,
		const VkDescriptorSet		*_descSets,
		uint32_t								_descSetCount
	) noexcept
	{
		auto &deviceData = m_device->getData();
		auto &swapchainExtent = deviceData.swapchainData.extent;

		vk::Command::setViewport(_cmdBuffer,	swapchainExtent);
		vk::Command::setScissor(_cmdBuffer,		swapchainExtent);

		vk::Command::bindPipeline(
			_cmdBuffer,
			_pipeline
		);
		vk::Command::bindDescSets(
			_cmdBuffer,
			_descSets,
			_descSetCount,
			nullptr, 0,
			_pipelineLayout
		);
		vk::Command::draw(
			_cmdBuffer,
			3,
			1
		);
	}

	void Base::loadAssets() noexcept
	{
//		m_screenData.model.load(constants::models::sponza);
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

		vk::Command::submitQueue(graphicsQueue, submitInfo, "Full Scene Composition");
	}

	void Base::resizeWindow() noexcept
	{
		auto &isInited = m_screenData.isInited;

		if(!isInited) { return; }

		isInited								= false;
//		m_screenData.isResized	= true;

		auto &deviceData = m_device->getData();
		auto &logicalDevice	= deviceData.logicalDevice;
		auto &cmdData = deviceData.cmdData;
		auto &swapchainData = deviceData.swapchainData;
		auto &swapchainExtent = swapchainData.extent;
		auto &cmdBuffers = cmdData.drawCmdBuffers;
		auto &fbData = m_screenData.m_renderPassData.framebufferData;

		int width = 0, height = 0;
		while(width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_window, &width, &height);
			glfwWaitEvents();
		}

		m_device->waitIdle();

		m_device->createSwapchainData(swapchainData);

		fbData.attachments.extent = swapchainExtent;
		vk::Framebuffer::recreate<ScreenData::s_fbAttCount>(
			logicalDevice,
			deviceData.physicalDevice,
			deviceData.memProps,
			swapchainData.buffers,
			swapchainData.framebuffers,
			fbData
		);

		vk::Command::destroyCmdBuffers(
			logicalDevice,
			cmdData.cmdPool,
			cmdBuffers.data(),
			cmdBuffers.size()
		);

		initCommands();
		setupCommands();

		m_device->waitIdle();

		m_screenData.camera.updateAspectRatio(
			(float) swapchainExtent.width,
			(float) swapchainExtent.height
		);

		isInited = true;
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
			semaphores.presentComplete, &swapchainData.currentBuffer,
			[&]() { resizeWindow(); }
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
			semaphores.renderComplete, &swapchainData.currentBuffer,
			[&]() { resizeWindow(); },
			m_screenData.isResized
		);
	}

	void Base::framebufferResize(
		GLFWwindow *_window,
		int _width,
		int _height
	) noexcept
	{
		auto base = reinterpret_cast<Base*>(
			glfwGetWindowUserPointer(_window)
		);

		base->m_screenData.isResized = true;

//				glfwGetFramebufferSize(_window, &_width, &_height);
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