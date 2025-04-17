cmake_minimum_required(VERSION 3.10)

project(CppTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(CppTestApp src/main.cpp)

# Link libraries
target_link_libraries(CppTestApp PRIVATE Vulkan::Vulkan glfw glm::glm)

# Include directories (GLFW needs this, Vulkan might too)
target_include_directories(CppTestApp PRIVATE ${Vulkan_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS} ${glm_INCLUDE_DIRS})

# Basic output directory setup (optional but good practice)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) 