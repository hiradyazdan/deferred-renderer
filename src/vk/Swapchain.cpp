#include "vk/Swapchain.h"
#include "vk/Image.h"

namespace vk
{
	Swapchain::Data::SupportDetails Swapchain::getSupportDetails(
		const VkPhysicalDevice &_physicalDevice,
		const VkSurfaceKHR &_surface
	) noexcept
	{
		Data::SupportDetails details;
		auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			_physicalDevice,
			_surface,
			&details.capabilities
		);
		ASSERT_VK(result, "Failed to retrieve capabilities!");

		retrieveSurfaceFormats(_physicalDevice, _surface, details.formats);
		retrievePresentModes  (_physicalDevice, _surface, details.presentModes);

		return details;
	}

	void Swapchain::retrieveSurfaceFormats(
		const VkPhysicalDevice					&_physicalDevice,
		const VkSurfaceKHR 							&_surface,
		std::vector<VkSurfaceFormatKHR>	&_surfaceFormats,
		uint32_t												_formatCount
	) noexcept
	{
		const auto &result = vkGetPhysicalDeviceSurfaceFormatsKHR(
			_physicalDevice,
			_surface,
			&_formatCount,
			_surfaceFormats.data()
		);
		ASSERT_VK(result, "Failed to retrieve surface formats!");

		if(_surfaceFormats.empty() && _formatCount > 0)
		{
			_surfaceFormats.resize(_formatCount);

			retrieveSurfaceFormats(
				_physicalDevice,
				_surface,
				_surfaceFormats,
				_formatCount
			);
		}
	}

	void Swapchain::retrievePresentModes(
		const VkPhysicalDevice				&_physicalDevice,
		const VkSurfaceKHR 						&_surface,
		std::vector<VkPresentModeKHR> &_presentModes,
		uint32_t											_presentModeCount
	) noexcept
	{
		const auto &result = vkGetPhysicalDeviceSurfacePresentModesKHR(
			_physicalDevice,
			_surface,
			&_presentModeCount,
			_presentModes.data()
		);
		ASSERT_VK(result, "Failed to retrieve present modes!");

		if(_presentModes.empty() && _presentModeCount > 0)
		{
			_presentModes.resize(_presentModeCount);

			retrievePresentModes(
				_physicalDevice,
				_surface,
				_presentModes,
				_presentModeCount
			);
		}
	}

	void Swapchain::retrieveImages(
		const VkDevice						&_logicalDevice,
		const VkSwapchainKHR 			&_swapchain,
		std::vector<VkImageView>	&_imageViews,
		std::vector<VkImage> 			&_images,
		uint32_t									_imageCount
	) noexcept
	{
		const auto &result = vkGetSwapchainImagesKHR(
			_logicalDevice,
			_swapchain,
			&_imageCount,
			_imageCount > 0 ? _images.data() : nullptr
		);
		ASSERT_VK(result, "Failed to retrieve Swapchain images!");

		if(_images.empty() && _imageCount > 0)
		{
			_images.resize(_imageCount);
			_imageViews.resize(_imageCount);

			retrieveImages(
				_logicalDevice,
				_swapchain,
				_imageViews,
				_images,
				_imageCount
			);
		}
	}

	VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR> &_availableFormats
	) noexcept
	{
		for(const auto &availableFormat : _availableFormats)
		{
			if(
				availableFormat.format      == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace  == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
			) return availableFormat;
		}

		return _availableFormats[0];
	}

	VkPresentModeKHR Swapchain::choosePresentMode(
		const std::vector<VkPresentModeKHR>		&_availablePresentModes,
		bool																	_vsync
	) noexcept
	{
		auto presentMode = VK_PRESENT_MODE_FIFO_KHR;

		if(_vsync) { return presentMode; }

		for(const auto &availablePresentMode : _availablePresentModes)
		{
			if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = availablePresentMode;
				break;
			}

			if(availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				presentMode = availablePresentMode;
			}
		}

		return presentMode;
	}

	VkExtent2D Swapchain::chooseExtent(
		const VkExtent2D &_windowExtent,
		const VkSurfaceCapabilitiesKHR &_capabilities
	) noexcept
	{
		if(_capabilities.currentExtent.width != UINT32_MAX)
		{
			return _capabilities.currentExtent;
		}
		else
		{
			auto &minImageExtent = _capabilities.minImageExtent;
			auto &maxImageExtent = _capabilities.maxImageExtent;

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(_windowExtent.width),
				static_cast<uint32_t>(_windowExtent.height)
			};

			actualExtent.width = std::clamp(
				actualExtent.width,
				minImageExtent.width,
				maxImageExtent.width
			);
			actualExtent.height = std::clamp(
				actualExtent.height,
				minImageExtent.height,
				maxImageExtent.height
			);

			return actualExtent;
		}
	}

	void Swapchain::create(
		const VkDevice						&_logicalDevice,
		const VkPhysicalDevice		&_physicalDevice,
		const VkSurfaceKHR 				&_surface,
		const QueueFamilyIndices 	&_queueFamilyIndices,
		Data											&_swapchainData,
		const VkImageUsageFlags		&_imageUsage
	) noexcept
	{
		auto oldSwapchain	= _swapchainData.swapchain;

		const auto &[
			capabilities,
			formats,
			presentModes
		] = getSupportDetails(_physicalDevice, _surface);

		const auto &surfaceFormat    = chooseSurfaceFormat(formats);
		const auto &presentMode      = choosePresentMode	(presentModes);
		const auto &extent           = chooseExtent				(_swapchainData.extent, capabilities);

		auto &scSize		= _swapchainData.size;

		auto imageCount = capabilities.minImageCount + 1;

		if(capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchainInfo = {};

		swapchainInfo.sType                   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface                 = _surface;
		swapchainInfo.minImageCount           = scSize    						= imageCount;
		swapchainInfo.imageFormat             = _swapchainData.format	= surfaceFormat.format;
		swapchainInfo.imageColorSpace         = surfaceFormat.colorSpace;
		swapchainInfo.imageExtent             = _swapchainData.extent	= extent;
		swapchainInfo.imageArrayLayers        = 1;
		swapchainInfo.imageUsage              = _imageUsage;

		const auto &indices = _queueFamilyIndices;
		uint32_t queueFamilyIndices[2] = {
			static_cast<uint32_t>(indices.graphicsFamily),
			static_cast<uint32_t>(indices.presentFamily)
		};

		if(indices.graphicsFamily != indices.presentFamily)
		{
			swapchainInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
			swapchainInfo.queueFamilyIndexCount = sizeof(queueFamilyIndices) / sizeof(queueFamilyIndices[0]);
			swapchainInfo.pQueueFamilyIndices   = queueFamilyIndices;
		}
		else
		{
			swapchainInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
			swapchainInfo.queueFamilyIndexCount = 0;
			swapchainInfo.pQueueFamilyIndices   = nullptr;
		}

		swapchainInfo.preTransform            = capabilities.currentTransform;
		swapchainInfo.compositeAlpha          = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainInfo.presentMode             = presentMode;
		swapchainInfo.clipped                 = VK_TRUE;
		swapchainInfo.oldSwapchain            = oldSwapchain;

		auto result = vkCreateSwapchainKHR(
			_logicalDevice,
			&swapchainInfo,
			nullptr,
			&_swapchainData.swapchain
		);
		ASSERT_VK(result, "Failed to create Swapchain!");

		destroy(_logicalDevice, _swapchainData.imageViews, oldSwapchain);
	}

	void Swapchain::destroy(
		const VkDevice							&_logicalDevice,
		std::vector<VkImageView>		&_imageViews,
		VkSwapchainKHR 							&_swapchain,
		const VkAllocationCallbacks	*_pAllocator
	) noexcept
	{
		if(_swapchain != VK_NULL_HANDLE)
		{
			for(auto &imageView : _imageViews)
			{
				Image::destroyImageView(_logicalDevice, imageView, _pAllocator);
			}

			vkDestroySwapchainKHR(_logicalDevice, _swapchain, _pAllocator);
		}
	}

	void Swapchain::acquireNextImage(
		const VkDevice							&_logicalDevice,
		const VkSwapchainKHR				&_swapchain,
		const VkSemaphore						&_presentCompleteSemaphore,
		uint32_t										*_pIndex,
		const std::function<void()>	&_winResizeCallback
	) noexcept
	{
		auto acquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(
			vkGetDeviceProcAddr(_logicalDevice, "vkAcquireNextImageKHR")
		);

		auto result = acquireNextImageKHR(
			_logicalDevice, _swapchain,
			UINT64_MAX, _presentCompleteSemaphore,
			nullptr, _pIndex
		);

		if(
			result == VK_ERROR_OUT_OF_DATE_KHR ||
			result == VK_SUBOPTIMAL_KHR
		)
		{
			if(result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				_winResizeCallback();
			}
			return;
		}
		else
		{
			ASSERT_VK(result, "Failed to acquire next image!");
		}
	}

	void Swapchain::queuePresentImage(
		const VkDevice							&_logicalDevice,
		const VkSwapchainKHR				&_swapchain,
		const VkQueue								&_queue,
		const VkSemaphore						&_renderCompleteSemaphore,
		uint32_t										_index,
		const std::function<void()>	&_winResizeCallback
	) noexcept
	{
		auto queuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(
			vkGetDeviceProcAddr(_logicalDevice, "vkQueuePresentKHR")
		);

		VkPresentInfoKHR presentInfo			= {};
		presentInfo.sType									= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext									= nullptr;
		presentInfo.swapchainCount				= 1;
		presentInfo.pSwapchains						= &_swapchain;
		presentInfo.pImageIndices					= &_index;

		if (_renderCompleteSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores			= &_renderCompleteSemaphore;
			presentInfo.waitSemaphoreCount	= 1;
		}

		auto result = queuePresentKHR(_queue, &presentInfo);

		if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			_winResizeCallback();
			if(result == VK_ERROR_OUT_OF_DATE_KHR/* || _isResized*/)
			{
//				_isResized = false;
//				_winResizeCallback();
				return;
			}
		}
		else
		{
			ASSERT_VK(result, "Failed to queue image for presentation!");
		}

		result = vkQueueWaitIdle(_queue);
		ASSERT_VK(result, "Failed to wait for the graphics queue!");
	}
}
