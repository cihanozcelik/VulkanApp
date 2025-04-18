cmake_minimum_required(VERSION 3.10)

project(VulkanApp CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Add source files to the executable
add_executable(VulkanApp
  src/main.cpp
  src/core/Application.cpp
  src/platform/Window.cpp
  src/vulkan/VulkanInstance.cpp
  src/vulkan/VulkanDevice.cpp
  src/vulkan/VulkanSwapChain.cpp
  # Add other .cpp files here later
)

# Link libraries
target_link_libraries(VulkanApp PRIVATE Vulkan::Vulkan glfw glm::glm)

# Include directories (GLFW needs this, Vulkan might too)
target_include_directories(VulkanApp PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src # Allow includes relative to src
  ${Vulkan_INCLUDE_DIRS}
  ${glfw3_INCLUDE_DIRS}
  ${glm_INCLUDE_DIRS}
)

# Basic output directory setup (optional but good practice)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Add option for Debug/Release build type if not set by preset/toolchain
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}") 