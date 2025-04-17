#include "HelloTriangleApplication.h"

#include <cstring> // For strcmp
#include <iostream>
#include <map> // For ranking devices (optional)
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
  createSurface();       // Create surface after instance
  pickPhysicalDevice();  // Pick GPU after surface creation
  createLogicalDevice(); // Create logical device using chosen physical device
}

void HelloTriangleApplication::createSurface()
{
  // Use GLFW function to create platform-agnostic surface
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

  // Optional: Use a map to store candidate devices and their scores
  // std::multimap<int, VkPhysicalDevice> candidates;

  for (const auto& device : devices)
  {
     VkPhysicalDeviceProperties properties;
     vkGetPhysicalDeviceProperties(device, &properties);
     std::cout << "\t" << properties.deviceName;

    if (isDeviceSuitable(device))
    {
      _physicalDevice = device;
      std::cout << " (Selected)" << std::endl;
      break; // Pick the first suitable device
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

  // Add more checks here later (e.g., swap chain adequacy, features)

  return indices.isComplete() && extensionsSupported;
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
    // Check for graphics support
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      indices.graphicsFamily = i;
    }

    // Check for presentation support (needs the surface)
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

  std::set<std::string> requiredDevExtensions(deviceExtensions.begin(), deviceExtensions.end());

  // --- Check for MoltenVK Portability Subset Extension --- 
  // This is needed on macOS/iOS if the device requires it.
  bool requiresPortabilitySubset = false;
  for (const auto& extension : availableExtensions) {
      if (strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0) {
          requiresPortabilitySubset = true;
          requiredDevExtensions.insert("VK_KHR_portability_subset");
          std::cout << " [Device requires VK_KHR_portability_subset] ";
          break;
      }
  }
  // --------------------------------------------------------

  // Check if all required extensions are available
  for (const auto& extension : availableExtensions)
  {
    requiredDevExtensions.erase(extension.extensionName);
  }

  return requiredDevExtensions.empty();
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

  // Specify device features we want to use (none for now)
  VkPhysicalDeviceFeatures deviceFeatures{};

  // Create the logical device
  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pEnabledFeatures = &deviceFeatures;

  // --- Enable required device extensions --- 
  std::vector<const char*> requiredDevExtensions = deviceExtensions;
  // Add portability subset if required by the physical device
  if (checkDeviceExtensionSupport(_physicalDevice)) { // Re-check to see if portability needed
       // Check if portability was added during checkDeviceExtensionSupport
      uint32_t extCount;
      vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, nullptr);
      std::vector<VkExtensionProperties> availableExts(extCount);
      vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, availableExts.data());
      for (const auto& ext : availableExts) {
          if (strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0) {
              bool already_added = false;
              for(const char* req_ext : requiredDevExtensions) {
                  if(strcmp(req_ext, "VK_KHR_portability_subset") == 0) {
                      already_added = true;
                      break;
                  }
              }
              if(!already_added) {
                 requiredDevExtensions.push_back("VK_KHR_portability_subset");
              }
              break;
          }
      }
  }
  createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDevExtensions.size());
  createInfo.ppEnabledExtensionNames = requiredDevExtensions.data();
  // -----------------------------------------

  // Link to device-level validation layers (consistent with instance layers)
  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  // Create the logical device
  VkResult result = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create logical device! Error code: " +
                             std::to_string(result));
  }
  std::cout << "Vulkan logical device created successfully." << std::endl;

  // Get queue handles (we only requested one queue from each family)
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
    // createInfo.pNext = &debugCreateInfo; // Only if logging instance creation
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
  // Destroy logical device first
  if (_device != VK_NULL_HANDLE)
  {
    vkDestroyDevice(_device, nullptr);
    std::cout << "Vulkan logical device destroyed." << std::endl;
  }

  // Destroy surface BEFORE instance
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