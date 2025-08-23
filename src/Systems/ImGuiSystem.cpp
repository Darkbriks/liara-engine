#include "ImGuiSystem.h"

#include "Core/FrameInfo.h"
#include "Core/ImGui/ImGuiElementMainMenu.h"
#include "Core/Liara_Utils.h"
#include "Graphics/Liara_Device.h"

#include <vulkan/vulkan.h>

#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include <stdexcept>

namespace Liara::Systems
{
    bool ImGuiSystem::imguiInitialized = false;

    ImGuiSystem::ImGuiSystem(const Plateform::Liara_Window& window,
                             Graphics::Liara_Device& device,
                             const Core::ApplicationInfo& appInfo,
                             VkRenderPass renderPass,
                             const uint32_t imageCount)
        : Liara_System("ImGui System", {.major = 0, .minor = 2, .patch = 3, .prerelease = "dev"})
        , m_lveDevice{device} {
        // set up a descriptor pool stored on this instance
        const VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000}
        };
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
        poolInfo.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes));
        poolInfo.pPoolSizes = poolSizes;
        if (vkCreateDescriptorPool(device.GetDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            LIARA_THROW_RUNTIME_ERROR(LogSystems, "Failed to set up descriptor pool for imgui");
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        const ImGuiIO& io = ImGui::GetIO();
        (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        // Initialize imgui for vulkan
        ImGui_ImplSDL2_InitForVulkan(window.GetWindow());
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = device.GetInstance();
        initInfo.PhysicalDevice = device.GetPhysicalDevice();
        initInfo.Device = device.GetDevice();
        initInfo.QueueFamily = device.GetGraphicsQueueFamily();
        initInfo.Queue = device.GetGraphicsQueue();

        // pipeline cache is a potential future optimization, ignoring for now
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = m_descriptorPool;
        initInfo.Allocator = VK_NULL_HANDLE;
        initInfo.MinImageCount = 2;
        initInfo.ImageCount = imageCount;
        initInfo.CheckVkResultFn = Core::CheckVkResult;
        initInfo.RenderPass = renderPass;

        ImGui_ImplVulkan_Init(&initInfo);

        // upload fonts, this is done by recording and submitting a one time use command buffer
        // which can be done easily bye using some existing helper functions on the lve device object
        auto* const commandBuffer = device.BeginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture();
        device.EndSingleTimeCommands(commandBuffer);
        // ImGui_ImplVulkan_DestroyFontUploadObjects();

        imguiInitialized = true;

        AddElement(std::make_unique<Core::ImGuiElements::MainMenu>(appInfo));
    }

    ImGuiSystem::~ImGuiSystem() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(m_lveDevice.GetDevice(), m_descriptorPool, nullptr);
    }

    void ImGuiSystem::NewFrame() {
        if (!imguiInitialized) { return; }
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiSystem::Update(const Core::FrameInfo& frameInfo, Graphics::Ubo::GlobalUbo& ubo) {
        SDL_PumpEvents();
        for (const auto& element : m_Elements) { element->Draw(frameInfo, ubo); }
    }

    void ImGuiSystem::Render(const Core::FrameInfo& frameInfo) const {
        ImGui::Render();
        ImDrawData* drawdata = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawdata, frameInfo.commandBuffer);
    }
}
