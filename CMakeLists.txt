cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(TARGET_NAME DeferredRenderer)
project(
    ${TARGET_NAME}
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY                  ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG            ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE          ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO   ${CMAKE_CURRENT_BINARY_DIR})

set(BUILD_ENV_FILE ${CMAKE_CURRENT_SOURCE_DIR}/.env.build)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/env_var_parser.cmake)

include_directories(include)
include_directories(externals/tinygltf)
#include_directories(externals/ktx/include)
#include_directories(externals/ktx/other_include)

find_package(glfw3  REQUIRED)
find_package(glm    REQUIRED)
find_package(Ktx    REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(${TARGET_NAME})

file(
    GLOB_RECURSE ${TARGET_NAME}_SOURCE

    ${PROJECT_SOURCE_DIR}/include/*.h
    ${PROJECT_SOURCE_DIR}/include/*.hpp
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_compile_definitions(GLFW_INCLUDE_VULKAN ENABLE_VK_VARS_ASSERT)

if($ENV{PCH_ENABLED})
    target_precompile_headers(${TARGET_NAME} PRIVATE include/vk/_pch.h)
    set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
endif()

target_sources(${TARGET_NAME} PRIVATE ${${TARGET_NAME}_SOURCE})

target_link_libraries(${TARGET_NAME} PRIVATE glfw)
target_link_libraries(${TARGET_NAME} PRIVATE glm::glm)
target_link_libraries(${TARGET_NAME} PRIVATE KTX::ktx)
target_link_libraries(${TARGET_NAME} PRIVATE Vulkan::Vulkan)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_link_libraries(${TARGET_NAME} PRIVATE Dbghelp.lib)
endif()

add_custom_target(
    ${TARGET_NAME}_ASSET_COMPILE ALL

    COMMAND ${CMAKE_COMMAND} -P
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/static_assets_compile.cmake
    ${BUILD_ENV_FILE}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}

    COMMENT "Copying Assets, Compiling Shaders/Textures & Removing Redundant files."
    VERBATIM
)

# Doxygen
#############################################################################

if($ENV{DOC_BUILD_ENABLED})
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/private/doxygen.cmake)
endif()