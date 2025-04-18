# Vulkan Learning Project & Unity-Style Engine Base

This project serves as a hands-on learning exercise for the Vulkan graphics API, with the secondary goal of building a foundational C++ structure inspired by familiar engine concepts like those found in Unity.

## What is Vulkan?

Vulkan is a modern, explicit, low-overhead, cross-platform graphics and compute API. Developed by the Khronos Group (the consortium behind OpenGL, OpenCL, WebGL, and other standards), Vulkan represents a significant evolution from older APIs like OpenGL. Its core design philosophy is centered around providing direct, explicit control over modern GPU hardware.

**Key Characteristics:**

*   **Explicit Control:** Unlike OpenGL where the driver often manages state and performs background work implicitly, Vulkan requires the developer to explicitly manage almost everything: memory allocation, synchronization between the CPU and GPU, command buffer recording and submission, pipeline state objects, and resource transitions. This reduces hidden driver overhead.
*   **Low Overhead:** Designed from the ground up to minimize CPU usage during rendering compared to older APIs. This allows applications to better utilize multi-core CPUs for preparing rendering commands.
*   **Layered Design:** Vulkan has a core API focused purely on interacting with the GPU hardware. Essential features like validation, debugging tools, and platform-specific window system integration are handled through optional layers and extensions.
*   **Pipeline State Objects (PSOs):** Most rendering state (shaders, vertex formats, blending, rasterization settings, etc.) is pre-compiled into immutable PSO objects. This allows drivers to optimize for specific state combinations upfront, reducing state change overhead during rendering.
*   **Cross-Platform:** Designed as a single API specification intended to run on diverse hardware (desktop GPUs, mobile GPUs, integrated graphics) and operating systems (Windows, Linux, Android, Fuchsia, and via translation layers like MoltenVK on macOS/iOS).

**Adoption and Usage:**

Vulkan has seen significant adoption across the industry:

*   **Game Engines:** Major engines like Unreal Engine, Unity (via SRPs), Godot Engine, and id Tech have Vulkan backends.
*   **Major Game Titles:** Numerous AAA and indie games leverage Vulkan for improved performance and control, including titles like Doom Eternal, Red Dead Redemption 2 (PC), Baldur's Gate 3, No Man's Sky, and many mobile games.
*   **Emulators:** Emulators for consoles like Nintendo Switch (Yuzu, Ryujinx) and PS3 (RPCS3) often use Vulkan for efficient GPU emulation.
*   **Mobile:** It's the primary high-performance graphics API on Android.
*   **Creative Tools & CAD:** While adoption is slower here compared to games, interest is growing for high-performance visualization and GPU compute tasks.

By providing closer access to the hardware, Vulkan enables developers to push performance boundaries and implement advanced rendering techniques more effectively, though it comes with a significantly steeper learning curve compared to APIs like OpenGL or DirectX 11.

## Project Goals

1.  **Learn Vulkan:** Gain a deep understanding of the Vulkan API, its explicit nature, and best practices for modern graphics programming.
2.  **Develop a Render Pipeline:** Implement the core components of a Vulkan rendering pipeline from scratch, including device setup, swap chain management, command buffers, synchronization, render passes, and shader loading.
3.  **Build a Unity-Inspired Structure:** Explore creating C++ abstractions that mirror Unity's architecture, potentially including concepts such as:
    *   Scenes
    *   GameObjects
    *   Components (e.g., Transform, MeshRenderer)
    *   Meshes
    *   Materials / Shaders
    *   ...and related engine systems.
4.  **Cross-Platform Architecture:** Design the core C++ structure with cross-platform compatibility in mind for potential future ports to Windows or Linux. (**Note:** The current implementation is macOS-focused.)

## Setup and Building

**Current Platform Focus:** This project is currently developed and tested primarily on **macOS** using **MoltenVK** (Vulkan on Metal). While prerequisites for other platforms are listed, the code may require minor modifications (e.g., conditional compilation for portability extensions/flags) to compile and run correctly on Windows or Linux with native Vulkan drivers.

### Prerequisites

Before building, ensure you have the following installed:

