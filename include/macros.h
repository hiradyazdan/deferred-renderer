#ifndef MACROS_H_
#define MACROS_H_

#include "pch.h"
#include "_constants.h"

//#define NDEBUG

	#ifndef NDEBUG

		#ifdef WIN32

			#define RED_COLOR			FOREGROUND_RED
			#define GREEN_COLOR		FOREGROUND_GREEN
			#define BLUE_COLOR		(FOREGROUND_BLUE | FOREGROUND_INTENSITY)
			#define YELLOW_COLOR	(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)

			template<typename... TArgs>
			void COLOR_PRINTF(const char* const _message, int _color, TArgs ..._args)
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), _color);
				printf(_message, std::forward<TArgs>(_args)...);
				printf("\n\n");
			}

		#else

			#define RESET_COLOR   "\x1B[0m"
			#define RED_COLOR     "\x1B[31m"
			#define GREEN_COLOR		"\x1B[32m"
			#define BLUE_COLOR		"\x1B[36m"
			#define YELLOW_COLOR  "\x1B[33m"

			template<typename... TArgs>
			void COLOR_PRINTF(const char* const _message, int _color, TArgs ..._args)
			{
				printf((_color _message RESET_COLOR), std::forward<TArgs>(_args)...);
				printf("\n\n");
			}

		#endif

		#define DEBUG_LOG(message, ...)						COLOR_PRINTF(message, BLUE_COLOR, 	##__VA_ARGS__);
		#define TRACE_LOG(message, ...)						COLOR_PRINTF(message, BLUE_COLOR, 	##__VA_ARGS__);
		#define INFO_LOG(message, ...)						COLOR_PRINTF(message, GREEN_COLOR, 	##__VA_ARGS__);
		#define WARN_LOG(message, ...)						COLOR_PRINTF(message, YELLOW_COLOR, ##__VA_ARGS__);
		#define ERROR_LOG(message, ...)						COLOR_PRINTF(message, RED_COLOR, 		##__VA_ARGS__);
		#define CRITICAL_ERROR_LOG(message, ...)	COLOR_PRINTF(message, RED_COLOR, 		##__VA_ARGS__);
		#define FATAL_ERROR_LOG(message, ...)			COLOR_PRINTF(message, RED_COLOR, 		##__VA_ARGS__); std::abort();

		#define ASSERT(x, message)		if(!(x)) std::cerr << "Assertion Failed @: " << __FILE__ << ":" << __LINE__ << ": " << (message) << std::endl;
		#define ASSERT_VK(x, message)	ASSERT((x) == VK_SUCCESS, message);

		#define ERROR_VK(message)			ERROR_LOG(message);
		#define WARN_VK(message)			WARN_LOG(message);
		#define TRACE_VK(message)			TRACE_LOG(message);

	#else

		#define DEBUG_LOG(message, ...);
		#define TRACE_LOG(message, ...);
		#define INFO_LOG(message, ...);
		#define WARN_LOG(message, ...);
		#define ERROR_LOG(message, ...);
		#define CRITICAL_ERROR_LOG(message, ...);
		#define FATAL_ERROR_LOG(message, ...); std::abort();

		#define ASSERT(x, message);
		#define ASSERT_VK(x, message);

		#define ERROR_VK(message);
		#define WARN_VK(message);
		#define TRACE_VK(message);

	#endif

	#define STACK_ONLY(Type)										\
		private:          												\
			Type() = default;												\
		public:																		\
			static auto create() noexcept { return Type(); }

#endif