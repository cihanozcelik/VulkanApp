// Include dependent class definitions *before* the namespace
#include "../vulkan/VulkanDevice.h" 
#include "../vulkan/VulkanSwapChain.h"

#include "Renderer.h" // Include own header after dependencies

#include <stdexcept> 
#include <iostream>
#include <fstream> 
#include <array> // For clear values

namespace VulkanApp::Rendering {

// Helper: Implementation of readFile (static)
std::vector<char> Renderer::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

// Helper: Implementation of CreateShaderModule
VkShaderModule Renderer::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(_device.getDevice(), &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Renderer::CreateShaderModule failed! Error: " + std::to_string(result));
    }
    return shaderModule;
}

// Constructor: Use types directly
Renderer::Renderer(VulkanDevice& device, VulkanSwapChain& swapChain)
    : _device(device), _swapChain(swapChain)
{
    std::cout << "Renderer created." << std::endl;
}

// Destructor: Call full cleanup
Renderer::~Renderer()
{
    Cleanup();
    std::cout << "Renderer destroyed." << std::endl;
}

// Init: Call all creation helpers in order
void Renderer::Init()
{
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
    std::cout << "Renderer initialized successfully." << std::endl;
}

// --- Vulkan Object Creation Methods ---

void Renderer::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _swapChain.getImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; 
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Ready for presentation

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // Index into the pAttachments array
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Subpass dependency to handle layout transitions
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

    VkResult result = vkCreateRenderPass(_device.getDevice(), &renderPassInfo, nullptr, &_renderPass);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass! Error: " + std::to_string(result));
    }
    std::cout << "Vulkan render pass created successfully." << std::endl;
}

void Renderer::CreateGraphicsPipeline()
{
    auto vertShaderCode = ReadFile("shaders/vert.spv");
    auto fragShaderCode = ReadFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);
    std::cout << "Shader modules created successfully." << std::endl;

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_swapChain.getExtent().width;
    viewport.height = (float)_swapChain.getExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapChain.getExtent();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VkResult layoutResult = vkCreatePipelineLayout(_device.getDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout);
    if (layoutResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout! Error: " + std::to_string(layoutResult));
    }
    std::cout << "Vulkan pipeline layout created successfully." << std::endl;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkResult pipelineResult = vkCreateGraphicsPipelines(_device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline);
    if (pipelineResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline! Error: " + std::to_string(pipelineResult));
    }
    std::cout << "Vulkan graphics pipeline created successfully." << std::endl;

    vkDestroyShaderModule(_device.getDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(_device.getDevice(), vertShaderModule, nullptr);
    std::cout << "Shader modules destroyed." << std::endl;
}

void Renderer::CreateFramebuffers()
{
    _swapChainFramebuffers.resize(_swapChain.getImageViews().size());

    for (size_t i = 0; i < _swapChain.getImageViews().size(); i++) {
        VkImageView attachments[] = {
            _swapChain.getImageViews()[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = _swapChain.getExtent().width;
        framebufferInfo.height = _swapChain.getExtent().height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(_device.getDevice(), &framebufferInfo, nullptr, &_swapChainFramebuffers[i]);
        if (result != VK_SUCCESS) {
             throw std::runtime_error("Failed to create framebuffer! Error: " + std::to_string(result));
        }
    }
     std::cout << "Vulkan swap chain framebuffers created successfully (" << _swapChainFramebuffers.size() << ")." << std::endl;
}

void Renderer::CreateCommandPool()
{
    // Use the type defined in VulkanDevice.h (now included above)
    QueueFamilyIndices queueFamilyIndices = _device.getQueueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; 
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkResult result = vkCreateCommandPool(_device.getDevice(), &poolInfo, nullptr, &_commandPool);
     if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool! Error: " + std::to_string(result));
    }
    std::cout << "Vulkan command pool created successfully." << std::endl;
}

void Renderer::CreateCommandBuffers()
{
    _commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    // Primary buffers can be submitted to a queue, secondary called from primary
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

    VkResult result = vkAllocateCommandBuffers(_device.getDevice(), &allocInfo, _commandBuffers.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers! Error: " + std::to_string(result));
    }
    std::cout << "Vulkan command buffers allocated successfully (" << _commandBuffers.size() << ")." << std::endl;
}

void Renderer::CreateSyncObjects()
{
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled so first frame doesn't wait

    VkResult result;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        result = vkCreateSemaphore(_device.getDevice(), &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]);
        if (result != VK_SUCCESS) throw std::runtime_error("Failed to create imageAvailable semaphore!" + std::to_string(result));

        result = vkCreateSemaphore(_device.getDevice(), &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]);
        if (result != VK_SUCCESS) throw std::runtime_error("Failed to create renderFinished semaphore!" + std::to_string(result));

        result = vkCreateFence(_device.getDevice(), &fenceInfo, nullptr, &_inFlightFences[i]);
        if (result != VK_SUCCESS) throw std::runtime_error("Failed to create inFlight fence!" + std::to_string(result));
    }
    std::cout << "Vulkan synchronization objects created successfully." << std::endl;
}