1.  **C++ Compiler:** A modern C++ compiler supporting C++23.
    *   **macOS:** Install the Xcode Command Line Tools (`xcode-select --install`). This provides Clang.
    *   **Linux:** GCC or Clang.
    *   **Windows:** Visual Studio (with C++ workload) or MinGW/Clang.
2.  **CMake:** Version 3.10 or higher.
    *   **macOS:** `brew install cmake`
    *   **Linux:** `sudo apt update && sudo apt install cmake` (Debian/Ubuntu) or equivalent.
    *   **Windows:** Download from [cmake.org](https://cmake.org/download/).
3.  **Vulkan SDK:** Provides headers, validation layers, shader compiler (`glslc`), etc.
    *   **All Platforms:** Download and install from the LunarG website: [https://vulkan.lunarg.com/](https://vulkan.lunarg.com/)
    *   **macOS Specific:** The Vulkan SDK for macOS includes MoltenVK. Ensure the SDK installer sets up environment variables (like `VULKAN_SDK`) or source the `setup-env.sh` script in your shell profile (e.g., add `source /path/to/your/sdk/setup-env.sh` to `~/.zshrc` or `~/.bash_profile`). The code currently depends on MoltenVK's portability features.
4.  **GLFW:** Windowing and input library.
    *   **macOS:** `brew install glfw`
    *   **Linux:** `sudo apt install libglfw3-dev` (Debian/Ubuntu) or equivalent. *(Note: Code adjustments may be needed for instance extensions)*.
    *   **Windows:** Download pre-compiled binaries or source from [glfw.org](https://www.glfw.org/download.html) and configure CMake to find them. *(Note: Code adjustments may be needed for instance extensions)*.
5.  **GLM:** Math library (header-only).
    *   **macOS:** `brew install glm`
    *   **Linux:** `sudo apt install libglm-dev` (Debian/Ubuntu) or equivalent.
    *   **Windows:** Download from [glm.g-truc.net](https://glm.g-truc.net/0.9.9/index.html) and ensure headers are findable by CMake.

### Building the Project

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/cihanozcelik/VulkanApp.git
    cd VulkanApp
    ```
2.  **Configure using CMake:** Create a build directory and run CMake.
    ```bash
    mkdir build
    cd build
    cmake .. 
    # Or specify build type: cmake .. -DCMAKE_BUILD_TYPE=Release
    ```
3.  **Compile:**
    ```bash
    cmake --build . 
    # Or specify config for multi-config generators: cmake --build . --config Release
    ```
4.  **Run:** The executable (`VulkanApp`) will be placed in the `build` directory.
    ```bash
    ./VulkanApp
    ```

## Vulkan Cross-Platform Capabilities

Vulkan achieves cross-platform support through:

1.  **Standardized API:** The core Vulkan API is consistent across all supported platforms.
2.  **Platform-Specific Extensions:** Window system integration (WSI) and surface interaction are handled via extensions (e.g., `VK_KHR_surface`, `VK_EXT_metal_surface` for macOS, `VK_KHR_win32_surface` for Windows, `VK_KHR_xcb_surface` or `VK_KHR_wayland_surface` for Linux).
3.  **Hardware Vendors:** GPU vendors (NVIDIA, AMD, Intel, etc.) provide Vulkan drivers for their hardware on supported operating systems.
4.  **Translation Layers (Optional):** For platforms that don't natively support Vulkan (like macOS/iOS), translation layers like MoltenVK (Vulkan on Metal) or DXVK (Vulkan on DirectX) can be used.

This project currently uses MoltenVK and includes necessary portability extensions/flags for macOS development. Adapting for native Vulkan drivers on other platforms would require conditional compilation for certain instance extensions/flags.

## Current Status

*   Basic project setup with CMake.
*   GLFW window creation.
*   Vulkan Instance, Device (Physical/Logical), Surface, and Swap Chain setup.
*   Validation Layers and Debug Messenger enabled.
*   Code structure refactored for Single Responsibility Principle (SRP).
*   Render Pass created.

## Next Steps

*   Implement the Graphics Pipeline.
*   Load basic shaders.
*   Create framebuffers.
*   Set up command pools and buffers.
*   Implement basic rendering loop with synchronization.
*   Draw the first triangle! 