cmake_minimum_required(VERSION 3.10)

project(VulkanApp CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Find the glslc compiler from the Vulkan SDK
find_program(GLSLC_EXECUTABLE glslc HINTS ENV VULKAN_SDK)
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc compiler not found. Make sure the Vulkan SDK is installed and VULKAN_SDK environment variable is set.")
endif()
message(STATUS "Found glslc: ${GLSLC_EXECUTABLE}")

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

# --- Shader Compilation ---
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(OUTPUT_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders) # Compile directly to build dir

# Ensure output directory exists
file(MAKE_DIRECTORY ${OUTPUT_SHADER_DIR})

# Define source shaders
set(VERTEX_SHADER_SOURCE ${SHADER_DIR}/shader.vert)
set(FRAGMENT_SHADER_SOURCE ${SHADER_DIR}/shader.frag)

# Define output SPIR-V files
set(VERTEX_SHADER_OUTPUT ${OUTPUT_SHADER_DIR}/vert.spv)
set(FRAGMENT_SHADER_OUTPUT ${OUTPUT_SHADER_DIR}/frag.spv)

# Command to compile vertex shader
add_custom_command(
    OUTPUT ${VERTEX_SHADER_OUTPUT}
    COMMAND ${GLSLC_EXECUTABLE} ${VERTEX_SHADER_SOURCE} -o ${VERTEX_SHADER_OUTPUT}
    DEPENDS ${VERTEX_SHADER_SOURCE}
    COMMENT "Compiling ${VERTEX_SHADER_SOURCE} -> ${VERTEX_SHADER_OUTPUT}"
    VERBATIM
)

# Command to compile fragment shader
add_custom_command(
    OUTPUT ${FRAGMENT_SHADER_OUTPUT}
    COMMAND ${GLSLC_EXECUTABLE} ${FRAGMENT_SHADER_SOURCE} -o ${FRAGMENT_SHADER_OUTPUT}
    DEPENDS ${FRAGMENT_SHADER_SOURCE}
    COMMENT "Compiling ${FRAGMENT_SHADER_SOURCE} -> ${FRAGMENT_SHADER_OUTPUT}"
    VERBATIM
)

# List of all shader outputs
set(SHADER_OUTPUTS
    ${VERTEX_SHADER_OUTPUT}
    ${FRAGMENT_SHADER_OUTPUT}
)

# Custom target to ensure shaders are compiled as part of the build process
add_custom_target(CompileShaders ALL DEPENDS ${SHADER_OUTPUTS})

# Make VulkanApp depend on the compiled shaders
add_dependencies(VulkanApp CompileShaders)

# --- End Shader Compilation ---

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
# Place executable in the build root for easier access to shaders/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib) # If needed later
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib) # If needed later


# Add option for Debug/Release build type if not set by preset/toolchain
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}") 