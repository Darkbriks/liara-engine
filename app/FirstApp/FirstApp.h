#pragma once

#include "Graphics/Liara_Device.h"
#include "Graphics/Liara_Renderer.h"
#include "Core/Liara_GameObject.h"
#include "Graphics/Descriptors/Liara_Descriptor2.h"
#include "Plateform/Liara_Window.h"

class FirstApp
{
public:
    static constexpr unsigned short WIDTH = 800;
    static constexpr unsigned short HEIGHT = 600;

    FirstApp();
    ~FirstApp() = default;
    FirstApp(const FirstApp&) = delete;
    FirstApp& operator=(const FirstApp&) = delete;

    void Run();

private:
    void LoadGameObjects();

    Liara::Plateform::Liara_Window m_Window{ "Test App With Vulkan", WIDTH, HEIGHT };
    Liara::Graphics::Liara_Device m_Device{ m_Window };
    Liara::Graphics::Liara_Renderer m_Renderer{m_Window, m_Device};

    // Note: Order of declaration matters
    std::unique_ptr<Liara::Graphics::Descriptors::Liara_DescriptorAllocator> m_descriptorAllocator{};
    std::unique_ptr<Liara::Graphics::Descriptors::Liara_DescriptorLayoutCache> m_descriptorLayoutCache{};
    Liara::Core::Liara_GameObject::Map m_GameObjects;
};
