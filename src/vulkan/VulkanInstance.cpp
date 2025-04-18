#include "VulkanInstance.h"
#include "../platform/Window.h" // Include the concrete Window class definition

#include <cstring> // For strcmp
#include <iostream>
#include <set> 
#include <stdexcept>

// --- Constructor / Destructor ---

VulkanInstance::VulkanInstance(const Window& window) : _windowRef(window)
{
  createInstance();
  setupDebugMessenger();
  createSurface(); // Surface depends on instance and window
}

VulkanInstance::~VulkanInstance()
{
  // Destroy in reverse order of creation
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
}

// --- Public Methods --- (Accessors are inline in header)

// --- Private Methods --- 

void VulkanInstance::createInstance()
{
  if (enableValidationLayers && !checkValidationLayerSupport())
  {
    throw std::runtime_error("Validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Vulkan App"; // Generic name now
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

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{}; // Needed for pNext if logging instance creation
  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    populateDebugMessengerCreateInfo(debugCreateInfo);
    // To capture instance creation/destruction messages, chain the debug messenger info
    // createInfo.pNext = &debugCreateInfo; 
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

void VulkanInstance::setupDebugMessenger()
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

void VulkanInstance::createSurface()
{
  // Use the referenced window object to create the surface
  VkResult result = glfwCreateWindowSurface(_instance, _windowRef.getGLFWwindow(), nullptr, &_surface);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create window surface! Error code: " +
                             std::to_string(result));
  }
  std::cout << "Vulkan window surface created successfully." << std::endl;
}

// --- Helpers --- (Validation Layer Checks, Extension Gathering, Callback, etc.)

bool VulkanInstance::checkValidationLayerSupport()
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

std::vector<const char*> VulkanInstance::getRequiredExtensions()
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  if (glfwExtensions == nullptr)
  {
    throw std::runtime_error("GLFW required extensions unavailable.");
  }
  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  // MoltenVK portability requirements
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

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
  // Could filter severity here
  std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}

void VulkanInstance::populateDebugMessengerCreateInfo(
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


// --- Proxy Function Definitions ---
// These need to be defined once, placing them here is fine.

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