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

	template<typename TElement, std::size_t count>
	using STLArray = std::array<TElement, count>;

	template<typename TElement>
	using STLVector = std::vector<TElement>;

	template<typename TElement, std::size_t count>
	class Array : public STLArray<TElement, count>
	{
		public:
			Array() = default;
			Array(const std::initializer_list<TElement> &_list)
			{
				if(_list.size() > count)
				{ FATAL_ERROR_LOG("Initializer list size does not match array size"); }

				std::move(_list.begin(), _list.end(), this->begin());
			}

		public:
			template<typename TIndex>
			const TElement& operator[](TIndex _index) const
			{
				static_assert(
					std::is_integral<TIndex>::value || std::is_enum<TIndex>::value,
					"_index value should be either an integral or enum type"
				);

				return STLArray<TElement, count>::operator[](static_cast<std::size_t>(_index));
			}

			template<typename TIndex>
			TElement& operator[](TIndex _index)
			{
				static_assert(
					std::is_integral<TIndex>::value || std::is_enum<TIndex>::value,
					"_index value should be either an integral or enum type"
				);

				return STLArray<TElement, count>::operator[](static_cast<std::size_t>(_index));
			}
	};

	template<typename TElement>
	class Vector : public STLVector<TElement>
	{
		public:
			template<typename TIndex>
			const TElement& operator[](TIndex _index) const
			{
				static_assert(
					std::is_integral<TIndex>::value || std::is_enum<TIndex>::value,
					"_index value should be either an integral or enum type"
				);

				return STLVector<TElement>::operator[](static_cast<std::size_t>(_index));
			}

			template<typename TIndex>
			TElement& operator[](TIndex _index)
			{
				static_assert(
					std::is_integral<TIndex>::value || std::is_enum<TIndex>::value,
					"_index value should be either an integral or enum type"
				);

				return STLVector<TElement>::operator[](static_cast<std::size_t>(_index));
			}
	};

	template<typename TInt = uint16_t, typename TEnum>
	inline static constexpr TInt toInt(TEnum _enum) noexcept
	{
		static_assert(
			std::is_integral<TInt>::value,
			"TInt type should be an integral type"
		);

		static_assert(
			std::is_enum<TEnum>::value,
			"_enum value should be an enum type"
		);

		return static_cast<TInt>(_enum);
	}

	namespace debug
	{
		inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT _messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *_pCallBackData,
			void *_pUserData
		) noexcept
		{
			if(_messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			{
				switch (_messageSeverity)
				{
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   ERROR_VK(_pCallBackData->pMessage); break;
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	WARN_VK	(_pCallBackData->pMessage); break;
					default:                        											TRACE_VK(_pCallBackData->pMessage); break;
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
			const VkDebugUtilsMessengerCreateInfoEXT *_pInfo,
			const VkAllocationCallbacks *_pAllocator,
			VkDebugUtilsMessengerEXT *_pMessenger
		) noexcept
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
				_instance,
				"vkCreateDebugUtilsMessengerEXT"
			);

			return func != nullptr ? func(
				_instance,
				_pInfo,
				_pAllocator,
				_pMessenger
			) : VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		template<typename TEnum>
		static void isEnumDefined() noexcept
		{
			static_assert(
				std::is_enum<decltype(TEnum::_count_)>::value
			);
		}
	}
}