#include "aimbot.h"
#include "esp.h"
#include "makcu.h"
#include "../globals.h"
#include "../sdk/structs.h"
#include <imgui.h>
#include <chrono>
#include <cmath>

static auto s_lastAimTime = std::chrono::high_resolution_clock::now();
static bool s_wasAiming = false;

// Sub-pixel accumulation
static float s_accumX = 0.0f;
static float s_accumY = 0.0f;

// Instant lock tracking
static int s_lockFrames = 0; // frames since acquiring current target

void aimbot::Run()
{
    if (!g::aimbotEnabled || !makcu::IsConnected()) return;

    // Check aimbot activation key (read from game memory via DMA)
    bool keyHeld = esp::IsAimbotKeyHeld();
    if (!keyHeld) {
        s_wasAiming = false;
        s_accumX = 0.0f;
        s_accumY = 0.0f;
        s_lockFrames = 0;
        return;
    }

    // Delta time for smooth aiming
    auto now = std::chrono::high_resolution_clock::now();
    float deltaTime;

    if (!s_wasAiming) {
        deltaTime = 0.016f;
        s_wasAiming = true;
        s_lockFrames = 0;
    } else {
        deltaTime = std::chrono::duration<float>(now - s_lastAimTime).count();
    }
    s_lastAimTime = now;

    if (deltaTime > 0.1f)    deltaTime = 0.1f;
    if (deltaTime < 0.0001f) deltaTime = 0.0001f;

    // Get cached data from ESP
    const esp::PlayerData* players = esp::GetPlayers();
    Vector3 viewAngles = esp::GetViewAngles();
    Vector3 localPos   = esp::GetLocalPos();
    int     localTeam  = esp::GetLocalTeam();
    int     localCtrlIdx = esp::GetLocalControllerIndex();
    float   sensitivity  = esp::GetSensitivity();

    // Eye position (standing eye height)
    localPos.z += 64.0f;

    if (sensitivity <= 0.0f) sensitivity = 1.0f;

    // Find best target
    float bestFov = g::aimbotFov;
    Vector3 bestAngle = {};
    bool found = false;

    for (int i = 0; i < 64; i++) {
        const esp::PlayerData& p = players[i];
        if (!p.valid) continue;
        if (p.team == localTeam) continue;
        if (p.health <= 0) continue;

        // Visibility check
        if (g::aimbotVisCheck && localCtrlIdx > 0) {
            int bitIndex = localCtrlIdx - 1;
            if (bitIndex >= 0 && bitIndex < 64) {
                if (!((p.spottedMask >> bitIndex) & 1))
                    continue;
            }
        }

        // Get target bone position
        if (!p.hasBones) continue;
        Vector3 bonePos = p.bones[g::aimbotBone];
        if (bonePos.x == 0.0f && bonePos.y == 0.0f && bonePos.z == 0.0f) continue;

        Vector3 angle = CalcAngle(localPos, bonePos);
        NormalizeAngles(angle);

        float fov = GetFov(viewAngles, angle);
        if (fov < bestFov) {
            bestFov = fov;
            bestAngle = angle;
            found = true;
        }
    }

    if (!found) {
        s_lockFrames = 0;
        return;
    }

    s_lockFrames++;

    // Calculate angle delta
    Vector3 delta;
    delta.x = bestAngle.x - viewAngles.x;
    delta.y = bestAngle.y - viewAngles.y;
    delta.z = 0.0f;

    if (delta.y >  180.0f) delta.y -= 360.0f;
    if (delta.y < -180.0f) delta.y += 360.0f;

    // Smoothing
    float smoothDeltaYaw, smoothDeltaPitch;

    if (g::aimbotInstantLock && s_lockFrames <= g::aimbotInstantFrames) {
        // Instant lock: skip smoothing for first N frames on new target
        smoothDeltaYaw   = delta.y;
        smoothDeltaPitch = delta.x;
    } else {
        // Exponential smoothing (same as before)
        float speed = 20.0f / g::aimbotSmooth;
        if (speed < 0.1f) speed = 0.1f;

        float factor = 1.0f - expf(-speed * deltaTime * 60.0f);
        if (factor > 1.0f) factor = 1.0f;
        if (factor < 0.0f) factor = 0.0f;

        smoothDeltaYaw   = delta.y * factor;
        smoothDeltaPitch = delta.x * factor;
    }

    // Convert angle delta to mouse counts with sub-pixel accumulation
    float countFactor = 1.0f / (sensitivity * 0.022f);

    float rawX = -smoothDeltaYaw   * countFactor + s_accumX;
    float rawY =  smoothDeltaPitch * countFactor + s_accumY;

    int mouse_dx = (int)roundf(rawX);
    int mouse_dy = (int)roundf(rawY);

    // Accumulate the fractional remainder for next frame
    s_accumX = rawX - (float)mouse_dx;
    s_accumY = rawY - (float)mouse_dy;

    // Send to MAKCU
    makcu::MoveMouse(mouse_dx, mouse_dy);
}

void aimbot::DrawFovCircle()
{
    if (!g::aimbotEnabled || !g::aimbotDrawFov) return;

    float screenW = (float)g::screenWidth;
    float screenH = (float)g::screenHeight;
    if (screenW <= 0 || screenH <= 0) return;

    const view_matrix_t& vm = esp::GetViewMatrix();
    float fovScale = vm[0][0];
    if (fovScale <= 0.0f) return;

    float fovRad = g::aimbotFov * (3.14159265358979f / 180.0f);
    float radius = tanf(fovRad) * fovScale * (screenW * 0.5f);

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImU32 col = IM_COL32(
        (int)(g::fovCircleColor[0] * 255.0f),
        (int)(g::fovCircleColor[1] * 255.0f),
        (int)(g::fovCircleColor[2] * 255.0f),
        (int)(g::fovCircleColor[3] * 255.0f));

    drawList->AddCircle(
        ImVec2(screenW * 0.5f, screenH * 0.5f),
        radius, col, 64, 1.5f);
}

void aimbot::DrawMenu()
{
    if (ImGui::CollapsingHeader("Aimbot")) {
        ImGui::Checkbox("Enabled##aimbot", &g::aimbotEnabled);

        if (g::aimbotEnabled) {
            ImGui::SliderFloat("FOV", &g::aimbotFov, 1.0f, 30.0f, "%.1f");
            ImGui::SliderFloat("Smooth", &g::aimbotSmooth, 0.5f, 20.0f, "%.1f");

            const char* boneNames[] = { "Head (6)", "Neck (5)", "Chest (4)", "Spine (2)" };
            const int   boneValues[] = { 6, 5, 4, 2 };
            int currentBoneIdx = 0;
            for (int i = 0; i < 4; i++) {
                if (boneValues[i] == g::aimbotBone) { currentBoneIdx = i; break; }
            }
            if (ImGui::Combo("Bone", &currentBoneIdx, boneNames, 4))
                g::aimbotBone = boneValues[currentBoneIdx];

            ImGui::Checkbox("Vis Check", &g::aimbotVisCheck);
            ImGui::Checkbox("Draw FOV Circle", &g::aimbotDrawFov);
            ImGui::Checkbox("Instant Lock", &g::aimbotInstantLock);

            if (g::aimbotInstantLock)
                ImGui::SliderInt("Lock Frames", &g::aimbotInstantFrames, 1, 10);

            if (g::aimbotDrawFov)
                ImGui::ColorEdit4("FOV Color", g::fovCircleColor);

            // MAKCU status
            if (makcu::IsConnected())
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "MAKCU: Connected (%s)", makcu::GetConnectedPort());
            else
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "MAKCU: Disconnected");
        }
    }
}
