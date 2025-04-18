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
  
  // Render Pass depends on swap chain image format
  createRenderPass(); 

  std::cout << "--- Application Initialized Successfully ---" << std::endl;
}

void Application::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    // Format should match the swap chain images
    colorAttachment.format = _vulkanSwapChain->getImageFormat(); 
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // No multisampling yet
    // What to do with data before/after rendering
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear framebuffer before drawing
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store result so it can be presented
    // Stencil data - not used
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // Image layout transition
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Don't care about previous layout
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Layout suitable for presentation

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // Index of the attachment in the pAttachments array
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout during subpass

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    // No input, resolve, depth/stencil, or preserve attachments for now

    // Subpass dependency (ensures render pass waits for image availability)
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Implicit subpass before render pass
    dependency.dstSubpass = 0; // Our first (and only) subpass
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(_vulkanDevice->getDevice(), &renderPassInfo, nullptr, &_renderPass);
    if (result != VK_SUCCESS) { 
        throw std::runtime_error("Failed to create render pass! Error code: " + std::to_string(result));
    }
    std::cout << "Vulkan render pass created successfully." << std::endl;
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

  // Destroy render pass BEFORE device
  if (_renderPass != VK_NULL_HANDLE && _vulkanDevice && _vulkanDevice->getDevice() != VK_NULL_HANDLE)
  {
      vkDestroyRenderPass(_vulkanDevice->getDevice(), _renderPass, nullptr);
      std::cout << "Vulkan render pass destroyed." << std::endl;
      _renderPass = VK_NULL_HANDLE; // Good practice to null handles after destruction
  }

  // unique_ptrs handle the rest in their destructors
  // _vulkanSwapChain.reset(); // Explicitly destroy (optional)
  // _vulkanDevice.reset();
  // _vulkanInstance.reset();
  // _window.reset();
} 