#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

// Forward declarations
class VulkanDevice;
class Window;
struct QueueFamilyIndices; // Use the struct defined elsewhere (or move to common header)

// Struct to hold swap chain support details
struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapChain 
{
public:
  VulkanSwapChain(const VulkanDevice& device, const Window& window, VkSurfaceKHR surface);
  ~VulkanSwapChain();

  // Delete copy/move semantics
  VulkanSwapChain(const VulkanSwapChain&) = delete;
  VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;
  VulkanSwapChain(VulkanSwapChain&&) = delete;
  VulkanSwapChain& operator=(VulkanSwapChain&&) = delete;

  // Accessors
  VkSwapchainKHR getSwapChain() const { return _swapChain; }
  VkFormat getImageFormat() const { return _swapChainImageFormat; }
  VkExtent2D getExtent() const { return _swapChainExtent; }
  const std::vector<VkImageView>& getImageViews() const { return _swapChainImageViews; }

private:
  VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
  std::vector<VkImage> _swapChainImages;
  VkFormat _swapChainImageFormat;
  VkExtent2D _swapChainExtent;
  std::vector<VkImageView> _swapChainImageViews;

  const VulkanDevice& _deviceRef; // Reference to logical device
  const Window& _windowRef;       // Reference to window for extent
  VkSurfaceKHR _surface;          // Copy surface handle
  VkDevice _logicalDevice;       // Copy logical device handle

  void createSwapChain();
  void createImageViews();

  // Helpers
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
}; 