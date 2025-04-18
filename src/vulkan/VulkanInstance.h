#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

// Forward declaration
class Window;

// List of validation layers to enable
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// Enable validation layers only in debug builds
#ifdef NDEBUG // NDEBUG is defined by CMake for Release builds
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class VulkanInstance
{
public:
  VulkanInstance(const Window& window);
  ~VulkanInstance();

  // Delete copy/move semantics
  VulkanInstance(const VulkanInstance&) = delete;
  VulkanInstance& operator=(const VulkanInstance&) = delete;
  VulkanInstance(VulkanInstance&&) = delete;
  VulkanInstance& operator=(VulkanInstance&&) = delete;

  // Accessors
  VkInstance getInstance() const { return _instance; }
  VkSurfaceKHR getSurface() const { return _surface; }

private:
  VkInstance _instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
  VkSurfaceKHR _surface = VK_NULL_HANDLE; // Surface is tightly coupled to instance
  const Window& _windowRef; // Reference to window for surface creation

  void createInstance();
  void setupDebugMessenger();
  void createSurface();

  // Helpers
  bool checkValidationLayerSupport();
  std::vector<const char*> getRequiredExtensions();
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

  // Static Debug Callback
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData);
};

// Helper function prototypes (Definition needed in cpp)
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator); 