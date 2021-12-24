#pragma once

#include "utils.h"

namespace vk
{
	class Swapchain
	{
		friend class Device;
		struct Data
		{
			struct SwapChainSupportDetails
			{
				VkSurfaceCapabilitiesKHR				capabilities;
				std::vector<VkSurfaceFormatKHR>	formats;
				std::vector<VkPresentModeKHR>		presentModes;
			};

			std::vector<VkImage>             	images;
			std::vector<VkImageView>					imageViews;
			std::vector<VkFramebuffer>				framebuffers;

			VkSwapchainKHR             				swapchain			= VK_NULL_HANDLE;
			uint32_t              						size      		= 2;
			uint32_t													currentBuffer	= 0;
			VkFormat                					format;
			VkExtent2D              					extent;
		};

		public:
			static Data::SwapChainSupportDetails getSwapChainSupportDetails(
				const VkPhysicalDevice	&_physicalDevice,
				const VkSurfaceKHR      &_surface
			) noexcept;

			static void createSwapchain(
				const VkDevice						&_logicalDevice,
				const VkPhysicalDevice		&_physicalDevice,
				const VkSurfaceKHR 				&_surface,
				const VkExtent2D					&_windowExtent,
				const QueueFamilyIndices	&_queueFamilyIndices,
				Data											&_swapchainData,
				const VkImageUsageFlags		&_imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			) noexcept;

			static void acquireNextImage(
				const VkDevice				&_logicalDevice,
				const VkSwapchainKHR	&_swapchain,
				const VkSemaphore			&_presentCompleteSemaphore,
				uint32_t							*_index
			) noexcept;

			static void queuePresentImage(
				const VkDevice				&_logicalDevice,
				const VkSwapchainKHR	&_swapchain,
				const VkQueue					&_queue,
				const VkSemaphore			&_waitSemaphore,
				uint32_t							_index
			) noexcept;

		private:
			static VkSurfaceFormatKHR	chooseSwapSurfaceFormat(
				const std::vector<VkSurfaceFormatKHR>	&_availableFormats
			)	noexcept;
			static VkPresentModeKHR		chooseSwapPresentMode(
				const std::vector<VkPresentModeKHR>		&_availablePresentModes
			) noexcept;
			static VkExtent2D					chooseSwapExtent(
				const VkExtent2D            					&_windowExtent,
				const VkSurfaceCapabilitiesKHR				&_capabilities
			) noexcept;

		private:
			static void retrieveSurfaceFormats(
				const VkPhysicalDevice  				&_physicalDevice,
				const VkSurfaceKHR         			&_surface,
				std::vector<VkSurfaceFormatKHR>	&_surfaceFormats,
				uint32_t              					_formatCount			= 0
			) noexcept;
			static void retrievePresentModes(
				const VkPhysicalDevice  				&_physicalDevice,
				const VkSurfaceKHR         			&_surface,
				std::vector<VkPresentModeKHR>		&_presentModes,
				uint32_t              					_presentModeCount = 0
			) noexcept;
			static void retrieveSwapchainImages(
				const VkDevice									&_logicalDevice,
				const VkSwapchainKHR 						&_swapchain,
				std::vector<VkImageView>				&_imageViews,
				std::vector<VkImage>						&_images,
				uint32_t              					_imageCount = 0
			) noexcept;
	};
}