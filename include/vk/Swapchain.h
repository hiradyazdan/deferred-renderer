#pragma once

#include "utils.h"

namespace vk
{
	class Swapchain
	{
		friend class Device;
		friend class Framebuffer;
		struct Data
		{
			struct SupportDetails
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
			uint32_t													activeFbIndex	= 0;
			VkFormat                					format;
			VkExtent2D              					extent;
		};

		public:
			static Data::SupportDetails getSupportDetails(
				const VkPhysicalDevice	&_physicalDevice,
				const VkSurfaceKHR      &_surface
			) noexcept;

			static void create(
				const VkDevice						&_logicalDevice,
				const VkPhysicalDevice		&_physicalDevice,
				const VkSurfaceKHR 				&_surface,
				const QueueFamilyIndices	&_queueFamilyIndices,
				Data											&_swapchainData,
				const VkImageUsageFlags		&_imageUsage = image::UsageFlag::COLOR_ATTACHMENT
			) noexcept;

			static void acquireNextImage(
				const VkDevice							&_logicalDevice,
				const VkSwapchainKHR				&_swapchain,
				const VkSemaphore						&_presentCompleteSemaphore,
				uint32_t										*_pIndex,
				const std::function<void()> &_winResizeCallback
			) noexcept;

			static void queuePresentImage(
				const VkDevice							&_logicalDevice,
				const VkSwapchainKHR				&_swapchain,
				const VkQueue								&_queue,
				const VkSemaphore						&_renderCompleteSemaphore,
				uint32_t										_index,
				const std::function<void()>	&_winResizeCallback,
				bool												&_isResized
			) noexcept;

		private:
			static VkSurfaceFormatKHR	chooseSurfaceFormat(
				const std::vector<VkSurfaceFormatKHR>	&_availableFormats
			)	noexcept;
			static VkPresentModeKHR		choosePresentMode(
				const std::vector<VkPresentModeKHR>		&_availablePresentModes,
				bool																	_vsync = false
			) noexcept;
			static VkExtent2D					chooseExtent(
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
			static void retrieveImages(
				const VkDevice									&_logicalDevice,
				const VkSwapchainKHR 						&_swapchain,
				std::vector<VkImageView>				&_imageViews,
				std::vector<VkImage>						&_images,
				uint32_t              					_imageCount = 0
			) noexcept;
	};
}