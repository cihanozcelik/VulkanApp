#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector> // Include vector for storing layer names

// Forward declare GLFWwindow if needed, though including glfw3.h is usually fine

// Define constants or include them from a separate header if preferred
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// List of validation layers to enable
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation" // Standard validation layer
};

// Enable validation layers only in debug builds
#ifdef NDEBUG // NDEBUG is defined by CMake for Release builds
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

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

  void initWindow();
  void initVulkan();
  void createInstance();
  void mainLoop();
  void cleanup();

  // Vulkan Setup Helpers
  void setupDebugMessenger();

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