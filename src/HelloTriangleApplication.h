#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional> // For QueueFamilyIndices
#include <vector>

// Forward declare GLFWwindow if needed, though including glfw3.h is usually fine

// Define constants or include them from a separate header if preferred
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// List of validation layers to enable
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation" // Standard validation layer
};

// List required device extensions
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    // VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME // Often needed for MoltenVK, checked dynamically
};

// Enable validation layers only in debug builds
#ifdef NDEBUG // NDEBUG is defined by CMake for Release builds
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Helper struct to hold queue family indices
struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() const
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

class HelloTriangleApplication
{
public:
  void run();

// Make constructor/destructor explicit if needed later
// HelloTriangleApplication();
// ~HelloTriangleApplication();

private:
  GLFWwindow* _window = nullptr;
  VkInstance _instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
  VkSurfaceKHR _surface = VK_NULL_HANDLE;
  VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE; // Implicitly destroyed with VkInstance
  VkDevice _device = VK_NULL_HANDLE;
  VkQueue _graphicsQueue = VK_NULL_HANDLE;
  VkQueue _presentQueue = VK_NULL_HANDLE;

  void initWindow();
  void initVulkan();
  void createInstance();
  void mainLoop();
  void cleanup();

  // Vulkan Setup Helpers
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();

  // Physical Device Helpers
  bool isDeviceSuitable(VkPhysicalDevice device);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);

  // Validation Layer Helpers
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