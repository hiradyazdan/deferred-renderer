#pragma once

#include "macros.h"
#include "Types.h"

namespace vk
{
	struct QueueFamilyIndices
	{
		int graphicsFamily  = -1;
		int presentFamily   = -1;

		bool isComplete() const noexcept
		{
			return graphicsFamily >= 0 &&
						 presentFamily  >= 0;
		}
	};

	namespace debug
	{
		inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT _messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *_callBackData,
			void *_userData
		) noexcept
		{
			if(_messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			{
				switch (_messageSeverity)
				{
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   ERROR_VK(_callBackData->pMessage); break;
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	WARN_VK	(_callBackData->pMessage); break;
					default:                        											TRACE_VK(_callBackData->pMessage); break;
				}
			}

			return VK_FALSE;
		}

		inline static void setMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &_info) noexcept
		{
			_info = {};
			_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

			_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT	|
															VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT	|
															VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
															VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
															VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

			_info.pfnUserCallback = debugCallback;
			_info.pUserData       = nullptr;
		}

		inline static VkResult setMessenger(
			const VkInstance &_instance,
			const VkDebugUtilsMessengerCreateInfoEXT *_info,
			const VkAllocationCallbacks *_allocator,
			VkDebugUtilsMessengerEXT *_messenger
		) noexcept
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
				_instance,
				"vkCreateDebugUtilsMessengerEXT"
			);

			return func != nullptr ? func(
				_instance,
				_info,
				_allocator,
				_messenger
			) : VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
}