#include "VulkanDevice.h"
#include "VulkanInstance.h" // Need access to VkInstance and VkSurfaceKHR

#include <cstring>   // For strcmp
#include <iostream>
#include <map>       // Optional for scoring
#include <set>
#include <stdexcept>
#include <vector>

// --- Constructor / Destructor ---

VulkanDevice::VulkanDevice(const VulkanInstance& instance)
    : _instanceRef(instance), _surface(instance.getSurface()) // Get dependencies
{
  pickPhysicalDevice();
  createLogicalDevice();
}

VulkanDevice::~VulkanDevice()
{
  if (_device != VK_NULL_HANDLE)
  {
    vkDestroyDevice(_device, nullptr);
    std::cout << "Vulkan logical device destroyed." << std::endl;
  }
  // Physical device is implicitly destroyed with instance
}

// --- Public Methods --- (Accessors are inline in header)

// --- Private Methods ---

void VulkanDevice::pickPhysicalDevice()
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(_instanceRef.getInstance(), &deviceCount, nullptr);

  if (deviceCount == 0)
  {
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(_instanceRef.getInstance(), &deviceCount, devices.data());

  std::cout << "Available physical devices (" << deviceCount << ") :" << std::endl;

  for (const auto& device : devices)
  {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    std::cout << "\t" << properties.deviceName;

    if (isDeviceSuitable(device))
    {
      _physicalDevice = device;
      _indices = findQueueFamilies(_physicalDevice); // Store indices for selected device
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

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device)
{
  QueueFamilyIndices indices = findQueueFamilies(device);
  bool extensionsSupported = checkDeviceExtensionSupport(device);

  // Check swap chain support (basic check for now)
  bool swapChainAdequate = false;
  if (extensionsSupported)
  {
    // Query details - implementation omitted here, belongs in SwapChain class logic
    // For now, just assume if extensions are supported, swap chain is okay
    // SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    // swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
     swapChainAdequate = true; // Simplified check for device suitability phase
  }

  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device)
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

bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device)
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


void VulkanDevice::createLogicalDevice()
{
  // _indices should be populated by pickPhysicalDevice
  if (!_indices.isComplete()) {
      throw std::runtime_error("Cannot create logical device without complete queue families!");
  }

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {
      _indices.graphicsFamily.value(), 
      _indices.presentFamily.value()
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

  VkPhysicalDeviceFeatures deviceFeatures{}; // Request no special features for now

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pEnabledFeatures = &deviceFeatures;

  // Enable required device extensions, including portability if needed
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
       std::cout << "Enabling VK_KHR_portability_subset for logical device." << std::endl;
  }

  createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDevExtensionsVec.size());
  createInfo.ppEnabledExtensionNames = requiredDevExtensionsVec.data();

  // Enable validation layers (consistent with instance)
  if (enableValidationLayers)
  {
    // Note: enabledLayerCount and ppEnabledLayerNames are deprecated in VkDeviceCreateInfo
    // They are ignored by modern drivers, but older implementations might require them.
    // Best practice is to set them anyway for compatibility, matching instance layers.
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

  // Get the queue handles
  vkGetDeviceQueue(_device, _indices.graphicsFamily.value(), 0, &_graphicsQueue);
  vkGetDeviceQueue(_device, _indices.presentFamily.value(), 0, &_presentQueue);
  std::cout << "Graphics and present queue handles obtained." << std::endl;
} 