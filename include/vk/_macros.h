#ifndef MACROS_H_
#define MACROS_H_

#include "_pch.h"

//#define NDEBUG

	#ifndef NDEBUG

		#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__) && !defined(__GNUG__)
			#define __PRETTY_FUNCTION__ __FUNCSIG__
		#endif

		#ifdef WIN32

			#define RED_COLOR			FOREGROUND_RED
			#define GREEN_COLOR		FOREGROUND_GREEN
			#define BLUE_COLOR		(FOREGROUND_BLUE | FOREGROUND_INTENSITY)
			#define YELLOW_COLOR	(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)

		#else

			#define RESET_COLOR   "\x1B[0m"
			#define RED_COLOR     "\x1B[31m"
			#define GREEN_COLOR		"\x1B[32m"
			#define BLUE_COLOR		"\x1B[36m"
			#define YELLOW_COLOR  "\x1B[33m"

		#endif

		#define DEBUG_LOG(message, ...)						vk::debug::COLOR_PRINTF(message, BLUE_COLOR,		"\n\n",	##__VA_ARGS__);
		#define TRACE_LOG(message, ...)						vk::debug::COLOR_PRINTF(message, BLUE_COLOR,		"\n",		##__VA_ARGS__);
		#define INFO_LOG(message, ...)						vk::debug::COLOR_PRINTF(message, GREEN_COLOR,		"\n\n",	##__VA_ARGS__);
		#define WARN_LOG(message, ...)						vk::debug::COLOR_PRINTF(message, YELLOW_COLOR,	"\n\n",	##__VA_ARGS__);
		#define ERROR_LOG(message, ...)						vk::debug::COLOR_PRINTF(message, RED_COLOR, 		"\n\n",	##__VA_ARGS__);
		#define CRITICAL_ERROR_LOG(message, ...)	vk::debug::COLOR_PRINTF(message, RED_COLOR, 		"\n\n",	##__VA_ARGS__);
		#define FATAL_ERROR_LOG(message, ...)			vk::debug::COLOR_PRINTF(message, RED_COLOR, 		"\n\n",	##__VA_ARGS__); std::abort();

		#define ASSERT(x, message)								if(!(x)) vk::debug::PRINT_ERR(message);
		#define ASSERT_VK(x, message)							ASSERT((x) == VK_SUCCESS, (message));
		#define ASSERT_FATAL(x, message)					ASSERT(x, message); if(!(x)) { vk::debug::PRINT_STACKTRACE(); std::abort(); }

		#define ASSERT_ENUMS(...)									vk::debug::ENUMS_ASSERT<__VA_ARGS__>();
		#define ASSERT_STATIC_VAR(variable)				vk::debug::STATIC_VAR_MACRO_ASSERT(variable); static_assert(variable, "static variable " #variable " not valid or defined correctly!");

		#ifndef ENABLE_VK_VARS_ASSERT
			#error "Add compile def ENABLE_VK_VARS_ASSERT in cmake and use macro ASSERT_VK_STATIC_VARS_FUNC() in the main.cpp to assert initialization of vulkan app static variables."
		#else
			#define ASSERT_VK_STATIC_VARS()      		ASSERT_STATIC_VAR(vk::Attachment::s_attCount)						\
																							ASSERT_STATIC_VAR(vk::RenderPass::s_subpassCount)       \
																							ASSERT_STATIC_VAR(vk::RenderPass::s_spDepCount)         \
																							ASSERT_STATIC_VAR(vk::Pipeline	::s_pipelineCount)      \
																							ASSERT_STATIC_VAR(vk::Buffer		::s_bufferCount)        \
																							ASSERT_STATIC_VAR(vk::Buffer		::s_ubcCount)						\
																							ASSERT_STATIC_VAR(vk::Texture		::s_samplerCount)       \
																							ASSERT_STATIC_VAR(vk::Descriptor::s_layoutBindingCount)	\
																							ASSERT_STATIC_VAR(vk::Descriptor::s_setLayoutCount)			\
																							ASSERT_STATIC_VAR(vk::Model			::s_modelCount)
			#define ASSERT_VK_STATIC_VARS_FUNC()		static void VK_STATIC_VARS_ASSERT() { ASSERT_VK_STATIC_VARS() }
		#endif

		#define ERROR_VK(message)									ERROR_LOG(message);
		#define WARN_VK(message)									WARN_LOG(message);
		#define TRACE_VK(message)									TRACE_LOG(message);

	#else

		#define DEBUG_LOG(message, ...);
		#define TRACE_LOG(message, ...);
		#define INFO_LOG(message, ...);
		#define WARN_LOG(message, ...);
		#define ERROR_LOG(message, ...);
		#define CRITICAL_ERROR_LOG(message, ...);
		#define FATAL_ERROR_LOG(message, ...) std::abort();

		#define ASSERT(x, message);
		#define ASSERT_VK(x, message);
		#define ASSERT_FATAL(x, message, ...);
		#define ASSERT_ENUMS(...);
		#define ASSERT_STATIC_VAR(member);
		#define ASSERT_VK_STATIC_VARS();
		#define ASSERT_VK_STATIC_VARS_FUNC();

		#define ERROR_VK(message);
		#define WARN_VK(message);
		#define TRACE_VK(message);

	#endif

	#define STACK_ONLY(Type)															\
		private:          																	\
			Type() = default;																	\
		public:																							\
			static auto create() noexcept { return Type(); }

#endif