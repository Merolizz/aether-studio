# Aether Studio

Professional Creative Suite - Edit, Animation, Photo/RAW, Color, Audio

## Building

### Prerequisites
- CMake 3.20+
- C++20 compatible compiler (MSVC 2019+, GCC 10+, Clang 12+)
- Vulkan SDK 1.3+
- Windows 10/11 (Linux/macOS support in development)

### Optional (Recommended)
- FFmpeg - For full video processing support
- OpenSSL - For enhanced cryptographic functions

See [DEPENDENCIES.md](DEPENDENCIES.md) for detailed installation instructions.

### Build Steps

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The executable will be in `build/bin/`.

## Project Structure

- `src/` - Source code
- `include/` - Header files
- `external/` - Third-party libraries (ImGui, GLFW)
- `resources/` - Resource files

## Development

This project uses:
- C++20
- ImGui for UI
- Vulkan for rendering
- GLFW for window management
