#include "HelloTriangleApplication.h"

#include <algorithm> // For std::clamp
#include <cstdint>   // For uint32_t limits
#include <cstring>   // For strcmp
#include <iostream>
#include <limits>    // For std::numeric_limits
#include <map>       // For ranking devices (optional)
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

// Helper function prototypes (defined below class implementation)
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator);

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
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();    // Create swap chain after logical device
  createImageViews(); // Create image views after swap chain
}

void HelloTriangleApplication::createSurface()
{
  VkResult result = glfwCreateWindowSurface(_instance, _window, nullptr, &_surface);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create window surface! Error code: " +
                             std::to_string(result));
  }
  std::cout << "Vulkan window surface created successfully." << std::endl;
}

void HelloTriangleApplication::pickPhysicalDevice()
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

  if (deviceCount == 0)
  {
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

  std::cout << "Available physical devices (" << deviceCount << ") :" << std::endl;

  for (const auto& device : devices)
  {
     VkPhysicalDeviceProperties properties;
     vkGetPhysicalDeviceProperties(device, &properties);
     std::cout << "\t" << properties.deviceName;

    if (isDeviceSuitable(device))
    {
      _physicalDevice = device;
      std::cout << " (Selected)" << std::endl;
      break; 
    }
    std::cout << std::endl;
  }

  if (_physicalDevice == VK_NULL_HANDLE)
  {
    throw std::runtime_error("Failed to find a suitable GPU!");
  }
}

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device)
{
  QueueFamilyIndices indices = findQueueFamilies(device);
  bool extensionsSupported = checkDeviceExtensionSupport(device);

  // Check swap chain support if extensions are available
  bool swapChainAdequate = false;
  if (extensionsSupported)
  {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device)
{
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto& queueFamily : queueFamilies)
  {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      indices.graphicsFamily = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
    if (presentSupport)
    {
      indices.presentFamily = i;
    }

    if (indices.isComplete())
    {
      break;
    }
    i++;
  }
  return indices;
}

bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredDevExtensionsSet(deviceExtensions.begin(), deviceExtensions.end());

  bool requiresPortabilitySubset = false;
  for (const auto& extension : availableExtensions) {
      if (strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0) {
          requiresPortabilitySubset = true;
          std::cout << " [Device requires VK_KHR_portability_subset] ";
          break;
      }
  }
  if (requiresPortabilitySubset) {
      requiredDevExtensionsSet.insert("VK_KHR_portability_subset");
  }

  for (const auto& extension : availableExtensions)
  {
    requiredDevExtensionsSet.erase(extension.extensionName);
  }

  return requiredDevExtensionsSet.empty();
}

SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device)
{
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
  if (formatCount != 0)
  {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);
  if (presentModeCount != 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
  for (const auto& availableFormat : availableFormats)
  {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
        std::cout << "Swap Format: Found preferred B8G8R8A8_SRGB / NONLINEAR_KHR" << std::endl;
      return availableFormat;
    }
  }
  std::cout << "Swap Format: Preferred not found, using first available." << std::endl;
  return availableFormats[0]; 
}

VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes)
{
  for (const auto& availablePresentMode : availablePresentModes)
  {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
        std::cout << "Swap Present Mode: Mailbox" << std::endl;
      return availablePresentMode;
    }
  }
  std::cout << "Swap Present Mode: FIFO" << std::endl;
  return VK_PRESENT_MODE_FIFO_KHR; 
}

VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    std::cout << "Swap Extent: Using surface current extent (" 
              << capabilities.currentExtent.width << "x" 
              << capabilities.currentExtent.height << ")" << std::endl;
    return capabilities.currentExtent;
  }
  else
  {
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    std::cout << "Swap Extent: Using window framebuffer size clamped (" 
              << actualExtent.width << "x" << actualExtent.height << ")" << std::endl;
    return actualExtent;
  }
}

void HelloTriangleApplication::createSwapChain()
{
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice);

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && 
      imageCount > swapChainSupport.capabilities.maxImageCount)
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }
  std::cout << "Swap Chain Image Count: " << imageCount << std::endl;

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = _surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
    std::cout << "Swap Chain Sharing Mode: Concurrent" << std::endl;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    std::cout << "Swap Chain Sharing Mode: Exclusive" << std::endl;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  VkResult result = vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create swap chain! Error code: " + std::to_string(result));
  }
  std::cout << "Vulkan swap chain created successfully." << std::endl;

  vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
  _swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());

  _swapChainImageFormat = surfaceFormat.format;
  _swapChainExtent = extent;
}

void HelloTriangleApplication::createImageViews()
{
  _swapChainImageViews.resize(_swapChainImages.size());

  for (size_t i = 0; i < _swapChainImages.size(); i++)
  {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = _swapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = _swapChainImageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[i]);
    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create image view " + std::to_string(i) + 
                               "! Error code: " + std::to_string(result));
    }
  }
  std::cout << "Vulkan swap chain image views created successfully (" 
            << _swapChainImageViews.size() << ")." << std::endl;
}

void HelloTriangleApplication::createLogicalDevice()
{
  QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {
      indices.graphicsFamily.value(), 
      indices.presentFamily.value()
  };

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies)
  {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pEnabledFeatures = &deviceFeatures;

  std::vector<const char*> requiredDevExtensionsVec = deviceExtensions;
  uint32_t extCount;
  vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, nullptr);
  std::vector<VkExtensionProperties> availableExts(extCount);
  vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, availableExts.data());
  bool portabilityRequired = false;
  for (const auto& ext : availableExts) {
      if (strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0) {
          portabilityRequired = true;
          break;
      }
  }
  if (portabilityRequired) {
       requiredDevExtensionsVec.push_back("VK_KHR_portability_subset");
  }

  createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDevExtensionsVec.size());
  createInfo.ppEnabledExtensionNames = requiredDevExtensionsVec.data();

  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  VkResult result = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create logical device! Error code: " +
                             std::to_string(result));
  }
  std::cout << "Vulkan logical device created successfully." << std::endl;

  vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
  vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
  std::cout << "Graphics and present queue handles obtained." << std::endl;
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

  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    populateDebugMessengerCreateInfo(debugCreateInfo);
  }
  else
  {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

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

  extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

  if (enableValidationLayers)
  {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  std::cout << "Required instance extensions:" << std::endl;
  for (const auto& ext : extensions) {
      std::cout << "\t" << ext << std::endl;
  }
  return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
  {
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
  }
  return VK_FALSE;
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;
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
  for (auto imageView : _swapChainImageViews)
  {
    vkDestroyImageView(_device, imageView, nullptr);
  }
  std::cout << "Vulkan swap chain image views destroyed." << std::endl;

  if (_swapChain != VK_NULL_HANDLE)
  {
    vkDestroySwapchainKHR(_device, _swapChain, nullptr);
    std::cout << "Vulkan swap chain destroyed." << std::endl;
  }

  if (_device != VK_NULL_HANDLE)
  {
    vkDestroyDevice(_device, nullptr);
    std::cout << "Vulkan logical device destroyed." << std::endl;
  }

  if (_surface != VK_NULL_HANDLE)
  {
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    std::cout << "Vulkan surface destroyed." << std::endl;
  }

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

// --- Helper Function Definitions (Proxies) --- (Unchanged) ---

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