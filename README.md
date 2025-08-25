# Liara Engine

[![Linux Build](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-linux.yml/badge.svg)](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-linux.yml)
[![Windows Build](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-windows.yml/badge.svg)](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-windows.yml)
[![Code Style](https://github.com/Darkbriks/liara-engine/workflows/Code%20Style%20Check/badge.svg)](https://github.com/Darkbriks/liara-engine/actions)
[![License](https://img.shields.io/github/license/Darkbriks/LiaraEngine_CPP_VULKAN)](LICENSE)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Vulkan](https://img.shields.io/badge/Vulkan-1.3-red.svg)](https://www.vulkan.org/)

> A modern 3D graphics engine built from scratch in C++20 with Vulkan

Liara Engine is a cross-platform 3D graphics engine designed to provide a solid foundation for learning modern graphics programming techniques using Vulkan. It features a modular architecture, real-time rendering capabilities, flexible settings system, and optional C++20 modules support, making it suitable for both educational purposes and small-scale game development.

---

## Used Technologies

- **Graphics API**: Vulkan 1.3+
- **Windowing**: SDL2
- **Math**: GLM
- **UI**: Dear ImGui
- **3D Models**: TinyObjLoader
- **Textures**: STB Image
- **JSON**: nlohmann/json
- **Standard**: C++20 (with optional modules support)
- **Build System**: CMake 3.29+
- **Documentation**: Doxygen
- **Testing**: GitHub Actions for CI/CD

---

## Build Status

The builds are automatically tested on multiple Linux distributions and Windows:

| Platform | Status | Tested Distributions/Versions |
|----------|--------|-------------------------------|
| Linux    | [![Linux Build](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-linux.yml/badge.svg)](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-linux.yml) | Ubuntu 22.04, Fedora Latest, Arch Linux Latest |
| Windows  | [![Windows Build](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-windows.yml/badge.svg)](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-windows.yml) | Windows Server 2022 (MSVC 2022) |

**Supported Compilers:**
- **Windows**: MSVC 2019 16.10+ (recommended: MSVC 2022 17.6+)
- **Linux**: GCC 13+, Clang 20+

---

## Demo

*Todo: Add a video or screenshots of the demo application showcasing the engine's capabilities.*

---

## Quick Start

### Prerequisites

**Essential Requirements:**
- **C++20 compatible compiler**:
  - Windows: MSVC 2019 16.10+ (Visual Studio 2019/2022)
  - Linux: GCC 13+ or Clang 20+
- **Vulkan SDK 1.3.280.0+** with glslc compiler
- **CMake 3.29+**
- **Git** with submodules support

**Vulkan SDK Installation:**

*Windows:*
- Download from [LunarG website](https://vulkan.lunarg.com/)
- Follow installer instructions and verify environment variables

*Linux:*
```bash
# Ubuntu/Debian
sudo apt install vulkan-sdk vulkan-validationlayers-dev

# Fedora
sudo dnf install vulkan-devel vulkan-validation-layers-devel

# Arch Linux
sudo pacman -S vulkan-devel vulkan-validation-layers
```

**Verify Installation:**
```bash
glslc --version  # Should show Vulkan SDK version
```

### Building

**1. Clone the Repository**
```bash
git clone --recursive https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN.git
cd LiaraEngine_CPP_VULKAN
```

**2. Choose Your Configuration**

The project uses CMake presets for easy configuration. Available presets:

**Linux:**
```bash
# Debug builds
cmake --preset=linux-debug-gcc      # GCC compiler
cmake --preset=linux-debug-clang    # Clang compiler

# Release builds (recommended)
cmake --preset=linux-release-gcc    # Optimized GCC build
cmake --preset=linux-release-clang  # Optimized Clang build
```

**Windows:**
```bash
# With C++20 modules (recommended for MSVC 2022 17.6+)
cmake --preset=windows-release-modules
cmake --preset=windows-debug-modules

# Traditional headers (more compatible)
cmake --preset=windows-release-no-modules  
cmake --preset=windows-debug-no-modules
```

**3. Build**
```bash
# Use the same preset name for building
cmake --build --preset=linux-release-gcc
# or
cmake --build --preset=windows-release-modules
```

**4. Run the Demo**

The demo must be run from its directory to ensure correct resource paths:

*Linux:*
```bash
cd build/linux-release-gcc/app/
./Demo
```

*Windows:*
```bash
cd build/windows-release-modules/app/Release/
Demo.exe
```

### Controls & Features

**Demo Navigation:**
- **WASD**: Move camera forward/back/left/right
- **QE**: Move camera down/up
- **Up/Down/Left/Right Arrows**: Rotate camera
- **F11**: Toggle fullscreen
- **F10**: Toggle VSync
- **F1**: Toggle engine stats
- **F2**: Toggle log console
- **Ctrl+E**: Engine statistics
- **Ctrl+L**: Log console

**ImGui Interface:**
- Real-time performance metrics
- Configurable logging system
- Engine statistics and debugging tools

---

## Advanced Build Options

### Build Options
```bash
# Available CMake options
-DLIARA_BUILD_APPS=ON          # Build demo applications (default: ON)
-DLIARA_BUILD_TESTS=OFF        # Build unit tests (default: OFF)  
-DLIARA_EMBED_SHADERS=ON       # Embed shaders in executable (default: OFF for Debug, ON for Release)
-DLIARA_ENABLE_VALIDATION=ON   # Enable Vulkan validation layers (default: ON for Debug)
-DLIARA_ENABLE_MODULES=ON      # Enable C++20 modules (auto-detected)
```

### C++20 Modules Support

The engine optionally supports C++20 modules for faster compilation and better dependency management:

- **Automatically detected** based on compiler and version
- **Force enable**: `-DLIARA_FORCE_ENABLE_MODULES=ON`
- **Force disable**: `-DLIARA_FORCE_NO_MODULES=ON`

**Module Support Status:**
- ✅ MSVC 2022 17.6+ (stable)
- ⚠️ MSVC 2022 17.2+ (good, some edge cases)
- ⚠️ MSVC 2019 16.10+ (experimental, may be unstable)
- ✅ Clang 20+ (supported)
- ❌ GCC 14+ (temporarily disabled due to stability issues)

### Custom Configuration

For advanced users, you can configure manually:

```bash
mkdir build && cd build
cmake .. \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLIARA_EMBED_SHADERS=ON \
  -DLIARA_ENABLE_VALIDATION=OFF
  
ninja
```

---

## Architecture Overview
```
Liara Engine v0.17
├── Core/                   # Engine foundation
│   ├── Application         # Main app loop and lifecycle
│   ├── GameObject          # Entity management  
│   ├── Camera              # View and projection matrices
│   ├── Settings            # Configuration system with serialization
│   ├── Logging             # Multi-threaded logging system
│   └── SignalHandler       # Graceful shutdown handling
├── Graphics/               # Rendering subsystem
│   ├── Device              # Vulkan device abstraction
│   ├── Pipeline            # Shader pipeline management
│   ├── Renderers/          # Multiple rendering backends
│   ├── Descriptors/        # Vulkan descriptor management
│   ├── Resources/          # Buffers, textures, models
│   └── SwapChain           # Vulkan swapchain management
├── Systems/                # ECS-style systems
│   ├── RenderSystem        # 3D object rendering
│   ├── LightSystem         # Dynamic lighting calculations
│   └── ImGuiSystem         # ImGui integration with console
├── UI/                     # User interface components
│   ├── ImGuiEngineStats    # Performance monitoring
│   └── ImGuiLogConsole     # Real-time log viewer
└── Platform/               # OS abstraction
    └── Window              # SDL2 window management
```

---

## Design Principles

- **RAII**: Automatic resource management with smart pointers
- **Modern C++**: Leveraging C++20 features for performance and safety
- **Modular**: Swappable renderers and systems architecture
- **Performance-first**: Memory-efficient, cache-friendly design
- **Cross-platform**: Write once, run on Windows and Linux

---

## Performance Metrics

| Metric            | Value                                     |
|-------------------|-------------------------------------------|
| Platform support  | Windows 10+, Linux (Ubuntu, Fedora, Arch) |
| Vulkan version    | 1.3+ with backwards compatibility         |
| Rendering         | Forward rendering with deferred planned   |
| Memory management | RAII with custom allocators               |

---

## Roadmap

### v0.17 - Consolidation (Current)

- [x] Enhanced settings system with serialization
- [x] Rework of specialization constants
- [x] Fix a Linux hack from v0.14
- [x] Fix a vulkan validation layer issue (semaphore synchronization in Swapchain)
- [x] Multi-threaded logging system with ImGui console
- [x] CI/CD pipeline for Linux and Windows
- [x] Improved CMake build system
- [x] Code style enforcement (clang-format, clang-tidy)
- [x] Signal handling for graceful shutdown
- [x] AppImage generation for Linux distribution
- [x] Cross-platform window management improvements
- [ ] C++20 modules support (optional)
  - [x] MSVC
  - [x] Clang
  - [ ] GCC (temporarily disabled due to stability issues)
  - [ ] Migrate some core systems to modules
    - [x] Liara_Utils.h
    - [ ] Result.h
    - [ ] ApplicationInfo.h
    - [ ] Logger
- [ ] Add Catch2 for unit testing

### v0.18 - Advanced Rendering & ECS (Next)

- [ ] Complete ECS architecture migration
- [ ] Multiple texture support
  - [ ] Texture arrays and atlases
  - [ ] Bindless textures
- [ ] Simple material system
- [ ] Integrated profiler with ImGui interface
- [ ] ImGui-based settings editor
- [ ] Wiki documentation with Doxygen
  - [ ] Github Pages hosting with CI/CD pipeline
  - [ ] User oriented documentation (mdbook or similar)
- [ ] Testing framework for visual regression
- [ ] Script component system (C++ based)

### v0.19 - Lighting & Scenes

- [ ] Wireframe rendering modes
- [ ] Shadow mapping system (cascaded shadow maps)
- [ ] Multiple render passes architecture
- [ ] Deferred rendering pipeline
- [ ] Scene management with GLTF support
- [ ] Asynchronous asset loading system

### v0.20 - Advanced Rendering Techniques

- [ ] PBR (Physically Based Rendering) materials
- [ ] Ray tracing renderer
- [ ] Skybox and IBL (Image-Based Lighting)
- [ ] Post-processing pipeline with effects
- [ ] Anti-aliasing (MSAA, TAA, FXAA)
- [ ] Tessellation and displacement mapping

### v0.21 - Optimization & Multi Window Support

- [ ] LOD (Level of Detail) system
- [ ] Occlusion culling
- [ ] Instancing support
- [ ] Static and dynamic lighting with baked lighting
- [ ] Multi-window support with drag-and-drop (Kitten Space Agency like)

### Other Features I'd Like to Implement (not planned yet)

- Hot-reloading of shaders and assets
- Physics engine integration (Bullet, PhysX, etc.) or custom
- Listener system for events (mouse, keyboard, gamepad)
- AI system (pathfinding, behavior trees)
- Multiple camera support
- Material editor
- Scene editor
- Animation system (skeletal, morph target)
- Terrain rendering
- Particle and volumetric effects
- Audio system integration
- Networking capabilities

---

## Configuration

The engine features a sophisticated settings system with runtime modification support:

```cpp
// Basic settings
settings.SetBool("graphics.vsync", true);
settings.SetUInt("texture.max_anisotropy", 16);
settings.SetFloat("camera.fov", 70.0f);

// Complex types with serialization
settings.RegisterSetting("graphics.ambient_light", glm::vec3(0.1f, 0.1f, 0.1f));
settings.Set("graphics.ambient_light", glm::vec3(0.2f, 0.2f, 0.2f));

// Subscribe to changes
settings.Subscribe<bool>("graphics.vsync", [](const bool& newValue) {
    // Handle VSync change
});
```

**Features:**
- Automatic serialization to `settings.cfg`
- Type-safe API with compile-time validation
- Runtime change notifications
- Performance-optimized storage for frequent access

---

## Contributing

This is primarily a learning project, but feedback and contributions are welcome!

### 🐛 Found a bug?

Open an issue with:
- Platform and hardware details
- Steps to reproduce with minimal example
- Expected vs actual behavior
- Log output (available via F2 in demo or in a log file)

### 💡 Have suggestions?

- Architecture improvements
- Performance optimizations
- Feature requests
- Code quality improvements

### 📝 Code Style

The project enforces consistent code style:
- **clang-format**: Automatic code formatting
- **clang-tidy**: Static analysis and modern C++ suggestions
- **GitHub Actions**: Automated style checking on PRs

Run locally:
```bash
# Format code
find src app -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Check with clang-tidy
clang-tidy src/**/*.cpp -p build/
```

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- **Vulkan API**: The Khronos Group for the modern graphics API
- **Brendan Galea**: For his excellent [YouTube Vulkan tutorial series](https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR)
- **Vulkan Tutorial**: [vulkan-tutorial.com](https://vulkan-tutorial.com/) for comprehensive learning resources

---

<div align="center">

## Built with ❤️, epic soundtracks 🎵 and purr-fessional supervision 🐱

</div>