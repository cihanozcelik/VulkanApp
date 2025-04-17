#include "HelloTriangleApplication.h"

#include <cstdlib> // Included for std::to_string potentially, though <string> is better
#include <iostream>
#include <stdexcept>
#include <string> // Include for std::to_string
#include <vector>

void HelloTriangleApplication::run()
{
  initWindow();
  initVulkan();
  mainLoop();
  cleanup();
}

void HelloTriangleApplication::initWindow()
{
  if (!glfwInit())
  {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  _window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Window", nullptr, nullptr);
  if (!_window)
  {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }
  std::cout << "GLFW window created successfully." << std::endl;
}

void HelloTriangleApplication::initVulkan()
{
  createInstance();
}

void HelloTriangleApplication::createInstance()
{
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  if (glfwExtensions == nullptr)
  {
    throw std::runtime_error("GLFW required extensions unavailable.");
  }

  std::vector<const char*> requiredExtensions(glfwExtensions,
                                          glfwExtensions + glfwExtensionCount);

  requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#ifdef VK_KHR_get_physical_device_properties2
  requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#else
#error \
    "VK_KHR_get_physical_device_properties2 not defined, required for MoltenVK portability"
#endif

  createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
  createInfo.ppEnabledExtensionNames = requiredExtensions.data();

  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = nullptr;

  createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

  VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);

  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Vulkan instance! Error code: " +
                             std::to_string(result));
  }
  std::cout << "Vulkan instance created successfully." << std::endl;
}

void HelloTriangleApplication::mainLoop()
{
  while (!glfwWindowShouldClose(_window))
  {
    glfwPollEvents();
  }
}

void HelloTriangleApplication::cleanup()
{
  if (_instance != VK_NULL_HANDLE)
  {
    vkDestroyInstance(_instance, nullptr);
    std::cout << "Vulkan instance destroyed." << std::endl;
  }

  if (_window)
  {
    glfwDestroyWindow(_window);
    std::cout << "GLFW window destroyed." << std::endl;
  }

  glfwTerminate();
  std::cout << "GLFW terminated." << std::endl;
} 