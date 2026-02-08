# Aether Studio - Build Instructions

## Prerequisites

1. **CMake 3.20+** - Download from https://cmake.org/download/
2. **C++20 Compatible Compiler**:
   - Windows: Visual Studio 2019 or later (with C++ desktop development workload)
   - Or: MinGW-w64 with GCC 10+
3. **Vulkan SDK 1.3+** - Download from https://vulkan.lunarg.com/sdk/home

## Building

1. **Clone or download the project**

2. **Create build directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**:
   ```bash
   cmake ..
   ```
   Or with Visual Studio generator:
   ```bash
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

4. **Build**:
   ```bash
   cmake --build . --config Release
   ```
   Or open the generated solution in Visual Studio and build from there.

5. **Run**:
   The executable will be in `build/bin/Release/` (or `build/bin/` on Unix-like systems).

## Dependencies

The following dependencies are automatically downloaded by CMake using FetchContent:
- **GLFW 3.3.8** - Window management
- **ImGui v1.90.0** - UI framework

## Troubleshooting

### Vulkan SDK Not Found Error

If you encounter the error:
```
Could NOT find Vulkan (missing: Vulkan_LIBRARY Vulkan_INCLUDE_DIR)
```

**Solution 1: Install Vulkan SDK**
1. Download and install the Vulkan SDK from https://vulkan.lunarg.com/sdk/home
2. The installer should automatically set the `VULKAN_SDK` environment variable
3. Restart Visual Studio/CMake after installation

**Solution 2: Set VULKAN_SDK Manually**
If Vulkan SDK is installed but CMake can't find it:
1. Find your Vulkan SDK installation path (commonly `C:/VulkanSDK/1.3.xxx.x`)
2. Set the environment variable:
   - Open System Properties → Environment Variables
   - Add or edit `VULKAN_SDK` to point to your SDK path
3. Or specify it when configuring CMake:
   ```bash
   cmake .. -DVULKAN_SDK=C:/VulkanSDK/1.3.xxx.x
   ```

**Solution 3: Use CMakeSettings.json**
1. Open `CMakeSettings.json` in Visual Studio
2. Set the `VULKAN_SDK` variable to your SDK path:
   ```json
   {
       "name": "VULKAN_SDK",
       "value": "C:/VulkanSDK/1.3.xxx.x",
       "type": "STRING"
   }
   ```

## Notes

- Make sure Vulkan SDK is properly installed and `VULKAN_SDK` environment variable is set (usually done automatically by the SDK installer).
- On Windows, you may need to copy Vulkan DLLs to the executable directory if they're not in your PATH.
