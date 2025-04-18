#include "Application.h"

// Include concrete definitions needed for construction
#include "../platform/Window.h"
#include "../vulkan/VulkanInstance.h"
#include "../vulkan/VulkanDevice.h"
#include "../vulkan/VulkanSwapChain.h"

#include <stdexcept> // For exception handling
#include <iostream>  // For logging
#include <fstream> // For readFile
#include <vector>

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
  createGraphicsPipeline(); // Create pipeline after render pass

  std::cout << "--- Application Initialized Successfully ---" << std::endl;
}

// --- Shader Loading Helper ---
std::vector<char> Application::readFile(const std::string& filename)
{
  // Open file at the end to get size easily
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  // Seek back to beginning and read
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

VkShaderModule Application::createShaderModule(const std::vector<char>& code)
{
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  // Vulkan expects uint32_t*, careful with alignment if vector isn't char
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule;
  VkResult result = vkCreateShaderModule(_vulkanDevice->getDevice(), &createInfo, nullptr, &shaderModule);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create shader module! Error: " + std::to_string(result));
  }
  return shaderModule;
}

// --- Graphics Pipeline Creation ---
void Application::createGraphicsPipeline()
{
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    std::cout << "Shader modules created successfully." << std::endl;

    // --- Shader Stage Creation ---
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // Entry point function name in shader

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // --- Fixed Function State Setup ---

    // Vertex Input (no vertex buffers for now, vertices are hardcoded in shader)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    // Input Assembly (draw triangles)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and Scissor (cover the whole swap chain extent)
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_vulkanSwapChain->getExtent().width;
    viewport.height = (float)_vulkanSwapChain->getExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _vulkanSwapChain->getExtent();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer (fill polygons, no culling)
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // Cull back faces
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // Standard for Vulkan
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling (disabled for now)
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth/Stencil (disabled for now)
    // VkPipelineDepthStencilStateCreateInfo depthStencil{}; 

    // Color Blending (standard alpha blending, disabled for opaque triangle)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE; // Opaque
    // Optional: Set blend factors if blendEnable = VK_TRUE

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE; // Optional logic op (e.g., bitwise)
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    // Optional: Blend constants
    // colorBlending.blendConstants[0] = 0.0f; ... 

    // Dynamic State (none for now)
    // std::vector<VkDynamicState> dynamicStates = { ... };
    // VkPipelineDynamicStateCreateInfo dynamicState{};

    // --- Pipeline Layout --- (No descriptors needed yet)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    VkResult layoutResult = vkCreatePipelineLayout(_vulkanDevice->getDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout);
    if (layoutResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout! Error: " + std::to_string(layoutResult));
    }
     std::cout << "Vulkan pipeline layout created successfully." << std::endl;

    // --- Graphics Pipeline Creation ---
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2; // Vertex + Fragment
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional, no depth test
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0; // Index of the subpass where this pipeline will be used
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional: for deriving pipelines
    pipelineInfo.basePipelineIndex = -1; // Optional

    VkResult pipelineResult = vkCreateGraphicsPipelines(_vulkanDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline);
    if (pipelineResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline! Error: " + std::to_string(pipelineResult));
    }
     std::cout << "Vulkan graphics pipeline created successfully." << std::endl;

    // --- Cleanup Shader Modules --- (No longer needed after pipeline creation)
    vkDestroyShaderModule(_vulkanDevice->getDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(_vulkanDevice->getDevice(), vertShaderModule, nullptr);
    std::cout << "Shader modules destroyed." << std::endl;
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

  // Destroy pipeline and layout before render pass
  if (_graphicsPipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(_vulkanDevice->getDevice(), _graphicsPipeline, nullptr);
      std::cout << "Vulkan graphics pipeline destroyed." << std::endl;
      _graphicsPipeline = VK_NULL_HANDLE;
  }
  if (_pipelineLayout != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(_vulkanDevice->getDevice(), _pipelineLayout, nullptr);
      std::cout << "Vulkan pipeline layout destroyed." << std::endl;
       _pipelineLayout = VK_NULL_HANDLE;
  }

  if (_renderPass != VK_NULL_HANDLE) // Check added previously, nulling handle here
  {
      vkDestroyRenderPass(_vulkanDevice->getDevice(), _renderPass, nullptr);
      std::cout << "Vulkan render pass destroyed." << std::endl;
      _renderPass = VK_NULL_HANDLE; 
  }

  // unique_ptrs handle the rest in their destructors
  // _vulkanSwapChain.reset(); // Explicitly destroy (optional)
  // _vulkanDevice.reset();
  // _vulkanInstance.reset();
  // _window.reset();
} 