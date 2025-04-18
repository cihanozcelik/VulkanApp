#pragma once

#define GLFW_INCLUDE_VULKAN // Ensure Vulkan headers are included by GLFW
#include <GLFW/glfw3.h>
#include <string>
#include <cstdint> // For uint32_t

class Window
{
public:
  Window(uint32_t width, uint32_t height, const std::string& title);
  ~Window();

  // Delete copy constructor and assignment operator
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  bool shouldClose() const;
  void pollEvents() const;
  VkSurfaceKHR createSurface(VkInstance instance); // Needs Vulkan instance

  // Accessors
  GLFWwindow* getGLFWwindow() const { return _glfwWindow; }
  VkExtent2D getFramebufferExtent() const;

private:
  GLFWwindow* _glfwWindow = nullptr;
  uint32_t _width;
  uint32_t _height;
  std::string _title;

  void initGLFW();
  void createWindow();
  void cleanupGLFW();
}; 