# Sticky Paws - CMake Build Configuration
#
# This file configures the build system for Sticky Paws, a small game
# built with Raylib. It supports multiple platforms including desktop
# (Windows, Linux, macOS) and Web (via Emscripten).
#
# Requirements:
#   - CMake 3.25 or higher
#   - C compiler (Clang, GCC, or MSVC)
#   - For Web builds: Emscripten SDK
#
# Usage:
#   cmake -S . -B build
#   cmake --build build

cmake_minimum_required(VERSION 3.25)
project(sticky-paws)

include(FetchContent)

# Generate compile_commands.json for language servers and tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# === Dependencies ===
# Automatically fetch Raylib from GitHub if not found locally
set(RAYLIB_VERSION 5.5)

FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(raylib)

# === Project Configuration ===
# Create the main executable
add_executable(${PROJECT_NAME})
add_subdirectory(src)

# Set output directory for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

# Set working directory for Visual Studio debugger
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# === Asset Management ===
# Copy game assets to build directory
if ("${PLATFORM}" STREQUAL "Web")
  # For Web builds, copy assets before compilation (Emscripten needs them earlier)
  add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/../assets
    )
else()
  # For desktop builds, copy assets after compilation
  add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )
endif()

# Link Raylib to our executable
target_link_libraries(${PROJECT_NAME} raylib)

# === Web Platform Configuration ===
if ("${PLATFORM}" STREQUAL "Web")
  # Configure for Emscripten/WebAssembly build
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
  target_link_options(${PROJECT_NAME} PUBLIC -sUSE_GLFW=3 PUBLIC --preload-file assets)
endif()

# === macOS Platform Configuration ===
# Link required frameworks on macOS
if (APPLE)
  target_link_libraries(${PROJECT_NAME} "-framework IOKit")
  target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
  target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
