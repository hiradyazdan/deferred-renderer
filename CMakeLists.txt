cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(TARGET_NAME DeferredRenderer)
project(
    ${TARGET_NAME}
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY                  ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG            ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE          ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO   ${CMAKE_CURRENT_BINARY_DIR})

include_directories(include)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(${TARGET_NAME})

file(
    GLOB_RECURSE ${TARGET_NAME}_SOURCE

    ${PROJECT_SOURCE_DIR}/include/*.h
    ${PROJECT_SOURCE_DIR}/include/*.hpp
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

target_precompile_headers(${TARGET_NAME} PRIVATE include/pch.h)
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)

add_compile_definitions(GLFW_INCLUDE_VULKAN)

target_sources(${TARGET_NAME} PRIVATE ${${TARGET_NAME}_SOURCE})

target_link_libraries(${TARGET_NAME} PRIVATE glfw)
target_link_libraries(${TARGET_NAME} PRIVATE Vulkan::Vulkan)