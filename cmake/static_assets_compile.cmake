###########################
# STATIC ASSETS COMPILE
###########################

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

if(CMAKE_ARGC LESS 6)
    message(FATAL_ERROR "Usage: cmake -P cmake/static_assets_compile.cmake <BUILD_ENV_FILENAME> <SOURCE_DIR_PATH> <BIN_DIR_PATH>")
endif()

set(BUILD_ENV_FILE              "${CMAKE_ARGV3}")
set(SOURCE_DIR                  "${CMAKE_ARGV4}")
set(BIN_DIR                     "${CMAKE_ARGV5}")

include(${SOURCE_DIR}/cmake/env_var_parser.cmake)

if(NOT DEFINED ENV{ASSETS_PATH})
    set(ASSETS_PATH             assets)
else()
    set(ASSETS_PATH             $ENV{ASSETS_PATH})
endif()

set(SHADERS_PATH                ${ASSETS_PATH}/shaders)
#set(TEXTURES_PATH               ${ASSETS_PATH}/textures)

set(SPV_COMPILE_SCRIPT          ./spirv_static_compile.sh)
#set(KTX_COMPILE_SCRIPT          ./ktx_convert.sh)

find_program(SHELL bash HINTS /bin)

execute_process(
    # Copy SPIRV Static Compile Script
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SOURCE_DIR}/${SPV_COMPILE_SCRIPT}
    ${BIN_DIR}

#    # Copy KTX Compile Script
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#    ${SOURCE_DIR}/${KTX_COMPILE_SCRIPT}
#    ${BIN_DIR}

    # Copy All Assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SOURCE_DIR}/${ASSETS_PATH}
    ${BIN_DIR}/${ASSETS_PATH}

    COMMAND_ERROR_IS_FATAL ANY
)
message("\nAll assets were copied to build directory!")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env ${SHELL} -c
    "SHADERS_PATH=${SHADERS_PATH} . ${SPV_COMPILE_SCRIPT}"

    WORKING_DIRECTORY ${BIN_DIR}
    COMMAND_ERROR_IS_FATAL ANY
)
#execute_process(
#    COMMAND ${CMAKE_COMMAND} -E env ${SHELL} -c
#    "TEXTURES_PATH=${TEXTURES_PATH} . ${KTX_COMPILE_SCRIPT}"
#
#    WORKING_DIRECTORY ${BIN_DIR}
#    COMMAND_ERROR_IS_FATAL ANY
#)
