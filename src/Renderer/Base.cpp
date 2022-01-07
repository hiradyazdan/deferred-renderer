#include "Renderer/Base.h"
#include "Window.h"

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
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			submitInfo
		);
	}

	void Base::setupRenderPass() noexcept
	{
		using Att				= vk::Attachment;
		using AttData 	= Att::Data<Att::s_attCount>;
		using AttType 	= AttData::Type;

		auto &deviceData = m_device->getData();
		auto &swapchainData = deviceData.swapchainData;
		auto &framebufferData = getFbData();
		auto &attachmentsData = framebufferData.attachments;

		auto &attCount		= vk::Attachment::s_attCount;
		auto &spCount			= vk::RenderPass::s_subpassCount;
		auto &spDepCount	= vk::RenderPass::s_spDepCount;

		auto tempRPData = ScreenData::RenderPassData::Temp::create();
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
		auto &framebufferData = getFbData();
		auto &attData = framebufferData.attachments;
		auto &framebuffers = swapchainData.framebuffers;
		const auto &framebuffersSize = swapchainData.size;
		const auto &attCount = vk::Attachment::s_attCount;

		using AttType = vk::Attachment::Data<attCount>::Type;

		vk::Array<VkImageView, attCount> attachments = {};

		attachments[AttType::DEPTH] = attData.imageViews[attData.depthAttIndex];

		auto fbInfo = vk::Framebuffer::setFramebufferInfo<attCount>(
			framebufferData.renderPass,
			swapchainData.extent,
			attachments
		);

		framebuffers.resize(framebuffersSize);
		for(auto i = 0u; i < framebuffersSize; i++)
		{
			attachments[AttType::FRAMEBUFFER] = swapchainData.buffers[i].imageView;

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
		auto &framebufferData = getFbData();
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
		// TODO: Draw UI
	}

	void Base::loadAssets() noexcept
	{
		loadAsset(
			constants::models::sponza,
			m_screenData.modelData[vk::Model::Name::SPONZA].meshes
		);
	}

	void Base::loadAsset(
		const std::string									&_fileName,
		std::vector<vk::Mesh>							&_meshes,
		float 														_scale
	) noexcept
	{
		using LoadingFlags = AssetHelper::FileLoadingFlags;

		auto loadingFlags = LoadingFlags::PRE_TRANSFORM_VTX				|
												LoadingFlags::PRE_MULTIPLY_VTX_COLORS	|
												LoadingFlags::FLIP_Y;

		auto assetHelper = AssetHelper::create();

		assetHelper.load(
			constants::MODELS_PATH + _fileName,
			_meshes,
			loadingFlags, _scale
		);

		vk::Model::load(m_device, _meshes);
	}

	void Base::submitSceneQueue() noexcept
	{
		auto &deviceData = m_device->getData();
		auto &cmdData = deviceData.cmdData;
		auto &submitInfo = cmdData.submitInfo;
		auto &graphicsQueue = deviceData.graphicsQueue;
		auto &currentBuffer = deviceData.swapchainData.currentBuffer;

		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &cmdData.drawCmdBuffers[currentBuffer];

		vk::Command::submitQueue(graphicsQueue, submitInfo, "Full Scene Composition");
	}

	void Base::draw() noexcept
	{
		beginFrame();

		submitSceneQueue();

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
		auto &swapchainExtent = swapchainData.extent;
		auto &cmdBuffers = cmdData.drawCmdBuffers;
		auto &fbData = getFbData();

		int width = 0, height = 0;
		while(width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_window, &width, &height);
			glfwWaitEvents();
		}

		m_device->waitIdle();

		m_device->createSwapchainData(swapchainData);

		fbData.attachments.extent = swapchainExtent;
		vk::Framebuffer::recreate<vk::Attachment::s_attCount>(
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