# Aether Studio - Dependencies

## Required Dependencies

### Build System
- **CMake 3.20+** - Build system
- **C++20 Compiler** - MSVC 2019+, GCC 10+, or Clang 12+

### Core Libraries
- **Vulkan SDK 1.3+** - Graphics API
  - Download from: https://vulkan.lunarg.com/
  - Windows: Install Vulkan SDK and ensure it's in PATH
  - Linux: Install via package manager (`sudo apt-get install vulkan-sdk`)
  - macOS: Install via Homebrew (`brew install vulkan-headers vulkan-loader`)

### Auto-downloaded via FetchContent
- **GLFW 3.3.8** - Window management (automatically downloaded)
- **ImGui v1.90.0** - UI framework (automatically downloaded)
- **nlohmann/json v3.11.2** - JSON parsing (automatically downloaded)

## Optional Dependencies

### Video Processing
- **FFmpeg** - Video encoding/decoding (highly recommended)
  - **Windows:**
    - Download pre-built binaries from https://ffmpeg.org/download.html
    - Extract and add `bin` folder to PATH
    - Or use vcpkg: `vcpkg install ffmpeg`
  - **Linux:**
    ```bash
    sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev
    ```
  - **macOS:**
    ```bash
    brew install ffmpeg
    ```
  - If FFmpeg is not found, video processing features will be limited to placeholder implementations.

### Cryptographic Functions (Optional)
- **OpenSSL** - Enhanced cryptographic functions
  - **Windows:** Download from https://slproweb.com/products/Win32OpenSSL.html or use vcpkg
  - **Linux:** `sudo apt-get install libssl-dev`
  - **macOS:** `brew install openssl`
  - If not found, the built-in SHA-256 implementation will be used.

### AI Upscaling (Future)
- **NVIDIA TensorRT** - AI upscaling support (future feature)
  - Requires NVIDIA GPU with Tensor Cores (RTX series)
  - Download from: https://developer.nvidia.com/tensorrt
  - Currently implemented as placeholder

## Windows-Specific Libraries

These are automatically linked on Windows:
- `ws2_32` - Winsock2 for network operations (AetherLink)
- `psapi` - Process API for memory monitoring
- `wbemuuid` - WMI for hardware ID generation
- `ole32`, `oleaut32` - COM for hardware queries
- `pdh` - Performance Data Helper for GPU monitoring

## Installation Instructions

### Windows

1. Install Visual Studio 2019 or later with C++ support
2. Install CMake from https://cmake.org/
3. Install Vulkan SDK from https://vulkan.lunarg.com/
4. (Optional) Install FFmpeg:
   - Download from https://ffmpeg.org/download.html
   - Extract to `C:\ffmpeg`
   - Add `C:\ffmpeg\bin` to system PATH
5. Build the project:
   ```powershell
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

### Linux (Ubuntu/Debian)

```bash
# Install build tools
sudo apt-get update
sudo apt-get install build-essential cmake

# Install Vulkan SDK
sudo apt-get install vulkan-sdk

# Install FFmpeg (optional but recommended)
sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev

# Install OpenSSL (optional)
sudo apt-get install libssl-dev

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake vulkan-headers vulkan-loader

# Install FFmpeg (optional but recommended)
brew install ffmpeg

# Install OpenSSL (optional)
brew install openssl

# Build
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

## vcpkg Integration (Alternative)

If you prefer using vcpkg for dependency management:

1. Install vcpkg: https://github.com/Microsoft/vcpkg
2. Install packages:
   ```bash
   vcpkg install ffmpeg openssl
   ```
3. Configure CMake with vcpkg:
   ```bash
   cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
   ```

## Verification

After building, verify dependencies:

```bash
# Check if FFmpeg is available
ffmpeg -version

# Check if Vulkan is available
vulkaninfo
```

## Troubleshooting

### FFmpeg not found
- Ensure FFmpeg is installed and in PATH
- On Windows, restart terminal after adding to PATH
- On Linux/macOS, ensure development packages are installed (not just runtime)

### Vulkan not found
- Verify Vulkan SDK is installed
- Check that `VULKAN_SDK` environment variable is set (Windows)
- Ensure Vulkan loader is in system library path

### Build errors
- Ensure all required dependencies are installed
- Try cleaning build directory: `rm -rf build && mkdir build`
- Check CMake output for missing dependencies
