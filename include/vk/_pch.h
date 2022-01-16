#ifndef PCH_H_
#define PCH_H_

//#define NDEBUG
#include <cassert>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <set>
#include <functional>
#include <algorithm>
#include <string>
#include <map>
//#include <utility>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <Windows.h>
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#endif