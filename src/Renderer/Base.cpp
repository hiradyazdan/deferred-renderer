#include "Renderer/Base.h"
#include "Window.h"

namespace renderer
{
	Base::~Base()
	{
		const auto &logicalDevice = m_device->getData().logicalDevice;
		const auto &fbData = getFbData(m_screenData);

		vk::RenderPass::destroy(logicalDevice, fbData.renderPass);
		vk::Attachment::destroy(logicalDevice, fbData.attachments);

		for(const auto &textureData : m_screenData.texturesData)
		{
			vk::Texture::destroy(logicalDevice, textureData);
		}
	}

	void Base::init() noexcept
	{
		initVk();
		initCamera();
		initCommands();
		initSyncPrimitives();

		setupRenderPass();
		setupFramebuffers();
	}

	void Base::initVk() noexcept
	{
		int width, height;
		auto &deviceData = m_device->getData();
		auto &syncData = deviceData.syncData;
		auto &logicalDevice = deviceData.logicalDevice;
		auto &semaphores = syncData.semaphores;

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

		for(auto s = 0; s < vk::toInt(vk::Sync::SemaphoreType::_count_); ++s)
		{
			vk::Sync::createSemaphore(logicalDevice, semaphores[s]);
		}

		glfwSetFramebufferSizeCallback(m_window, framebufferResize);
	}

	// @todo: move this to child renderer
	void Base::initCamera() noexcept
	{
		const auto &scExtent = m_device->getData().swapchainData.extent;
		auto &camera = m_screenData.camera;

		camera.flipY();
		camera.setType(Camera::Data::Type::FIRST_PERSON);
		camera.setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
		camera.setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
		camera.setPerspective(
			60.0f,
			(float) scExtent.width / (float) scExtent.height,
			0.1f, 256.0f
		);
	}

	void Base::initCommands() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &logicalDevice = deviceData.logicalDevice;
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

	void Base::initSyncPrimitives() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &syncData = deviceData.syncData;
		auto &logicalDevice = deviceData.logicalDevice;
		auto &fences = syncData.waitFences;

