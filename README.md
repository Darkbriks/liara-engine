# Liara Engine

[![Linux Build](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-linux.yml/badge.svg)](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-linux.yml)
[![Windows Build](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-windows.yml/badge.svg)](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-windows.yml)
[![Code Style](https://github.com/Darkbriks/liara-engine/workflows/Code%20Style%20Check/badge.svg)](https://github.com/Darkbriks/liara-engine/actions)
[![License](https://img.shields.io/github/license/Darkbriks/LiaraEngine_CPP_VULKAN)](LICENSE)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Vulkan](https://img.shields.io/badge/Vulkan-1.3-red.svg)](https://www.vulkan.org/)

> A modern 3D graphics engine built from scratch in C++20 with Vulkan

Liara Engine is a cross-platform 3D graphics engine designed to provide a solid foundation for learning modern graphics programming techniques using Vulkan. It features a modular architecture, real-time rendering capabilities, and a flexible settings system, making it suitable for both educational purposes and small-scale game development.

---

## Used Technologies

- **Graphics API**: Vulkan 1.3
- **Windowing**: SDL2
- **Math**: GLM
- **UI**: Dear ImGui
- **3D Models**: TinyObjLoader
- **Textures**: STB Image
- **Standard**: C++20
- **Build System**: CMake 3.25+
- **Documentation**: Doxygen
- **Testing**: GitHub Actions for CI/CD
---

## Build Status

The builds are automatically tested on multiple Linux distributions and Windows:

| Platform | Status                                                                                                                                                                                                             | Tested Distributions                |
|----------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------|
| Linux    | [![Linux Build](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-linux.yml/badge.svg)](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-linux.yml)       | Ubuntu 22.04, Fedora 39, Arch Linux |
| Windows  | [![Windows Build](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-windows.yml/badge.svg)](https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN/actions/workflows/compile-windows.yml) | Windows Server 2022 (MSVC)          |

## Demo

*Todo: Add a video or screenshots of the demo application showcasing the engine's capabilities.*

---

## Quick Start
### Prerequisites

- C++20 compatible compiler (GCC 13+, Clang 12+, MSVC 2019+)
- Vulkan SDK 1.3+ and glslc compiler (included in the official SDK)
  - On Linux, install the Vulkan SDK via your package manager or download it from the [LunarG website](https://vulkan.lunarg.com/).
  - On Windows, download the Vulkan SDK from the [LunarG website](https://vulkan.lunarg.com/) and set the environment variables as instructed.
  - Alternatively, you can install vulkan packages via your package manager. Make sure to install the `vulkan-devel` package (or equivalent) and the `glslang` package (can be included in `shaderc` or `glslang` packages).
- CMake 3.25+
- Git with submodules support

### Building

Linux :
```bash
# Clone with submodules
git clone --recursive https://github.com/Darkbriks/liara-engine.git
cd liara-engine

# Configure and build
cmake --preset=linux-release # You can also use linux-debug for debug build
cmake --build --preset=linux-release

# Run demo
cd build/linux-release/app/ # It's important to run the demo from the app directory to ensure correct resource paths
./Demo
```

Windows :
```bash
# Clone with submodules
git clone --recursive https://github.com/Darkbriks/liara-engine.git
cd liara-engine

# Configure and build
cmake --preset=windows-release # You can also use windows-debug for debug build
cmake --build --preset=windows-release

# Run demo
cd build/windows-release/app/Release/ # It's important to run the demo from the app directory to ensure correct resource paths
Demo.exe
```

These commands will compile the engine and run the demo application. The demo showcases some capabilities of the engine.
The demo loads a viking room model, with dynamic lighting, ImGui interface, and basic camera controls.
You can navigate the scene using WASD keys. You can also modify fullscreen mode with F11, and VSync with F10.

---

## Architecture Overview
```
Liara Engine
├── Core/                  # Engine foundation
│   ├── Application        # Main app loop and lifecycle
│   ├── GameObject         # Entity management
│   ├── Camera             # View and projection matrices
│   └── Settings           # Configuration system
├── Graphics/              # Rendering subsystem
│   ├── Device             # Vulkan device abstraction
│   ├── Pipeline           # Shader pipeline management
│   ├── Renderers/         # Multiple rendering backends
│   ├── Descriptors/       # Vulkan descriptor management
│   └── Resources/         # Buffers, textures, models
├── Systems/               # ECS-style systems
│   ├── RenderSystem       # 3D object rendering
│   ├── LightSystem        # Lighting calculations
│   └── ImGuiSystem        # ImGui integration
└── Platform/              # OS abstraction
    └── Window             # SDL2 window management
```

---

## Design Principles

- RAII: Automatic resource management
- Modern C++: Leveraging C++20 features for performance
- Modular: Swappable renderers and systems
- Performance-first: Memory-efficient, cache-friendly design

---

## Performance Metrics

| Metric           | Value                                       |
|------------------|---------------------------------------------|
| TODO             | TODO                                        |
| Platform support | Windows, Ubuntu, Debian, Fedora, Arch Linux |

---

## Roadmap

### v0.17 - Consolidation (In Progress)

- [x] Rework of settings system
- [x] Rework of specialization constants
- [x] Fix a Linux hack from v0.14
- [x] Fix a vulkan validation layer issue (semaphore synchronization in Swapchain)
- [x] CI/CD pipeline for Linux and Windows
- [x] Logger
- [ ] Migrate to C++20 features (work in progress)
- [x] Rework of CMake build system
- [ ] Add Catch2 for unit testing
- [x] Add README.md documentation (work in progress)
- [x] Better definition of coding style (clang-format, clang-tidy, dedicated GitHub Action)

### v0.18 - Advanced Rendering & ECS

- [ ] Complete ECS architecture
- [ ] Multiple texture support
- [ ] Simple material system
- [ ] Bindless textures support ?
- [ ] Profiler
- [ ] ImGui integration for settings and debugging
- [ ] Wiki documentation with doxygen, github pages and github actions
- [ ] Testing framework for visual regression
- [ ] Script component system (c++ only for now)

### v0.19 - Lighting & Scenes

- [ ] Wireframe rendering
- [ ] Shadow mapping system
- [ ] Multiple render passes (preparation for deferred rendering)
- [ ] Deferred rendering pipeline
- [ ] Scene management (GLTF support)
- [ ] Asynchronous asset loading

### v0.20 - Advanced Rendering Techniques

- [ ] PBR (Physically Based Rendering) materials
- [ ] Ray tracing renderer
- [ ] Skybox and IBL
- [ ] Post-processing pipeline
- [ ] Anti-aliasing (MSAA, TAA, FXAA...)
- [ ] Tesselation and displacement mapping

### v0.21 - Optimization & Multi Window Support

- [ ] LOD (Level of Detail) system
- [ ] Occlusion culling
- [ ] Instancing support
- [ ] Static and dynamic lighting with baked lighting
- [ ] Multi-window support
- [ ] Drag and drop ImGui interface between windows (Kitten Space Agency like)

### Other Features I'd Like to Implement (not planned yet)

- Hot-reloading of shaders and assets
- Hot-reloading of C++ scripts
- Particle and volumetric effects
- Physics engine integration (Bullet, PhysX, etc.) or custom
- Listener system for events (mouse, keyboard, gamepad)
- Multiplayer networking support (UDP/TCP)
- AI system (pathfinding, behavior trees)
- Multiple camera support
- Material editor
- Scene editor
- Animation system (skeletal, morph target)
- Terrain rendering
- Audio system

---

## Configuration

The engine uses a flexible settings system supporting runtime modifications:
```c++
// Trivial type settings
settings.SetBool("graphics.vsync", true);
settings.SetUInt("texture.max_anisotropy", 16);

// Complex types with custom serialization
settings.RegisterType<glm::vec3>("graphics.ambient_light", glm::vec3(0.1f, 0.1f, 0.1f));
settings.Set<glm::vec3>("graphics.ambient_light", glm::vec3(0.2f, 0.2f, 0.2f));

// Accessing settings
bool vsync = settings.GetBool("graphics.vsync");
glm::vec3 ambientLight = settings.Get<glm::vec3>("graphics.ambient_light");
```
Settings are automatically saved to `settings.cfg` on exit and loaded on startup, allowing persistent configuration across sessions. In the future, I plan to add a GUI for settings management using ImGui.

---

## Contributing

This is primarily a learning project, but feedback and suggestions are welcome!

### 🐛 Found a bug?

Open an issue with:
- Platform and hardware details
- Steps to reproduce
- Expected vs actual behavior

### 💡 Have suggestions?

- Architecture improvements
- Performance optimizations
- Feature requests

---

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Heartfelt Thanks

- Vulkan team for the [Vulkan API](https://www.vulkan.org/)
- Brendan Galea for his [YouTube game engine series](https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR)
- [Vulkan Tutorial](https://vulkan-tutorial.com/)

---

<div style="text-align: center; margin-top: 20px;">
<h1>Built with ❤️, epic soundtracks 🎵 and purr-fessional supervision 🐱</h1>
</div>