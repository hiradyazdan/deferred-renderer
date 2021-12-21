#ifndef MACROS_H_
#define MACROS_H_

#include "pch.h"

#ifndef NDEBUG

	#define ASSERT(x, message)			if(!(x)) std::cerr << "Assertion Failed @: " << __FILE__ << ":" << __LINE__ << ": " << message << "\n\n";
	#define ASSERT_VK(x, message)		ASSERT(x == VK_SUCCESS, message);

	#define ERROR_VK(message)				std::cerr << message << "\n\n";
	#define WARN_VK(message)				std::cerr << message << "\n\n";
	#define TRACE_VK(message)				std::cout << message << "\n\n";

	#define FATAL_ERROR(message, x)	std::cerr << message << ": " << x << "\n\n"; std::abort();

#endif

#endif