		fences.resize(deviceData.cmdData.drawCmdBuffers.size());
		for(auto &fence : fences)
		{
			vk::Sync::createFence(logicalDevice, fence, VK_FENCE_CREATE_SIGNALED_BIT);
		}
	}

	void Base::setupRenderPass() noexcept
	{
		using Att = vk::Attachment;
		using AttType = Att::Type;

		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &framebufferData = getFbData(m_screenData);
		auto &attachmentsData = framebufferData.attachments;
		auto &formats = attachmentsData.formats;

		auto &attCount = vk::Attachment::s_attCount;
		auto &spCount = vk::RenderPass::s_subpassCount;
		auto &spDepCount = vk::RenderPass::s_spDepCount;

		auto tempRPData = ScreenData::RenderPassData::create();
		auto &attSpMaps = tempRPData.attSpMaps;
		auto &dependencies = tempRPData.deps;
		auto &subpasses = tempRPData.subpasses;

		attachmentsData.extent = swapchainData.extent;

		formats[AttType::FRAMEBUFFER] = swapchainData.format;
		formats[AttType::DEPTH] = deviceData.depthFormat;

		attSpMaps[AttType::FRAMEBUFFER] = { AttType::FRAMEBUFFER, { 0 } };
		attSpMaps[AttType::DEPTH] = { AttType::DEPTH, { 0 } };

		// TODO
		dependencies[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass			= 0;
		dependencies[0].srcStageMask		= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].dstStageMask		= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dstAccessMask		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		dependencies[0].dependencyFlags	= 0;

		dependencies[1].srcSubpass			= VK_SUBPASS_EXTERNAL;
		dependencies[1].dstSubpass			= 0;
		dependencies[1].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].srcAccessMask		= 0;
		dependencies[1].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		dependencies[1].dependencyFlags	= 0;

		vk::Framebuffer::createAttachments<attCount>(
			deviceData.logicalDevice,
			deviceData.memProps,
			attSpMaps,
			attachmentsData, true
		);

		vk::RenderPass::createSubpasses<attCount, spCount>(
			attSpMaps,
			subpasses
		);

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
		using AttType = vk::Attachment::Type;

		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &framebufferData = getFbData(m_screenData);
		auto &attData = framebufferData.attachments;
		auto &framebuffers = swapchainData.framebuffers;
		const auto &framebuffersSize = swapchainData.size;
		const auto &attCount = vk::Attachment::s_attCount;

		vk::Array<VkImageView, attCount> attachments = {};

		attachments[AttType::DEPTH] = attData.imageViews[attData.depthAttIndex];

		auto fbInfo = vk::Framebuffer::setFramebufferInfo(
			framebufferData.renderPass,
			swapchainData.extent,
			attachments
		);

		framebuffers.resize(framebuffersSize);
		for (auto i = 0u; i < framebuffersSize; ++i)
		{
			attachments[AttType::FRAMEBUFFER] = swapchainData.imageViews[i];

			vk::Framebuffer::create(
				deviceData.logicalDevice,
				fbInfo,
				framebuffers[i]
			);
		}
	}

	void Base::setupCommands(
		const VkPipeline &_pipeline,
		const VkPipelineLayout &_pipelineLayout,
		const VkDescriptorSet *_descSets,
		uint32_t _descSetCount
	) noexcept
	{
		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &swapchainExtent = swapchainData.extent;
		auto &framebufferData = getFbData(m_screenData);
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
		for (auto i = 0u; i < cmdBuffers.size(); ++i)
		{
			framebufferData.framebuffer = swapchainData.framebuffers[i];

			vk::Command::record(cmdBuffers[i], recordCallback);
		}
	}

	void Base::setupRenderPassCommands(
		const VkCommandBuffer &_cmdBuffer,
		const VkPipeline &_pipeline,
		const VkPipelineLayout &_pipelineLayout,
		const VkDescriptorSet *_descSets,
		uint32_t _descSetCount
	) noexcept
	{
		auto &deviceData = m_device->getData();
		auto &swapchainExtent = deviceData.swapchainData.extent;

		vk::Command::setViewport(_cmdBuffer, swapchainExtent);
		vk::Command::setScissor(_cmdBuffer, swapchainExtent);

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
		vk::Command::draw(_cmdBuffer, 3);
		// TODO: Draw UI
	}

	void Base::submitSceneToQueue() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &cmdData = deviceData.cmdData;
		auto &semaphores = deviceData.syncData.semaphores;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &activeFbIndex = deviceData.swapchainData.activeFbIndex;

		VkSubmitInfo submitInfo = {};
		vk::Command::setSubmitInfo(
			&semaphores[vk::Sync::SemaphoreType::PRESENT_COMPLETE],
			&semaphores[vk::Sync::SemaphoreType::RENDER_COMPLETE],
			&cmdData.drawCmdBuffers[activeFbIndex],
			submitInfo
		);
		vk::Command::submitToQueue(
			graphicsQueue, submitInfo,
			"Full Scene"
		);
	}

	void Base::draw() noexcept
	{
		beginFrame();

		submitSceneToQueue();

		endFrame();
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
		auto &syncData = deviceData.syncData;
		auto &swapchainExtent = swapchainData.extent;
		auto &cmdBuffers = cmdData.drawCmdBuffers;
		auto &fbData = getFbData(m_screenData);

		int width = 0, height = 0;
		while(width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_window, &width, &height);
			glfwWaitEvents();
		}
		fbData.attachments.extent = swapchainExtent = { (uint32_t) width, (uint32_t) height };

		m_device->waitIdle();

		m_device->createSwapchainData(swapchainData);

		vk::Framebuffer::recreate(
			logicalDevice,
			deviceData.memProps,
			swapchainData.imageViews,
			fbData.renderPass,
			fbData.attachments,
			swapchainData.framebuffers
		);

		vk::Command::destroyCmdBuffers(
			logicalDevice,
			cmdData.cmdPool,
			cmdBuffers.data(),
			cmdBuffers.size()
		);

		initCommands();
		setupBaseCommands();

		for(auto &fence : syncData.waitFences)
		{
			vk::Sync::destroyFence(logicalDevice, fence);
		}
		initSyncPrimitives();

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
		auto &logicalDevice = deviceData.logicalDevice;
		auto &swapchain = swapchainData.swapchain;
		auto &semaphores = deviceData.syncData.semaphores;

		vk::Swapchain::acquireNextImage(
			logicalDevice, swapchain,
			semaphores[vk::Sync::SemaphoreType::PRESENT_COMPLETE],
			&swapchainData.activeFbIndex,
			[&]() { resizeWindow(); }
		);
	}

	void Base::endFrame() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &logicalDevice = deviceData.logicalDevice;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &swapchain = swapchainData.swapchain;
		auto &semaphores = deviceData.syncData.semaphores;

		vk::Swapchain::queuePresentImage(
			logicalDevice, swapchain, graphicsQueue,
			semaphores[vk::Sync::SemaphoreType::RENDER_COMPLETE],
			swapchainData.activeFbIndex,
			[&]() { resizeWindow(); }
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