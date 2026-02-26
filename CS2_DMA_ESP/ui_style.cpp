#include "ui_style.h"
#include "globals.h"
#include "config.h"
#include "features/esp.h"
#include "features/aimbot.h"
#include "features/makcu.h"

#include <imgui.h>

// ============================================================================
// Clean dark theme
// ============================================================================

void ui::ApplyStyle()
{
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowPadding    = ImVec2(10, 10);
    s.FramePadding     = ImVec2(8, 4);
    s.ItemSpacing      = ImVec2(8, 6);
    s.ItemInnerSpacing = ImVec2(6, 4);
    s.ScrollbarSize    = 12.0f;
    s.GrabMinSize      = 10.0f;

    s.WindowRounding    = 6.0f;
    s.FrameRounding     = 4.0f;
    s.GrabRounding      = 3.0f;
    s.ScrollbarRounding = 4.0f;
    s.TabRounding       = 4.0f;
    s.ChildRounding     = 4.0f;
    s.PopupRounding     = 4.0f;

    s.WindowBorderSize = 1.0f;
    s.FrameBorderSize  = 0.0f;
    s.TabBorderSize    = 0.0f;

    ImVec4* c = s.Colors;

    c[ImGuiCol_WindowBg]        = ImVec4(0.08f, 0.08f, 0.10f, 0.95f);
    c[ImGuiCol_ChildBg]         = ImVec4(0.10f, 0.10f, 0.12f, 0.60f);
    c[ImGuiCol_PopupBg]         = ImVec4(0.10f, 0.10f, 0.12f, 0.95f);

    c[ImGuiCol_Border]          = ImVec4(0.30f, 0.30f, 0.35f, 0.50f);
    c[ImGuiCol_BorderShadow]    = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    c[ImGuiCol_FrameBg]         = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
    c[ImGuiCol_FrameBgHovered]  = ImVec4(0.22f, 0.22f, 0.28f, 1.00f);
    c[ImGuiCol_FrameBgActive]   = ImVec4(0.30f, 0.30f, 0.38f, 1.00f);

    c[ImGuiCol_TitleBg]         = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    c[ImGuiCol_TitleBgActive]   = ImVec4(0.12f, 0.12f, 0.16f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.08f, 0.10f, 0.75f);

    c[ImGuiCol_Tab]                = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
    c[ImGuiCol_TabHovered]         = ImVec4(0.35f, 0.35f, 0.50f, 0.80f);
    c[ImGuiCol_TabSelected]        = ImVec4(0.25f, 0.25f, 0.38f, 1.00f);
    c[ImGuiCol_TabSelectedOverline] = ImVec4(0.45f, 0.45f, 0.70f, 1.00f);
    c[ImGuiCol_TabDimmed]          = ImVec4(0.10f, 0.10f, 0.14f, 1.00f);
    c[ImGuiCol_TabDimmedSelected]  = ImVec4(0.18f, 0.18f, 0.28f, 1.00f);

    c[ImGuiCol_Button]          = ImVec4(0.18f, 0.18f, 0.25f, 1.00f);
    c[ImGuiCol_ButtonHovered]   = ImVec4(0.30f, 0.30f, 0.45f, 1.00f);
    c[ImGuiCol_ButtonActive]    = ImVec4(0.38f, 0.38f, 0.55f, 1.00f);

    c[ImGuiCol_Header]          = ImVec4(0.18f, 0.18f, 0.25f, 0.70f);
    c[ImGuiCol_HeaderHovered]   = ImVec4(0.30f, 0.30f, 0.45f, 0.80f);
    c[ImGuiCol_HeaderActive]    = ImVec4(0.38f, 0.38f, 0.55f, 1.00f);

    c[ImGuiCol_CheckMark]       = ImVec4(0.55f, 0.55f, 0.90f, 1.00f);
    c[ImGuiCol_SliderGrab]      = ImVec4(0.45f, 0.45f, 0.75f, 1.00f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.55f, 0.90f, 1.00f);

    c[ImGuiCol_ScrollbarBg]     = ImVec4(0.08f, 0.08f, 0.10f, 0.50f);
    c[ImGuiCol_ScrollbarGrab]   = ImVec4(0.25f, 0.25f, 0.35f, 0.80f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.50f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.45f, 0.45f, 0.65f, 1.00f);

    c[ImGuiCol_Separator]       = ImVec4(0.25f, 0.25f, 0.35f, 0.40f);
    c[ImGuiCol_SeparatorHovered] = ImVec4(0.40f, 0.40f, 0.60f, 0.70f);
    c[ImGuiCol_SeparatorActive]  = ImVec4(0.50f, 0.50f, 0.75f, 1.00f);

    c[ImGuiCol_Text]            = ImVec4(0.92f, 0.92f, 0.94f, 1.00f);
    c[ImGuiCol_TextDisabled]    = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);

    c[ImGuiCol_ResizeGrip]      = ImVec4(0.25f, 0.25f, 0.35f, 0.25f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.40f, 0.60f, 0.65f);
    c[ImGuiCol_ResizeGripActive]  = ImVec4(0.50f, 0.50f, 0.75f, 0.90f);
}

// ============================================================================
// Menu rendering
// ============================================================================

