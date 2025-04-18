#pragma once

#include <memory> // For std::unique_ptr
// Removed <vector> and vulkan includes if not directly needed by Application

// Forward declarations
class Window;
class VulkanInstance;
class VulkanDevice;
class VulkanSwapChain;

// Forward declare Renderer instead of including the full header
namespace VulkanApp::Rendering { class Renderer; }

class Application
{
public:
    Application();
    ~Application();

    void Run(); // Renamed for consistency maybe?

private:
    void InitWindow();
    void InitVulkan(); // Will initialize core Vulkan + Renderer
    void MainLoop();
    void Cleanup();

    // Order matters for initialization and destruction!
    std::unique_ptr<Window> _window;
    std::unique_ptr<VulkanInstance> _vulkanInstance;
    std::unique_ptr<VulkanDevice> _vulkanDevice;
    std::unique_ptr<VulkanSwapChain> _vulkanSwapChain;
    std::unique_ptr<VulkanApp::Rendering::Renderer> _renderer; // Added Renderer

    // No longer owns render pass, pipeline, etc.
}; 