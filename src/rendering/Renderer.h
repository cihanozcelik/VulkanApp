#pragma once

#include <vector>
#include <string>
#include <memory> // For unique_ptr forward declaration if needed

#include <vulkan/vulkan.h>

// Forward declarations are not needed here if full headers are included in Renderer.cpp

namespace VulkanApp::Rendering {

// Forward declare dependent types used as references/pointers if needed
// class VulkanDevice; // Prefer including full header in .cpp
// class VulkanSwapChain;

class Renderer {
public:
    // Use types directly without global scope resolution
    Renderer(VulkanDevice& device, VulkanSwapChain& swapChain);
    ~Renderer();

    // Prevent copying and moving for simplicity for now
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void Init(); // Further initialization requiring more setup
    void DrawFrame();

private:
    // Initialization steps (called by Init or constructor)
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects();

    // Drawing helpers
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    // Shader helpers
    static std::vector<char> ReadFile(const std::string& filename);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    // Cleanup
    void CleanupSwapChainResources(); // For recreation
    void Cleanup();                 // Full cleanup

    // --- Member Variables ---
    // Use types directly
    VulkanDevice& _device;
    VulkanSwapChain& _swapChain;

    const int MAX_FRAMES_IN_FLIGHT = 2;

    // Vulkan rendering objects
    VkRenderPass _renderPass = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
    VkPipeline _graphicsPipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> _swapChainFramebuffers;
    VkCommandPool _commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> _commandBuffers;

    // Synchronization objects (per frame in flight)
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    uint32_t _currentFrame = 0;
};

} // namespace VulkanApp::Rendering 