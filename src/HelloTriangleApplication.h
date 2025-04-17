#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Forward declare GLFWwindow if needed, though including glfw3.h is usually fine

// Define constants or include them from a separate header if preferred
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication
{
public:
  void run();

// Make constructor/destructor explicit if needed later
// HelloTriangleApplication();
// ~HelloTriangleApplication();

private:
  GLFWwindow* _window = nullptr;
  VkInstance _instance = VK_NULL_HANDLE;

  void initWindow();
  void initVulkan();
  void createInstance();
  void mainLoop();
  void cleanup();
}; 