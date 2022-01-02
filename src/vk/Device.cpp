#include "vk/Device.h"
#include "vk/Image.h"

namespace vk
{
	void Device::createVkInstance() noexcept
	{
		VkApplicationInfo appInfo = {};
		
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
		appInfo.pEngineName = "";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;
		
		VkInstanceCreateInfo instanceInfo = {};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;

		const auto &validationLayers = m_data.validationLayers;
		auto &extensions = m_data.surfaceExtensions;

		if(!validationLayers.empty() && isLayersSupported(validationLayers))
		{
			instanceInfo.enabledLayerCount      = validationLayers.size();
			instanceInfo.ppEnabledLayerNames    = validationLayers.data();

			VkDebugUtilsMessengerCreateInfoEXT debugInfo;
			debug::setMessengerInfo(debugInfo);

			instanceInfo.pNext                  = (VkDebugUtilsMessengerCreateInfoEXT*) &debugInfo;

			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		
		instanceInfo.enabledExtensionCount = m_data.surfaceExtensions.size();
		instanceInfo.ppEnabledExtensionNames = m_data.surfaceExtensions.data();
		
		auto result = vkCreateInstance(
			&instanceInfo,
			nullptr,
			&m_data.vkInstance
		);
		ASSERT_VK(result, "Failed to create vulkan instance!");

		if (!validationLayers.empty())
		{
			VkDebugUtilsMessengerCreateInfoEXT debugInfo;
			debug::setMessengerInfo(debugInfo);

			result = debug::setMessenger(
				m_data.vkInstance,
				&debugInfo,
				nullptr,
				&m_data.debugMessenger
			);
			ASSERT_VK(result, "Failed to create debug messenger!");
		}
	}
	
	void Device::createSurface(
		const std::function<VkResult()> &_surfaceCreateCb,
		const CharPtrList &_surfaceExtensions,
		uint32_t _width, uint32_t _height
	) noexcept
	{
		m_data.windowExtent = { _width, _height };
		m_data.surfaceExtensions = _surfaceExtensions;

		createVkInstance();

		auto result = _surfaceCreateCb();
		ASSERT_VK(result, "Failed to create window surface!");
	}

	void Device::createDevice() noexcept
	{
		ASSERT(m_data.surface != VK_NULL_HANDLE, "Window surface is not created yet!");

		pickPhysicalDevice  ();
		setDepthFormat      ();
		createLogicalDevice ();
	}

	void Device::pickPhysicalDevice() noexcept
	{
		std::vector<VkPhysicalDevice> devices;
		retrievePhysicalDevices(m_data.vkInstance, devices);

		for(const auto &device : devices)
		{
			if(isDeviceSuitable(device))
			{
				m_data.physicalDevice = device;

				vkGetPhysicalDeviceProperties(
					m_data.physicalDevice, &m_data.props
				);
				vkGetPhysicalDeviceMemoryProperties(
					m_data.physicalDevice, &m_data.memProps
				);
				vkGetPhysicalDeviceFeatures(
					m_data.physicalDevice, &m_data.features
				);

				if(m_data.features.samplerAnisotropy)
				{ break; }
				else
				{ continue; }
			}
		}
	}

	void Device::createLogicalDevice() noexcept
	{
		ASSERT(m_data.physicalDevice != VK_NULL_HANDLE, "PhysicalDevice is not selected yet!");

		const auto &indices = m_data.queueFamilyIndices = getQueueFamilies(
			m_data.physicalDevice
		);

		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		std::set<uint32_t> uniqueQueueFamilies = {
			static_cast<uint32_t>(indices.graphicsFamily),
			static_cast<uint32_t>(indices.presentFamily)
		};

		float queuePriority = 1.0f;
		for(const auto &queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueInfo = {};

			queueInfo.sType             = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex  = queueFamily;
			queueInfo.queueCount        = 1;
			queueInfo.pQueuePriorities  = &queuePriority;

			queueInfos.push_back(queueInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures	= {};
		deviceFeatures.samplerAnisotropy				= VK_TRUE;

		VkDeviceCreateInfo deviceInfo				= {};

		deviceInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.queueCreateInfoCount     = static_cast<uint32_t>(queueInfos.size());
		deviceInfo.pQueueCreateInfos        = queueInfos.data();
		deviceInfo.pEnabledFeatures         = &deviceFeatures;
		deviceInfo.enabledExtensionCount    = static_cast<uint32_t>(m_data.deviceExtensions.size());
		deviceInfo.ppEnabledExtensionNames  = m_data.deviceExtensions.data();

		if(!m_data.validationLayers.empty())
    {
      deviceInfo.enabledLayerCount    = static_cast<uint32_t>(m_data.validationLayers.size());
      deviceInfo.ppEnabledLayerNames  = m_data.validationLayers.data();
    }

		const auto &result = vkCreateDevice(
			m_data.physicalDevice,
			&deviceInfo,
			nullptr,
			&m_data.logicalDevice
		);
		ASSERT_VK(result, "Failed to create logical device!");

		vkGetDeviceQueue(
			m_data.logicalDevice, indices.graphicsFamily,
			0, &m_data.graphicsQueue
		);
		vkGetDeviceQueue(
			m_data.logicalDevice, indices.presentFamily,
			0, &m_data.presentQueue
		);
	}

	void Device::setDepthFormat() noexcept
	{
		ASSERT(m_data.physicalDevice != VK_NULL_HANDLE, "PhysicalDevice is not selected yet!");

		const auto &formats = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		};
		const auto &features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

		VkFormatProperties props;
		for (const auto &format : formats)
		{
			vkGetPhysicalDeviceFormatProperties(
				m_data.physicalDevice,
				format,
				&props
			);

			if ((props.optimalTilingFeatures & features) == features)
			{
				m_data.depthFormat = format;
				break;
			}
		}
	}

	QueueFamilyIndices Device::getQueueFamilies(
		const VkPhysicalDevice &_physicalDevice
	) const noexcept
	{
		QueueFamilyIndices indices;
		std::vector<VkQueueFamilyProperties> queueFamilies;
		retrieveQueueFamilies(_physicalDevice, queueFamilies);

		int i = 0;
		for(const auto &family : queueFamilies)
		{
			if(
				family.queueCount > 0 &&
				family.queueFlags & VK_QUEUE_GRAPHICS_BIT
				) indices.graphicsFamily = i;

			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(
				_physicalDevice,
				i,
				m_data.surface,
				&presentSupport
			);

			if(family.queueCount > 0 && presentSupport) indices.presentFamily = i;
			if (indices.isComplete()) break;

			i++;
		}

		return indices;
	}

	bool Device::isDeviceSuitable(
		const VkPhysicalDevice &_physicalDevice
	) noexcept
	{
		const auto &indices = getQueueFamilies(_physicalDevice);
		const auto &isExtensionsSupported = isDeviceExtensionsSupported(_physicalDevice);

		bool isSwapChainAdequate = false;
		if (isExtensionsSupported)
		{
			auto swapChainSupport = Swapchain::getSwapChainSupportDetails(_physicalDevice, m_data.surface);
			isSwapChainAdequate   = !swapChainSupport.formats.empty() &&
															!swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() &&
					 isExtensionsSupported &&
					 isSwapChainAdequate;
	}

	bool Device::isDeviceExtensionsSupported(const VkPhysicalDevice &_physicalDevice) const noexcept
	{
		std::vector<VkExtensionProperties> availableExtensions;
		retrieveDeviceExtensions(_physicalDevice, availableExtensions);

		std::set<std::string_view> requiredExtensions(
			m_data.deviceExtensions.begin(),
			m_data.deviceExtensions.end()
		);

		for(const auto &extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool Device::isLayersSupported(const Device::CharPtrList &_layers) noexcept
	{
		std::vector<VkLayerProperties> availableLayers;
		retrieveAvailableLayers(availableLayers);

		for(const auto &layer : _layers)
		{
			bool layerFound = false;

			for(const auto &props : availableLayers)
			{
				if(strcmp(layer, props.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if(!layerFound)
			{
				CRITICAL_ERROR_LOG("This Validation layer isn't supported: %s", layer);

				return false;
			}
		}

		return true;
	}

	void Device::createSwapchainData(Swapchain::Data &_swapchainData) const noexcept
	{
		const auto &logicalDevice = m_data.logicalDevice;
		auto &swapchainImages = _swapchainData.images;
		auto &swapchainImageViews = _swapchainData.imageViews;

		Swapchain::createSwapchain(
			logicalDevice,
			m_data.physicalDevice,
			m_data.surface,
			m_data.windowExtent,
			m_data.queueFamilyIndices,
			_swapchainData
		);

		Swapchain::retrieveSwapchainImages(
			logicalDevice,
			_swapchainData.swapchain,
			swapchainImageViews,
			swapchainImages
		);

		const auto &swapchainSize = swapchainImages.size();
		for(auto i = 0u; i < swapchainSize; i++)
		{
			Image::createImageView(
				logicalDevice,
				swapchainImages[i],
				_swapchainData.format,
				swapchainImageViews[i]
			);
		}
	}

	void Device::retrieveAvailableLayers(
		std::vector<VkLayerProperties> &_availableLayers,
		uint32_t _layerCount
	) noexcept
	{
		const auto &result = vkEnumerateInstanceLayerProperties(
			&_layerCount,
			_availableLayers.data()
		);
		ASSERT_VK(result, "Failed to retrieve Available Layers!");

		if(_availableLayers.empty() && _layerCount > 0)
		{
			_availableLayers.resize(_layerCount);

			retrieveAvailableLayers(
				_availableLayers,
				_layerCount
			);
		}
	}

	void Device::retrievePhysicalDevices(
		const VkInstance							&_vkInstance,
		std::vector<VkPhysicalDevice>	&_physicalDevices,
		uint32_t											_deviceCount
	) noexcept
	{
		const auto &result = vkEnumeratePhysicalDevices(
			_vkInstance,
			&_deviceCount,
			_physicalDevices.data()
		);
		ASSERT_VK(result, "Failed to find GPUs with Vulkan support!");

		if(_physicalDevices.empty() && _deviceCount > 0)
		{
			_physicalDevices.resize(_deviceCount);

			retrievePhysicalDevices(
				_vkInstance,
				_physicalDevices,
				_deviceCount
			);
		}
	}

	void Device::retrieveQueueFamilies(
		const VkPhysicalDevice &_physicalDevice,
		std::vector<VkQueueFamilyProperties>	&_queueFamilies,
		uint32_t              								_queueFamilyCount
	) noexcept
	{
		vkGetPhysicalDeviceQueueFamilyProperties(
			_physicalDevice,
			&_queueFamilyCount,
			_queueFamilies.data()
		);

		if(_queueFamilies.empty() && _queueFamilyCount > 0)
		{
			_queueFamilies.resize(_queueFamilyCount);

			retrieveQueueFamilies(
				_physicalDevice,
				_queueFamilies,
				_queueFamilyCount
			);
		}
	}

	void Device::retrieveDeviceExtensions(
		const VkPhysicalDevice							&_physicalDevice,
		std::vector<VkExtensionProperties>	&_availableExtensions,
		uint32_t														_extensionCount
	) noexcept
	{
		const auto &result = vkEnumerateDeviceExtensionProperties(
			_physicalDevice,
			nullptr,
			&_extensionCount,
			_availableExtensions.data()
		);
		ASSERT_VK(result, "Failed to retrieve available device extensions!");

		if(_availableExtensions.empty() && _extensionCount > 0)
		{
			_availableExtensions.resize(_extensionCount);

			retrieveDeviceExtensions(
				_physicalDevice,
				_availableExtensions,
				_extensionCount
			);
		}
	}

	uint32_t Device::getMemoryType(
		uint32_t										_typeBits,
		const VkMemoryPropertyFlags	&_props,
		VkBool32										*_isFound
	) const noexcept
	{
		const auto &memProps = m_data.memProps;

		for(auto i = 0u; i < memProps.memoryTypeCount; i++)
		{
			if((_typeBits & 1) == 1)
			{
				if((memProps.memoryTypes[i].propertyFlags & _props) == _props)
				{
					if(_isFound)
					{
						*_isFound = true;
					}
					return i;
				}
			}
			_typeBits >>= 1;
		}

		if(_isFound)
		{
			*_isFound = false;
			return 0;
		}
		else
		{
			FATAL_ERROR_LOG("Failed to find a matching memory type!");
		}
	}
}