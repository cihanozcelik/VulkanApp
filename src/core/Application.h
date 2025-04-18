#pragma once

#include <memory> // For std::unique_ptr

// Needed for VkRenderPass handle
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Forward declarations to avoid including full headers here
class Window;
class VulkanInstance;
class VulkanDevice;
class VulkanSwapChain;

class Application
{
public:
  Application();
  ~Application(); // Default destructor is likely fine with unique_ptr

  void run();

private:
  void init();
  void mainLoop();
  void cleanup(); // Cleanup might be minimal due to RAII

  void createRenderPass(); // Add render pass creation method

  // Order matters for initialization and destruction!
  std::unique_ptr<Window> _window;
  std::unique_ptr<VulkanInstance> _vulkanInstance;
  std::unique_ptr<VulkanDevice> _vulkanDevice;
  std::unique_ptr<VulkanSwapChain> _vulkanSwapChain;
  
  // Vulkan objects managed here (or potentially in a dedicated Renderer class later)
  VkRenderPass _renderPass = VK_NULL_HANDLE;

  // Add other components like Renderer, Pipeline Cache etc. later
}; 