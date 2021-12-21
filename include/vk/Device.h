#pragma once

#include "utils.h"
#include "Swapchain.h"

namespace vk
{
	class Device
	{
		using CharPtrList = std::vector<const char*>;
		struct Data
		{
			QueueFamilyIndices queueFamilyIndices;

			CharPtrList				surfaceExtensions;
			CharPtrList 			deviceExtensions	= {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
			CharPtrList   		validationLayers	= {
				"VK_LAYER_KHRONOS_validation"
			};

			VkDebugUtilsMessengerEXT	debugMessenger	= VK_NULL_HANDLE;

			VkDevice									logicalDevice		= VK_NULL_HANDLE;
			VkPhysicalDevice					physicalDevice	= VK_NULL_HANDLE;

			VkInstance								vkInstance			= VK_NULL_HANDLE;
			VkSurfaceKHR							surface					= VK_NULL_HANDLE;

			VkQueue										graphicsQueue		= VK_NULL_HANDLE;
			VkQueue										presentQueue		= VK_NULL_HANDLE;

			VkFormat									depthFormat			= {};
			VkExtent2D								windowExtent		= {};

			Swapchain::Data						swapchainData;
		} m_data;

		public:
			void createVkInstance() noexcept;
			void createSurface(
				const std::function<VkResult()> &_surfaceCreateCb,
				const CharPtrList &_surfaceExtensions,
				uint32_t _width, uint32_t _height
			) noexcept;
			void createDevice() noexcept;
			void createSwapchainData(Swapchain::Data &_swapchainData) const noexcept;

		public:
			Data &getData() noexcept { return m_data; }

		/**
		 * Create Device Helpers
		 */
		private:
			void pickPhysicalDevice() noexcept;
			void createLogicalDevice() noexcept;
			void setDepthFormat() noexcept;

		private:
			void retrievePhysicalDevices(
				std::vector<VkPhysicalDevice>					&_physicalDevices,
				uint32_t															_deviceCount = 0
			)	const noexcept;
			void retrieveQueueFamilies(
				const VkPhysicalDevice								&_physicalDevice,
				std::vector<VkQueueFamilyProperties>	&_queueFamilies,
				uint32_t              								_queueFamilyCount = 0
			) const noexcept;
			void retrieveDeviceExtensions(
				const VkPhysicalDevice								&_physicalDevice,
				std::vector<VkExtensionProperties>		&_availableExtensions,
				uint32_t              								_extensionCount = 0
			) const noexcept;
			static void retrieveAvailableLayers(
				std::vector<VkLayerProperties>				&_availableLayers,
				uint32_t              								_layerCount = 0
			) noexcept;

		private:
			QueueFamilyIndices getQueueFamilies(
				const VkPhysicalDevice	&_physicalDevice
			) const noexcept;
			bool isDeviceSuitable(
				const VkPhysicalDevice	&_physicalDevice
			) const noexcept;
			bool isDeviceExtensionsSupported(
				const VkPhysicalDevice	&_physicalDevice
			) const noexcept;
			static bool isLayersSupported(const CharPtrList &_layers) noexcept;

		/**
		 * Sync Helpers
		 */
		private:
			void createFence()							noexcept;
			void createSemaphore()					noexcept;
	};
}