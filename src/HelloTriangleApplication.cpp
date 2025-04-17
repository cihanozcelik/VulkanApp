#include "HelloTriangleApplication.h"

#include <cstring> // For strcmp
#include <iostream>
#include <set> // For checking validation layer support efficiently
#include <stdexcept>
#include <string> // Include for std::to_string
#include <vector>

// Helper proxy function to load vkCreateDebugUtilsMessengerEXT
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

// Helper proxy function to load vkDestroyDebugUtilsMessengerEXT
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    func(instance, debugMessenger, pAllocator);
  }
}

// --- Class Implementation ---

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
  setupDebugMessenger(); // Setup messenger after instance creation
}

void HelloTriangleApplication::createInstance()
{
  if (enableValidationLayers && !checkValidationLayerSupport())
  {
    throw std::runtime_error("Validation layers requested, but not available!");
  }

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

  // Get required extensions (now uses helper function)
  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  // Enable validation layers if requested
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{}; // Need this outside the if for potential use in createInfo.pNext
  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    // Populate debug messenger create info structure (for setupDebugMessenger later)
    populateDebugMessengerCreateInfo(debugCreateInfo);
    // createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo; // Optionally log instance creation/destruction
  }
  else
  {
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.pNext = nullptr;
  }

  // MoltenVK Portability Flag
  createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

  // Create the instance
  VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Vulkan instance! Error code: " +
                             std::to_string(result));
  }
  std::cout << "Vulkan instance created successfully." << std::endl;
}

void HelloTriangleApplication::setupDebugMessenger()
{
  if (!enableValidationLayers) return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  VkResult result = CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to set up debug messenger! Error code: " +
                             std::to_string(result));
  }
  std::cout << "Vulkan debug messenger created successfully." << std::endl;
}


bool HelloTriangleApplication::checkValidationLayerSupport()
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  std::cout << "Available validation layers:" << std::endl;
  std::set<std::string> availableLayerNames;
  for (const auto& layerProperties : availableLayers)
  {
      std::cout << "\t" << layerProperties.layerName << std::endl;
      availableLayerNames.insert(layerProperties.layerName);
  }

  std::cout << "Required validation layers:" << std::endl;
  for (const char* layerName : validationLayers)
  {
     std::cout << "\t" << layerName << std::endl;
    if (availableLayerNames.find(layerName) == availableLayerNames.end())
    {
      std::cerr << "ERROR: Required layer " << layerName << " not found!" << std::endl;
      return false;
    }
  }

  return true;
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions()
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  if (glfwExtensions == nullptr)
  {
    throw std::runtime_error("GLFW required extensions unavailable.");
  }

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  // Add MoltenVK portability extensions
  extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

  // Add Debug Utils extension if validation layers are enabled
  if (enableValidationLayers)
  {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  // You could print the required extensions here for debugging
  std::cout << "Required instance extensions:" << std::endl;
  for (const auto& ext : extensions) {
      std::cout << "\t" << ext << std::endl;
  }

  return extensions;
}

// Definition for the static debug callback function
VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
  // Filter out less important messages if desired
  // if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
  {
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
  }

  // Must return VK_FALSE - see Vulkan spec
  return VK_FALSE;
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
  createInfo = {}; // Zero initialize
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  // Specify which severity levels trigger the callback
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  // Specify which message types trigger the callback
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  // Pointer to our callback function
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr; // Optional user data pointer
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
  // Destroy debug messenger BEFORE the instance
  if (enableValidationLayers && _debugMessenger != VK_NULL_HANDLE)
  {
    DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    std::cout << "Vulkan debug messenger destroyed." << std::endl;
  }

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