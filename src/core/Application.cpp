#include "Application.h"

// Include concrete definitions needed for construction
#include "../platform/Window.h"
#include "../vulkan/VulkanInstance.h"
#include "../vulkan/VulkanDevice.h"
#include "../vulkan/VulkanSwapChain.h"
#include "../rendering/Renderer.h"

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
  // _renderer -> _vulkanSwapChain -> _vulkanDevice -> _vulkanInstance -> _window
  std::cout << "Application shutting down." << std::endl;
}

void Application::Run()
{
  try {
    InitWindow();
    InitVulkan();
    MainLoop();
  } catch (const std::exception& e) {
    std::cerr << "Unhandled Exception: " << e.what() << std::endl;
    // Consider exiting more gracefully or providing user feedback
  }
  Cleanup(); // Cleanup any remaining non-RAII resources if necessary
}

void Application::InitWindow()
{
  _window = std::make_unique<Window>(INITIAL_WIDTH, INITIAL_HEIGHT, "Vulkan App");
  std::cout << "Window initialized." << std::endl;
}

void Application::InitVulkan()
{
  // Initialize core Vulkan components
  _vulkanInstance = std::make_unique<VulkanInstance>(*_window); 
  _vulkanDevice = std::make_unique<VulkanDevice>(*_vulkanInstance);
  _vulkanSwapChain = std::make_unique<VulkanSwapChain>(*_vulkanDevice, *_window, _vulkanInstance->getSurface());
  
  // Explicitly get lvalue references
  VulkanDevice& deviceRef = *_vulkanDevice;
  VulkanSwapChain& swapChainRef = *_vulkanSwapChain;

  // Create and initialize the Renderer using the explicit references
  _renderer.reset(new VulkanApp::Rendering::Renderer(deviceRef, swapChainRef));
  _renderer->Init(); // Call the renderer's initialization

  std::cout << "--- Vulkan Initialized Successfully ---" << std::endl;
}

// --- Main Loop ---
void Application::MainLoop()
{
  std::cout << "Starting main loop..." << std::endl;
  while (!_window->shouldClose())
  {
    glfwPollEvents();
    _renderer->DrawFrame(); // Delegate drawing to the renderer
  }
  std::cout << "Main loop finished." << std::endl;

  // Wait for the device to be idle before cleanup, especially before Application destructor runs
  // This prevents destroying resources while they might still be in use by the GPU.
  vkDeviceWaitIdle(_vulkanDevice->getDevice()); 
  std::cout << "GPU finished processing." << std::endl;
}

// --- Cleanup ---
void Application::Cleanup()
{
  // Most cleanup is handled by unique_ptr destructors in the correct order.
  // _renderer will call its Cleanup() via its destructor.
  // If any non-RAII cleanup specific to Application itself is needed, add it here.

  std::cout << "Application cleanup finished." << std::endl;
  // GLFW termination happens in Window destructor
} 