#include "ImGuiSystem.h"
#include "Core/FrameInfo.h"
#include "Graphics/Liara_Device.h"

#include <vulkan/vulkan.h>
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
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
        ImGui_ImplGlfw_InitForVulkan(window.GetWindow(), true);
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
        init_info.CheckVkResultFn = CheckVkResult;
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
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(lveDevice.GetDevice(), descriptorPool, nullptr);
    }

    void ImGuiSystem::NewFrame()
    {
        if (!IMGUI_INITIALIZED) { return; }
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiSystem::Update(const Core::FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo)
    {
        for (const auto& element: m_Elements) { element->Draw(frame_info, ubo); }
    }

    void ImGuiSystem::Render(const Core::FrameInfo &frame_info) const
    {
        ImGui::Render();
        ImDrawData* drawdata = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawdata, frame_info.m_CommandBuffer);
    }

    /*void ImGuiSystem::RunExample()
    {
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can
        // browse its code to learn more about Dear ImGui!).
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named
        // window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", reinterpret_cast<float*>(&clear_color)); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true
                // when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a
            // closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me")) show_another_window = false;
            ImGui::End();
        }
    }*/
}
