#include "vk/Swapchain.h"

namespace vk
{
	Swapchain::Data::SwapChainSupportDetails Swapchain::getSwapChainSupportDetails(
		const VkPhysicalDevice &_physicalDevice,
		const VkSurfaceKHR &_surface
	) noexcept
	{
		Data::SwapChainSupportDetails details;
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

	void Swapchain::retrieveSwapchainImages(
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
			_images.data()
		);
		ASSERT_VK(result, "Failed to retrieve Swapchain images!");

		if(_images.empty() && _imageCount > 0)
		{
			_images.resize(_imageCount);
			_imageViews.resize(_imageCount);

			retrieveSwapchainImages(
				_logicalDevice,
				_swapchain,
				_imageViews,
				_images,
				_imageCount
			);
		}
	}

	VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(
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

	VkPresentModeKHR Swapchain::chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR> &_availablePresentModes
	) noexcept
	{
		for(const auto &availablePresentMode : _availablePresentModes)
		{
			if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Swapchain::chooseSwapExtent(
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

	void Swapchain::createSwapchain(
		const VkDevice						&_logicalDevice,
		const VkPhysicalDevice		&_physicalDevice,
		const VkSurfaceKHR 				&_surface,
		const VkExtent2D					&_windowExtent,
		const QueueFamilyIndices 	&_queueFamilyIndices,
		Data											&_swapchainData,
		const VkImageUsageFlags		&_imageUsage
	) noexcept
	{
		const auto &[
			capabilities,
			formats,
			presentModes
		] = getSwapChainSupportDetails(_physicalDevice, _surface);

		const auto &surfaceFormat    = chooseSwapSurfaceFormat (formats);
		const auto &presentMode      = chooseSwapPresentMode   (presentModes);
		const auto &extent           = chooseSwapExtent        (_windowExtent, capabilities);

		auto imageCount = capabilities.minImageCount + 1;

		if(capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchainInfo = {};

		swapchainInfo.sType                   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface                 = _surface;
		swapchainInfo.minImageCount           = _swapchainData.size    = imageCount;
		swapchainInfo.imageFormat             = _swapchainData.format  = surfaceFormat.format;
		swapchainInfo.imageColorSpace         = surfaceFormat.colorSpace;
		swapchainInfo.imageExtent             = _swapchainData.extent  = extent;
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
		swapchainInfo.oldSwapchain            = VK_NULL_HANDLE;

		auto result = vkCreateSwapchainKHR(
			_logicalDevice,
			&swapchainInfo,
			nullptr,
			&_swapchainData.swapchain
		);
		ASSERT_VK(result, "Failed to create Swapchain!");
	}

	void Swapchain::acquireNextImage(
		const VkDevice				&_logicalDevice,
		const VkSwapchainKHR	&_swapchain,
		const VkSemaphore			&_presentCompleteSemaphore,
		uint32_t							*_index
	) noexcept
	{
		auto acquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(
			vkGetDeviceProcAddr(_logicalDevice, "vkAcquireNextImageKHR")
		);

		auto result = acquireNextImageKHR(
			_logicalDevice, _swapchain,
			UINT64_MAX, _presentCompleteSemaphore,
			nullptr, _index
		);
		ASSERT_VK(result, "Failed to acquire next image!");
	}

	void Swapchain::queuePresentImage(
		const VkDevice				&_logicalDevice,
		const VkSwapchainKHR	&_swapchain,
		const VkQueue					&_queue,
		const VkSemaphore 		&_waitSemaphore,
		uint32_t							_index
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

		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (_waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores			= &_waitSemaphore;
			presentInfo.waitSemaphoreCount	= 1;
		}

		auto result = queuePresentKHR(_queue, &presentInfo);
		ASSERT_VK(result, "Failed to queue image for presentation!");
	}
}
