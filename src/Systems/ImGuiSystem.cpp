#include "ImGuiSystem.h"
#include "Core/FrameInfo.h"
#include "Core/Liara_Utils.h"
#include "Graphics/Liara_Device.h"

#include <vulkan/vulkan.h>
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_sdl2.h>
#include <stdexcept>

namespace Liara::Systems
{
    bool ImGuiSystem::IMGUI_INITIALIZED = false;

    ImGuiSystem::ImGuiSystem(const Plateform::Liara_Window& window, Graphics::Liara_Device& device, const VkRenderPass renderPass, const uint32_t imageCount): lveDevice{device}
    {
        // set up a descriptor pool stored on this instance
        const VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
        pool_info.pPoolSizes = pool_sizes;
        if (vkCreateDescriptorPool(device.GetDevice(), &pool_info, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up descriptor pool for imgui");
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        const ImGuiIO& io = ImGui::GetIO();
        (void) io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        // Initialize imgui for vulkan
        ImGui_ImplSDL2_InitForVulkan(window.GetWindow());
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.GetInstance();
        init_info.PhysicalDevice = device.GetPhysicalDevice();
        init_info.Device = device.GetDevice();
        init_info.QueueFamily = device.GetGraphicsQueueFamily();
        init_info.Queue = device.GetGraphicsQueue();

        // pipeline cache is a potential future optimization, ignoring for now
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.MinImageCount = 2;
        init_info.ImageCount = imageCount;
        init_info.CheckVkResultFn = Core::CheckVkResult;
        init_info.RenderPass = renderPass;

        ImGui_ImplVulkan_Init(&init_info);

        // upload fonts, this is done by recording and submitting a one time use command buffer
        // which can be done easily bye using some existing helper functions on the lve device object
        const auto commandBuffer = device.BeginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture();
        device.EndSingleTimeCommands(commandBuffer);
        //ImGui_ImplVulkan_DestroyFontUploadObjects();

        IMGUI_INITIALIZED = true;
    }

    ImGuiSystem::~ImGuiSystem()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(lveDevice.GetDevice(), descriptorPool, nullptr);
    }

    void ImGuiSystem::NewFrame()
    {
        if (!IMGUI_INITIALIZED) { return; }
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiSystem::Update(const Core::FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo)
    {
        SDL_PumpEvents();
        for (const auto& element: m_Elements) { element->Draw(frame_info, ubo); }
    }

    void ImGuiSystem::Render(const Core::FrameInfo &frame_info) const
    {
        ImGui::Render();
        ImDrawData* drawdata = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawdata, frame_info.m_CommandBuffer);
    }
}
