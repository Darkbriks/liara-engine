#pragma once

#include "Liara_Camera.h"
#include "Liara_GameObject.h"
#include "Graphics/Liara_Device.h"
#include "Graphics/Liara_Renderer.h"
#include "Graphics/Liara_Texture.h"
#include "Graphics/Descriptors/Liara_Descriptor.h"
#include "Plateform/Liara_Window.h"
#include "Systems/Liara_System.h"

#include <memory>
#include <string>

namespace Liara::Systems {
    class ImGuiSystem;
}

namespace Liara::Core { struct FrameInfo; }

namespace Liara::Core
{
    class Liara_App
    {
    public:
        const std::string TITLE;
        const unsigned short WIDTH;
        const unsigned short HEIGHT;

        explicit Liara_App(std::string title="Liara Engine", unsigned short width=800, unsigned short height=600);
        virtual ~Liara_App() = default;
        Liara_App(const Liara_App&) = delete;
        Liara_App& operator=(const Liara_App&) = delete;

        virtual void Run();

        void AddSystem(std::unique_ptr<Systems::Liara_System> system) { m_Systems.push_back(std::move(system)); }

    protected:
        virtual void Init();

        virtual void InitUboBuffers();
        virtual void InitDescriptorSets();
        virtual void InitSystems();
        virtual void InitCamera();
        virtual void LateInit() {}

        virtual void SetProjection(float aspect);

        virtual void ProcessInput(float frameTime) {}
        virtual void Update(const FrameInfo& frameInfo) {}
        virtual void Render(const FrameInfo& frameInfo) {}

        virtual void Close();

    private:
        void MasterProcessInput(float frameTime);
        void MasterUpdate(const FrameInfo& frameInfo);
        void MasterRender(const FrameInfo &frameInfo);

    protected:
        Plateform::Liara_Window m_Window;
        Graphics::Liara_Device m_Device;
        Graphics::Liara_Renderer m_Renderer;

        std::vector<std::unique_ptr<Graphics::Liara_Buffer>> m_UboBuffers;

        std::unique_ptr<Graphics::Descriptors::Liara_DescriptorAllocator> m_DescriptorAllocator;
        std::unique_ptr<Graphics::Descriptors::Liara_DescriptorLayoutCache> m_DescriptorLayoutCache;
        VkDescriptorSetLayout m_GlobalSetLayout{};
        std::vector<VkDescriptorSet> m_GlobalDescriptorSets;

        Liara_Camera m_Camera;
        Liara_GameObject::Map m_GameObjects;
        std::vector<std::unique_ptr<Systems::Liara_System>> m_Systems;

        // TODO: Test texture, temporary
        std::unique_ptr<Graphics::Liara_Texture> m_Texture;
    };
}