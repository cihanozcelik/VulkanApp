#include "Application.h"

// Include concrete definitions needed for construction
#include "../platform/Window.h"
#include "../vulkan/VulkanInstance.h"
#include "../vulkan/VulkanDevice.h"
#include "../vulkan/VulkanSwapChain.h"

#include <stdexcept> // For exception handling
#include <iostream>  // For logging

// Constants can be moved to a config header later
const uint32_t INITIAL_WIDTH = 800;
const uint32_t INITIAL_HEIGHT = 600;

Application::Application()
{
  // Constructor can be empty if init() does all the work
}

Application::~Application()
{
  // Destructor is automatically correct thanks to std::unique_ptr
  // Order of destruction is reverse order of declaration in the header
  // _vulkanSwapChain -> _vulkanDevice -> _vulkanInstance -> _window
  std::cout << "Application shutting down." << std::endl;
}

void Application::run()
{
  init();
  mainLoop();
  cleanup(); // Keep cleanup for any non-RAII resources if added later
}

void Application::init()
{
  // Order is crucial
  _window = std::make_unique<Window>(INITIAL_WIDTH, INITIAL_HEIGHT, "Vulkan App");
  _vulkanInstance = std::make_unique<VulkanInstance>(*_window); // Pass window ref
  _vulkanDevice = std::make_unique<VulkanDevice>(*_vulkanInstance); // Pass instance ref
  _vulkanSwapChain = std::make_unique<VulkanSwapChain>(*_vulkanDevice, *_window, _vulkanInstance->getSurface());

  std::cout << "--- Application Initialized Successfully ---" << std::endl;
}

void Application::mainLoop()
{
  while (!_window->shouldClose())
  {
    _window->pollEvents();
    // TODO: Add drawing logic here
    // - Acquire image from swap chain
    // - Record command buffer (draw triangle)
    // - Submit command buffer
    // - Present image to swap chain
  }
}

void Application::cleanup()
{
  // Explicit cleanup is mostly handled by unique_ptr destructors (RAII).
  // vkDeviceWaitIdle might be needed here before destruction starts
  // if rendering were happening asynchronously.
  if (_vulkanDevice && _vulkanDevice->getDevice() != VK_NULL_HANDLE) {
      vkDeviceWaitIdle(_vulkanDevice->getDevice());
      std::cout << "vkDeviceWaitIdle completed." << std::endl;
  }

  // unique_ptrs handle the rest in their destructors
  // _vulkanSwapChain.reset(); // Explicitly destroy (optional)
  // _vulkanDevice.reset();
  // _vulkanInstance.reset();
  // _window.reset();
} 