#pragma once

#include <imgui.h>

#include "ImGuiElementDemo.h"

namespace Liara::Core
{
    struct FrameInfo;
}
namespace Liara::Graphics::Ubo
{
    struct GlobalUbo;
}

namespace Liara::Core::ImGuiElements
{
    class Example final : public Demo
    {
    public:
        Example() = default;
        ~Example() override = default;

        void Draw(const FrameInfo&, Graphics::Ubo::GlobalUbo&) override {
            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can
            // browse its code to learn more about Dear ImGui!).
            if (m_show_demo_window) ImGui::ShowDemoWindow(&m_show_demo_window);

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named
            // window.
            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text.");  // Display some text (you can use a format strings too)
                ImGui::Checkbox("Demo Window", &m_show_demo_window);  // Edit bools storing our window open/close state
                ImGui::Checkbox("Another Window", &m_show_another_window);

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::ColorEdit3("clear color",
                                  reinterpret_cast<float*>(&m_clear_color));  // Edit 3 floats representing a color

                if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true
                                              // when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                            1000.0f / ImGui::GetIO().Framerate,
                            ImGui::GetIO().Framerate);
                ImGui::End();
            }

            // 3. Show another simple window.
            if (m_show_another_window) {
                ImGui::Begin("Another Window",
                             &m_show_another_window);  // Pass a pointer to our bool variable (the window will have a
                // closing button that will clear the bool when clicked)
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me")) m_show_another_window = false;
                ImGui::End();
            }
        }

    protected:
        bool m_show_another_window = false;
        ImVec4 m_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    };
}
