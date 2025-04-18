#include "Window.h"
#include <stdexcept>
#include <iostream> // For cout temporarily

Window::Window(uint32_t width, uint32_t height, const std::string& title)
    : _width(width), _height(height), _title(title)
{
  initGLFW();
  createWindow();
}

Window::~Window()
{
  cleanupGLFW();
}

void Window::initGLFW()
{
  if (!glfwInit())
  {
    throw std::runtime_error("Failed to initialize GLFW");
  }
  std::cout << "GLFW initialized." << std::endl;
}

void Window::createWindow()
{
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Keep non-resizable for now

  _glfwWindow = glfwCreateWindow(static_cast<int>(_width),
                                 static_cast<int>(_height),
                                 _title.c_str(),
                                 nullptr,
                                 nullptr);
  if (!_glfwWindow)
  {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }
  std::cout << "GLFW window created successfully." << std::endl;
}

VkSurfaceKHR Window::createSurface(VkInstance instance)
{
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkResult result = glfwCreateWindowSurface(instance, _glfwWindow, nullptr, &surface);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create window surface! Error code: " +
                             std::to_string(result));
  }
  std::cout << "Vulkan window surface created successfully." << std::endl;
  return surface;
}

VkExtent2D Window::getFramebufferExtent() const
{
    int width, height;
    glfwGetFramebufferSize(_glfwWindow, &width, &height);
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

bool Window::shouldClose() const
{
  return glfwWindowShouldClose(_glfwWindow);
}

void Window::pollEvents() const
{
  glfwPollEvents();
}

void Window::cleanupGLFW()
{
  if (_glfwWindow)
  {
    glfwDestroyWindow(_glfwWindow);
    std::cout << "GLFW window destroyed." << std::endl;
  }
  glfwTerminate();
  std::cout << "GLFW terminated." << std::endl;
} 