#pragma once

#include "utils.h"
#include "Swapchain.h"
#include "Sync.h"
#include "Command.h"

namespace vk
{
	class Device
	{
		friend class Attachment;
		friend class Framebuffer;

		using CharPtrList = std::vector<const char*>;
		struct Data
		{
			VkPhysicalDeviceProperties				props;
			VkPhysicalDeviceMemoryProperties	memProps;
			VkPhysicalDeviceFeatures					features;

			QueueFamilyIndices queueFamilyIndices;

			CharPtrList				surfaceExtensions;
			CharPtrList 			deviceExtensions	= {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
			CharPtrList   		validationLayers	= {
				"VK_LAYER_KHRONOS_validation"
			};

			VkDebugUtilsMessengerEXT	debugMessenger				= VK_NULL_HANDLE;

			VkDevice									logicalDevice					= VK_NULL_HANDLE;
			VkPhysicalDevice					physicalDevice				= VK_NULL_HANDLE;

			VkInstance								vkInstance						= VK_NULL_HANDLE;
			VkSurfaceKHR							surface								= VK_NULL_HANDLE;

			VkQueue										graphicsQueue					= VK_NULL_HANDLE;
			VkQueue										presentQueue					= VK_NULL_HANDLE;

			VkFormat									depthFormat						= {};

			Swapchain	::Data					swapchainData;
			Sync			::Data					syncData;
			Command		::Data					cmdData;
		} m_data;

		public:
			static void freeMemory(
				const VkDevice				&_logicalDevice,
				const VkDeviceMemory	&_memory
			) noexcept;
			static void getFormatProps(
				const VkPhysicalDevice			&_physicalDevice,
				const VkFormat							&_format,
				VkFormatProperties					&_props
			) noexcept;

		public:
			void createVkInstance() noexcept;
			void createSurface(
				const CharPtrList &_surfaceExtensions,
				const std::function<VkResult()> &_surfaceCreateCb,
				uint32_t _width, uint32_t _height
			) noexcept;
			void createDevice() noexcept;
			void createSwapchainData(Swapchain::Data &_swapchainData) const noexcept;
			void waitIdle() const noexcept;

		public:
			Data &getData() noexcept { return m_data; }
			uint32_t getMemoryType(
				uint32_t										_typeBits,
				const VkMemoryPropertyFlags	&_props,
				VkBool32										*_isFound	= nullptr
			) const noexcept;

		/**
		 * Create Device Helpers
		 */
		private:
			void pickPhysicalDevice() noexcept;
			void createLogicalDevice() noexcept;
			void setDepthFormat() noexcept;

		private:
			static void retrievePhysicalDevices(
				const VkInstance											&_vkInstance,
				std::vector<VkPhysicalDevice>					&_physicalDevices,
				uint32_t															_deviceCount = 0
			) noexcept;
			static void retrieveQueueFamilies(
				const VkPhysicalDevice								&_physicalDevice,
				std::vector<VkQueueFamilyProperties>	&_queueFamilies,
				uint32_t              								_queueFamilyCount = 0
			) noexcept;
			static void retrieveDeviceExtensions(
				const VkPhysicalDevice								&_physicalDevice,
				std::vector<VkExtensionProperties>		&_availableExtensions,
				uint32_t              								_extensionCount = 0
			) noexcept;
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
			) noexcept;
			bool isDeviceExtensionsSupported(
				const VkPhysicalDevice	&_physicalDevice
			) const noexcept;
			static bool isLayersSupported(const CharPtrList &_layers) noexcept;
	};
}