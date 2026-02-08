# Aether Studio

**Qt6 tabanlı ön odak arayüzü** — Edit, Animation, Photo/RAW, Color, Audio için profesyonel kreatif paket.

Qt6 ile geliştirilmiş, ön odak (front-end) arayüzüne sahip tek masaüstü uygulaması.

### Description

Aether Studio, **Qt6** kullanarak geliştirilmiş, video düzenleme, animasyon, fotoğraf/RAW, renk ve ses işleme için tek bir **ön odak arayüzü** sunan profesyonel kreatif pakettir. Vulkan, FFmpeg ve modern C++20 ile desteklenir.

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
- **Qt6** – ana ön odak arayüzü (main front-end UI)
- C++20
- Vulkan for rendering
- FFmpeg for media; ImGui/Vulkan where used
