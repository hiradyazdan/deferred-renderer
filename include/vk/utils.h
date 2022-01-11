#ifndef UTILS_H_
#define UTILS_H_

#include "_macros.h"
#include "Types.h"

namespace vk::debug
{
#ifndef NDEBUG

	template<typename T>
	struct is_constexpr_context
	{ static constexpr bool value = false; };

	template<typename TReturn, typename... TArgs>
	struct is_constexpr_context<TReturn(TArgs...)>
	{ static constexpr bool value = true; };

	template<typename TFunc>
	static auto getReturnType(TFunc &&_func)
	{ return _func(); }

	template<typename TContainer, typename TIndex>
	static auto getReturnType(TContainer &&_container, TIndex &&_index)
	{ return std::forward<TContainer>(_container)[std::forward<TIndex>(_index)]; }

	template<typename TContainer, typename TIndex>
	static auto getReturnType(TIndex &&_index)
	{ return std::declval<TContainer>()[std::forward<TIndex>(_index)]; }

	template<typename... TVars, typename TVarType = uint16_t>
	static constexpr void STATIC_VAR_MACRO_ASSERT(TVars ..._vars) noexcept
	{
		static_assert(
			(std::is_same_v<TVars, TVarType> && ...),
			"This macro only takes a static variable with the type 'const uint16_t'"
		);
	}

	template<typename TVar, typename TVarType = uint16_t>
	static constexpr void STATIC_VAR_MACRO_ASSERT(TVar) noexcept
	{
		static_assert(
			std::is_same_v<TVar, TVarType>,
			"This macro only takes a static variable with the type 'const uint16_t'"
		);
	}

	template<typename... TEnums>
	static void ENUMS_ASSERT() noexcept
	{
		static_assert((std::is_enum_v<decltype(TEnums::_count_)> && ...));
	}

	template<typename... TArgs>
	static void COLOR_PRINTF(
		const char *_message, int _color, const char *_lineSpace, TArgs ..._args
	) noexcept
	{
#ifdef WIN32
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), _color);
		printf(_message, std::forward<TArgs>(_args)...);
#else
		printf((_color _message RESET_COLOR), std::forward<TArgs>(_args)...);
#endif
		printf("%s", _lineSpace);
	}

	inline static void PRINT_ERR(const std::string &_message) noexcept
	{
		std::cerr << "\x1B[31m" << "Assertion Failed: " << (_message) << "\n" << "\x1B[0m" << std::endl;
	}

	inline static void PRINT_ERR(const char *_message) noexcept
	{
		ERROR_LOG("Assertion Failed: %s", _message);
	}

	inline static void PRINT_STACKTRACE()
	{
		const int maxFrames = 64;
		void* frames[maxFrames];

#if defined(__GNUC__) || defined(__GNUG__) //|| defined(__clang__)
		int framesCount = backtrace(frames, maxFrames);

		char** symbols = backtrace_symbols(frames, framesCount);

		TRACE_LOG("Call Stack:");
		for(auto i = 0u; i < framesCount; i++)
		{
			TRACE_LOG("  [%i]: %s", symbols[i]);
		}
		free(symbols);
#elif defined(_MSC_VER)
		unsigned short framesCount = CaptureStackBackTrace(0, maxFrames, frames, nullptr);

		SymInitialize(GetCurrentProcess(), nullptr, TRUE);

		char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		auto symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		IMAGEHLP_LINE64 line;
		DWORD displacement;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		TRACE_LOG("Call Stack:");
		for(auto i = 0u; i < framesCount; i++)
		{
			auto &frame		= frames[i];
			auto address	= (DWORD64)(frame);
			SymFromAddr(GetCurrentProcess(), address, nullptr, symbol);

			if(SymGetLineFromAddr64(GetCurrentProcess(), address, &displacement, &line))
			{
				TRACE_LOG("  [%d]: %s(%d): %s", i, line.FileName, line.LineNumber, symbol->Name);
			}
		}

		free(symbol);
#endif
	}

#endif

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
}

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

	template<typename TElement>
	using STLList = std::initializer_list<TElement>;

	template<typename TElement, std::size_t count>
	class Array : public STLArray<TElement, count>
	{
		public:
			template<typename... TArgs>
			constexpr Array(TArgs ..._args) noexcept
			: STLArray<TElement, count>{{std::forward<TArgs>(_args)...}}
			{
				static_assert(
					sizeof...(_args) <= count,
					"too many initializers"
				);
				static_assert(
					std::conjunction_v<std::is_convertible<TArgs, TElement>...>,
					"invalid argument types"
				);

				STLArray<TElement, count>::operator=(STLArray<TElement, count>{
					std::forward<TArgs>(_args)...
				});
			}

		public:
			template<typename TIndex>
			constexpr const TElement& operator[](TIndex _index) const noexcept
			{
				static_assert(
					std::is_integral<TIndex>::value || std::is_enum<TIndex>::value,
					"_index value should be either an integral or enum type"
				);

				auto index = static_cast<std::size_t>(_index);
				ASSERT_FATAL(index < count, "Array subscript index out of range");

				return STLArray<TElement, count>::operator[](index);
			}

			template<typename TIndex>
			constexpr TElement& operator[](TIndex _index) noexcept
			{
				static_assert(
					std::is_integral<TIndex>::value || std::is_enum<TIndex>::value,
					"_index value should be either an integral or enum type"
				);

				auto index = static_cast<std::size_t>(_index);
				ASSERT_FATAL(index < count, "Array subscript index out of range");

				return STLArray<TElement, count>::operator[](index);
			}
	};

	template<typename TElement>
	class Vector : public STLVector<TElement>
	{
		public:
			Vector() = default;
			Vector(const STLList<TElement> &_list) noexcept
			: Vector<TElement>()
			{
				auto i = 0u;
				for(const auto &element : _list)
				{
					(*this)[i++] = element;
				}
			}

		public:
			template<typename TIndex>
			const TElement& operator[](TIndex _index) const noexcept
			{
				static_assert(
					std::is_integral<TIndex>::value || std::is_enum<TIndex>::value,
					"_index value should be either an integral or enum type"
				);

				auto index = static_cast<std::size_t>(_index);
				ASSERT_FATAL(index < this->size(), "Vector subscript index out of range");

				return STLVector<TElement>::operator[](static_cast<std::size_t>(_index));
			}

			template<typename TIndex>
			TElement& operator[](TIndex _index) noexcept
			{
				static_assert(
					std::is_integral<TIndex>::value || std::is_enum<TIndex>::value,
					"_index value should be either an integral or enum type"
				);

				auto index = static_cast<std::size_t>(_index);
				ASSERT_FATAL(index < this->size(), "Vector subscript index out of range");

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
}

#endif