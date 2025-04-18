#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>

// Forward declarations
class VulkanInstance;

// Required device extensions
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    // Portability subset added dynamically if needed
};

// Helper struct - could potentially move to a common header
struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() const
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

class VulkanDevice
{
public:
  VulkanDevice(const VulkanInstance& instance);
  ~VulkanDevice();

  // Delete copy/move semantics
  VulkanDevice(const VulkanDevice&) = delete;
  VulkanDevice& operator=(const VulkanDevice&) = delete;
  VulkanDevice(VulkanDevice&&) = delete;
  VulkanDevice& operator=(VulkanDevice&&) = delete;

  // Accessors
  VkPhysicalDevice getPhysicalDevice() const { return _physicalDevice; }
  VkDevice getDevice() const { return _device; }
  VkQueue getGraphicsQueue() const { return _graphicsQueue; }
  VkQueue getPresentQueue() const { return _presentQueue; }
  const QueueFamilyIndices& getQueueFamilyIndices() const { return _indices; }

private:
  VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
  VkDevice _device = VK_NULL_HANDLE;
  VkQueue _graphicsQueue = VK_NULL_HANDLE;
  VkQueue _presentQueue = VK_NULL_HANDLE;

  const VulkanInstance& _instanceRef; // Keep reference to instance
  VkSurfaceKHR _surface; // Copy surface handle from instance
  QueueFamilyIndices _indices;

  void pickPhysicalDevice();
  void createLogicalDevice();

  // Helpers
  bool isDeviceSuitable(VkPhysicalDevice device);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
}; 