void ui::RenderMenu()
{
    if (!g::menuOpen) return;

    ImGui::SetNextWindowSize(ImVec2(420, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("CS2 DMA ESP##main", &g::menuOpen, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("##tabs")) {

        // ===================== ESP Tab =====================
        if (ImGui::BeginTabItem("ESP")) {
            ImGui::Spacing();
            ImGui::Checkbox("Enabled##esp", &g::espEnabled);
            ImGui::Separator();

            if (g::espEnabled) {
                ImGui::BeginChild("##espchild", ImVec2(0, 0), ImGuiChildFlags_Borders);

                ImGui::Checkbox("Box",       &g::espBox);
                ImGui::SameLine(200);
                if (g::espBox) ImGui::ColorEdit4("##boxcol", g::espBoxColor, ImGuiColorEditFlags_NoInputs);

                ImGui::Checkbox("Health Bar", &g::espHealth);

                ImGui::Checkbox("Name",      &g::espName);
                ImGui::SameLine(200);
                if (g::espName) ImGui::ColorEdit4("##namecol", g::espNameColor, ImGuiColorEditFlags_NoInputs);

                ImGui::Checkbox("Skeleton",  &g::espSkeleton);
                ImGui::SameLine(200);
                if (g::espSkeleton) ImGui::ColorEdit4("##skelcol", g::espSkeletonColor, ImGuiColorEditFlags_NoInputs);

                ImGui::Checkbox("Snap Lines", &g::espSnaplines);
                if (g::espSnaplines) {
                    ImGui::SameLine(200);
                    ImGui::ColorEdit4("##snapcol", g::espSnaplineColor, ImGuiColorEditFlags_NoInputs);
                    ImGui::Checkbox("  From Top", &g::espSnaplineFromTop);
                }

                ImGui::EndChild();
            }
            ImGui::EndTabItem();
        }

        // ===================== Radar Tab =====================
        if (ImGui::BeginTabItem("Radar")) {
            ImGui::Spacing();
            ImGui::Checkbox("Enabled##radar", &g::radarEnabled);
            ImGui::Separator();

            if (g::radarEnabled) {
                ImGui::BeginChild("##radarchild", ImVec2(0, 0), ImGuiChildFlags_Borders);

                ImGui::SliderFloat("Range",    &g::radarRange,   500.0f, 5000.0f, "%.0f");
                ImGui::SliderFloat("Size",     &g::radarSize,    100.0f, 400.0f,  "%.0f");
                ImGui::SliderFloat("Dot Size", &g::radarDotSize, 2.0f,   8.0f,    "%.1f");

                ImGui::Spacing();
                ImGui::ColorEdit4("Background", g::radarBgColor);
                ImGui::ColorEdit4("Dot Color",  g::radarDotColor);

                ImGui::EndChild();
            }
            ImGui::EndTabItem();
        }

        // ===================== Aimbot Tab =====================
        if (ImGui::BeginTabItem("Aimbot")) {
            ImGui::Spacing();
            ImGui::Checkbox("Enabled##aimbot", &g::aimbotEnabled);
            ImGui::Separator();

            if (g::aimbotEnabled) {
                ImGui::BeginChild("##aimbotchild", ImVec2(0, 0), ImGuiChildFlags_Borders);

                ImGui::SliderFloat("FOV",    &g::aimbotFov,    1.0f, 30.0f, "%.1f deg");
                ImGui::SliderFloat("Smooth", &g::aimbotSmooth, 0.5f, 20.0f, "%.1f");

                const char* boneNames[] = { "Head (6)", "Neck (5)", "Chest (4)", "Spine (2)" };
                const int   boneValues[] = { 6, 5, 4, 2 };
                int currentBoneIdx = 0;
                for (int i = 0; i < 4; i++) {
                    if (boneValues[i] == g::aimbotBone) { currentBoneIdx = i; break; }
                }
                if (ImGui::Combo("Bone", &currentBoneIdx, boneNames, 4))
                    g::aimbotBone = boneValues[currentBoneIdx];

                ImGui::Spacing();

                // Aim Key selector — mouse only
                const char* aimKeyNames[] = {
                    "RMB (Game)", "LMB (Game)", "Mouse4", "Mouse5"
                };
                ImGui::Combo("Aim Key", &g::aimbotKey, aimKeyNames, 4);
                if (g::aimbotKey >= 2)
                    ImGui::TextDisabled("Mouse4/5: requires USB passthrough");

                ImGui::Spacing();
                ImGui::Checkbox("Vis Check",       &g::aimbotVisCheck);
                ImGui::Checkbox("Draw FOV Circle", &g::aimbotDrawFov);

                if (g::aimbotDrawFov)
                    ImGui::ColorEdit4("FOV Color", g::fovCircleColor, ImGuiColorEditFlags_NoInputs);

                ImGui::Spacing();
                ImGui::Checkbox("Instant Lock", &g::aimbotInstantLock);
                if (g::aimbotInstantLock)
                    ImGui::SliderInt("Lock Frames", &g::aimbotInstantFrames, 1, 10);

                ImGui::Spacing();
                ImGui::Separator();
                if (makcu::IsConnected())
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "MAKCU: %s", makcu::GetConnectedPort());
                else
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "MAKCU: Disconnected");

                ImGui::EndChild();
            }
            ImGui::EndTabItem();
        }

        // ===================== Config Tab =====================
        if (ImGui::BeginTabItem("Config")) {
            ImGui::Spacing();

            ImGui::BeginChild("##configchild", ImVec2(0, 0), ImGuiChildFlags_Borders);

            ImGui::Text("Configuration");
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Save Config", ImVec2(180, 30))) {
                config::Save();
            }
            ImGui::SameLine();
            if (ImGui::Button("Load Config", ImVec2(180, 30))) {
                config::Load();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Screen Resolution");
            ImGui::SliderInt("Width",  &g::screenWidth,  800, 3840);
            ImGui::SliderInt("Height", &g::screenHeight, 600, 2160);

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // --- Status bar ---
    ImGui::Separator();
    float sensitivity = esp::GetSensitivity();
    ImGui::TextDisabled("Sens: %.2f | INSERT = menu | END = exit", sensitivity);

    ImGui::End();
}
