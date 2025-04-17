#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication
{
public:
  void run()
  {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  GLFWwindow* _window = nullptr; // Initialize to nullptr
  VkInstance _instance = VK_NULL_HANDLE; // Initialize to VK_NULL_HANDLE

  void initWindow()
  {
    if (!glfwInit())
    {
      throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't create OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disable resizing for simplicity

    _window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Window", nullptr, nullptr);
    if (!_window)
    {
      glfwTerminate(); // Terminate GLFW if window creation fails
      throw std::runtime_error("Failed to create GLFW window");
    }
    std::cout << "GLFW window created successfully." << std::endl;
  }

  void initVulkan()
  {
    createInstance();
  }

  void createInstance()
  {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0; // Use a reasonable minimum API version

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Get required extensions from GLFW
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    if (glfwExtensions == nullptr)
    {
      throw std::runtime_error("GLFW required extensions unavailable.");
    }

    std::vector<const char*> requiredExtensions(glfwExtensions,
                                            glfwExtensions + glfwExtensionCount);

    // Add extensions required for MoltenVK on macOS
    requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#ifdef VK_KHR_get_physical_device_properties2
    requiredExtensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#else
#error \
    "VK_KHR_get_physical_device_properties2 not defined, required for MoltenVK portability"
#endif

    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // Enable validation layers (we'll add this later)
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    // Enable MoltenVK portability flag
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);

    if (result != VK_SUCCESS)
    {
      // Provide a more informative error message
      throw std::runtime_error("Failed to create Vulkan instance! Error code: " +
                               std::to_string(result));
    }
    std::cout << "Vulkan instance created successfully." << std::endl;
  }

  void mainLoop()
  {
    while (!glfwWindowShouldClose(_window))
    {
      glfwPollEvents();
    }
  }

  void cleanup()
  {
    // Destroy Vulkan instance before terminating GLFW
    if (_instance != VK_NULL_HANDLE)
    {
      vkDestroyInstance(_instance, nullptr);
      std::cout << "Vulkan instance destroyed." << std::endl;
    }

    if (_window)
    {
      glfwDestroyWindow(_window);
      std::cout << "GLFW window destroyed." << std::endl;
    }

    glfwTerminate();
    std::cout << "GLFW terminated." << std::endl;
  }
};

int main()
{
  HelloTriangleApplication app;

  try
  {
    app.run();
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl; // Add ERROR prefix
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
} 