#include "VulkanSwapChain.h"
#include "VulkanDevice.h"     // For QueueFamilyIndices, VkDevice, VkPhysicalDevice
#include "../platform/Window.h" // For getting framebuffer size

#include <algorithm> // For std::clamp
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <iostream> // For logging

// --- Constructor / Destructor ---

VulkanSwapChain::VulkanSwapChain(const VulkanDevice& device, const Window& window, VkSurfaceKHR surface)
    : _deviceRef(device),
      _windowRef(window),
      _surface(surface),
      _logicalDevice(device.getDevice()) // Cache logical device handle
{
  createSwapChain();
  createImageViews();
}

VulkanSwapChain::~VulkanSwapChain()
{
  // Destroy image views first
  for (auto imageView : _swapChainImageViews)
  {
    if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(_logicalDevice, imageView, nullptr);
    }
  }
   std::cout << "Vulkan swap chain image views destroyed." << std::endl;

  // Then destroy the swap chain
  if (_swapChain != VK_NULL_HANDLE)
  {
    vkDestroySwapchainKHR(_logicalDevice, _swapChain, nullptr);
     std::cout << "Vulkan swap chain destroyed." << std::endl;
  }
  // Surface is destroyed by VulkanInstance
}

// --- Public Methods --- (Accessors are inline in header)

// --- Private Methods ---

void VulkanSwapChain::createSwapChain()
{
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_deviceRef.getPhysicalDevice());

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

  const QueueFamilyIndices& indices = _deviceRef.getQueueFamilyIndices();
  uint32_t queueFamilyIndicesValue[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndicesValue;
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

  VkResult result = vkCreateSwapchainKHR(_logicalDevice, &createInfo, nullptr, &_swapChain);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create swap chain! Error code: " + std::to_string(result));
  }
  std::cout << "Vulkan swap chain created successfully." << std::endl;

  // Get swap chain images
  vkGetSwapchainImagesKHR(_logicalDevice, _swapChain, &imageCount, nullptr);
  _swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(_logicalDevice, _swapChain, &imageCount, _swapChainImages.data());

  // Store format and extent
  _swapChainImageFormat = surfaceFormat.format;
  _swapChainExtent = extent;
}

void VulkanSwapChain::createImageViews()
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

    VkResult result = vkCreateImageView(_logicalDevice, &createInfo, nullptr, &_swapChainImageViews[i]);
    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create image view " + std::to_string(i) + 
                               "! Error code: " + std::to_string(result));
    }
  }
  std::cout << "Vulkan swap chain image views created successfully (" 
            << _swapChainImageViews.size() << ")." << std::endl;
}

// --- Helpers ---

SwapChainSupportDetails VulkanSwapChain::querySwapChainSupport(VkPhysicalDevice physicalDevice)
{
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, _surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, nullptr);
  if (formatCount != 0)
  {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, nullptr);
  if (presentModeCount != 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(
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

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(
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

VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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
    // Use window framebuffer size from the Window reference
    VkExtent2D actualExtent = _windowRef.getFramebufferExtent();

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    std::cout << "Swap Extent: Using window framebuffer size clamped (" 
              << actualExtent.width << "x" << actualExtent.height << ")" << std::endl;
    return actualExtent;
  }
} 