// --- Drawing ---

void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // beginInfo.flags = 0; // Optional
    // beginInfo.pInheritanceInfo = nullptr; // Optional

    VkResult beginResult = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (beginResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer! Error: " + std::to_string(beginResult));
    }

    // --- Start Render Pass ---
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _swapChainFramebuffers[imageIndex]; // Use framebuffer for the acquired image
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _swapChain.getExtent();

    // Clear color (set to dark grey)
    VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // --- Bind Pipeline & Draw ---
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);
    
    // Draw the hardcoded triangle (3 vertices, 1 instance, starting at vertex 0, instance 0)
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    // --- End Render Pass ---
    vkCmdEndRenderPass(commandBuffer);

    // --- End Recording ---
    VkResult endResult = vkEndCommandBuffer(commandBuffer);
    if (endResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer! Error: " + std::to_string(endResult));
    }
}

void Renderer::DrawFrame()
{
    // --- Wait for the previous frame to finish ---
    vkWaitForFences(_device.getDevice(), 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(_device.getDevice(), 1, &_inFlightFences[_currentFrame]); // Reset fence for the current frame

    // --- Acquire an image from the swap chain ---
    uint32_t imageIndex;
    VkResult acquireResult = vkAcquireNextImageKHR(_device.getDevice(), _swapChain.getSwapChain(), UINT64_MAX, 
                                                 _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        // Swap chain is incompatible (e.g., window resized). Need to recreate.
        // TODO: Implement swap chain recreation
        std::cerr << "Swap chain out of date. Recreation needed." << std::endl;
        return;
    } else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image! Error: " + std::to_string(acquireResult));
    }

    // --- Record command buffer ---
    vkResetCommandBuffer(_commandBuffers[_currentFrame], 0); // Reset buffer before recording
    RecordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);

    // --- Submit the command buffer ---
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];

    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult submitResult = vkQueueSubmit(_device.getGraphicsQueue(), 1, &submitInfo, _inFlightFences[_currentFrame]);
    if (submitResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer! Error: " + std::to_string(submitResult));
    }

    // --- Present the swap chain image ---
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores; // Wait on render finished

    VkSwapchainKHR swapChains[] = {_swapChain.getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    // presentInfo.pResults = nullptr; // Optional: check results for multiple swapchains

    VkResult presentResult = vkQueuePresentKHR(_device.getPresentQueue(), &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        // Swap chain incompatible again (e.g., resize between acquire and present)
        // TODO: Implement swap chain recreation
        std::cerr << "Swap chain out of date or suboptimal during present. Recreation needed." << std::endl;
    } else if (presentResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image! Error: " + std::to_string(presentResult));
    }

    // Advance to the next frame index
    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// --- Cleanup ---

// Cleanup resources that depend on the swap chain (for recreation)
void Renderer::CleanupSwapChainResources()
{
    for (auto framebuffer : _swapChainFramebuffers) {
        vkDestroyFramebuffer(_device.getDevice(), framebuffer, nullptr);
    }
    _swapChainFramebuffers.clear();

    vkDestroyPipeline(_device.getDevice(), _graphicsPipeline, nullptr);
    _graphicsPipeline = VK_NULL_HANDLE;
    vkDestroyPipelineLayout(_device.getDevice(), _pipelineLayout, nullptr);
    _pipelineLayout = VK_NULL_HANDLE;
    vkDestroyRenderPass(_device.getDevice(), _renderPass, nullptr);
    _renderPass = VK_NULL_HANDLE;
    
    // Command buffers don't need explicit swapchain cleanup if pool is reused
    // Sync objects also don't usually depend directly on swapchain details
    std::cout << "Renderer swap chain resources cleaned up." << std::endl;
}

// Full cleanup in reverse order of creation
void Renderer::Cleanup()
{
    // Wait for device to be idle before destroying anything
    vkDeviceWaitIdle(_device.getDevice());

    CleanupSwapChainResources(); // Clean swap chain dependent resources first

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(_device.getDevice(), _renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(_device.getDevice(), _imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(_device.getDevice(), _inFlightFences[i], nullptr);
    }
    _renderFinishedSemaphores.clear();
    _imageAvailableSemaphores.clear();
    _inFlightFences.clear();

    vkDestroyCommandPool(_device.getDevice(), _commandPool, nullptr);
    _commandPool = VK_NULL_HANDLE;
    // Command buffers are implicitly destroyed with the pool
    _commandBuffers.clear();

     std::cout << "Renderer resources fully cleaned up." << std::endl;
}

} // namespace VulkanApp::Rendering 