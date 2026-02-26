#include "esp.h"
#include "../globals.h"
#include "../sdk/structs.h"
#include "../sdk/offsets.h"
#include <DMALibrary/Memory/Memory.h>
#include <imgui.h>
#include <cstring>
#include <Windows.h>

// ============================================================================
// Dati giocatore cachati (riempiti ogni frame via DMA scatter reads)
// ============================================================================

static esp::PlayerData s_players[64];
static int        s_localTeam = 0;
static uintptr_t  s_localPawn = 0;
static view_matrix_t s_viewMatrix = {};
static Vector3    s_localPos = {};
static Vector3    s_viewAngles = {};
static int        s_localControllerIndex = -1;
static float      s_sensitivity = 1.0f;
static bool       s_aimbotKeyHeld = false;

// ============================================================================
// Accessors for aimbot
// ============================================================================

const esp::PlayerData* esp::GetPlayers()              { return s_players; }
int         esp::GetLocalTeam()                       { return s_localTeam; }
Vector3     esp::GetLocalPos()                        { return s_localPos; }
Vector3     esp::GetViewAngles()                      { return s_viewAngles; }
const view_matrix_t& esp::GetViewMatrix()             { return s_viewMatrix; }
int         esp::GetLocalControllerIndex()            { return s_localControllerIndex; }
float       esp::GetSensitivity()                     { return s_sensitivity; }
bool        esp::IsAimbotKeyHeld()                    { return s_aimbotKeyHeld; }

// ============================================================================
// Skeleton bone pairs (identico all'internal)
// ============================================================================

struct BonePair { int from, to; };

static const BonePair skeletonPairs[] = {
    { 6,  5 }, { 5,  4 }, { 4,  2 }, { 2,  0 },   // spine + head
    { 5,  8 }, { 8,  9 }, { 9,  10 },               // left arm
    { 5,  13 }, { 13, 14 }, { 14, 15 },              // right arm
    { 0,  22 }, { 22, 23 }, { 23, 24 },              // left leg
    { 0,  25 }, { 25, 26 }, { 26, 27 },              // right leg
};
static constexpr int skeletonPairCount = sizeof(skeletonPairs) / sizeof(skeletonPairs[0]);

// Bone indices che servono per lo skeleton + aimbot
static const int neededBones[] = { 0, 2, 4, 5, 6, 8, 9, 10, 13, 14, 15, 22, 23, 24, 25, 26, 27 };
static constexpr int neededBoneCount = sizeof(neededBones) / sizeof(neededBones[0]);

// ============================================================================
// UpdateData — Legge tutto il game state via DMA scatter reads
// ============================================================================

// Increased to ~10s at 60fps to survive DMA hiccups and round transitions
static constexpr int MAX_STALE_FRAMES = 600;

void esp::UpdateData()
{
    if (!g::clientBase) return;

    // NOTE: stale counters are NOT incremented here anymore.
    // They are only incremented AFTER a successful entity list read,
    // for players that were not refreshed in that update cycle.
    // This prevents mass-invalidation during DMA read failures.

    s_localControllerIndex = -1;
    s_aimbotKeyHeld = false;

    // ------------------------------------------------------------------
    // Fase 1: Leggi puntatori base
    // ------------------------------------------------------------------
    uintptr_t entityList = 0;
    s_localPawn = 0;
    uintptr_t localController = 0;
    uintptr_t sensPtr = 0;
    uint32_t  buttonState = 0;

    auto handle = mem.CreateScatterHandle();
    mem.AddScatterReadRequest(handle, g::clientBase + offsets::dwEntityList, &entityList, sizeof(uintptr_t));
    mem.AddScatterReadRequest(handle, g::clientBase + offsets::dwLocalPlayerPawn, &s_localPawn, sizeof(uintptr_t));
    mem.AddScatterReadRequest(handle, g::clientBase + offsets::dwViewMatrix, &s_viewMatrix, sizeof(view_matrix_t));
    mem.AddScatterReadRequest(handle, g::clientBase + offsets::dwViewAngles, &s_viewAngles, sizeof(Vector3));
    mem.AddScatterReadRequest(handle, g::clientBase + offsets::dwLocalPlayerController, &localController, sizeof(uintptr_t));
    mem.AddScatterReadRequest(handle, g::clientBase + offsets::dwSensitivity, &sensPtr, sizeof(uintptr_t));

    // Read game button state for game-mode keys (0=RMB, 1=LMB)
    if (g::aimbotKey <= 1) {
        std::ptrdiff_t buttonOffset = (g::aimbotKey == 1) ? buttons::attack : buttons::attack2;
        mem.AddScatterReadRequest(handle, g::clientBase + buttonOffset, &buttonState, sizeof(uint32_t));
    }
    mem.ExecuteReadScatter(handle);

    // Determine aimbot key state
    if (g::aimbotKey <= 1) {
        // Game memory button: bit 0 = pressed
        s_aimbotKeyHeld = (buttonState & 1) != 0;
    } else {
        // Mouse4/Mouse5 via GetAsyncKeyState (requires mouse on overlay PC / USB passthrough)
        int vk = (g::aimbotKey == 2) ? VK_XBUTTON1 : VK_XBUTTON2;
        s_aimbotKeyHeld = (GetAsyncKeyState(vk) & 0x8000) != 0;
    }

    if (!entityList || !s_localPawn) {
        mem.CloseScatterHandle(handle);
        // Do NOT increment stale frames — DMA failure, not player disappearance
        return;
    }

    // ------------------------------------------------------------------
    // Fase 2: Leggi localTeam, localPos, listEntry, sensitivity
    // ------------------------------------------------------------------
    uintptr_t listEntry = 0;
    s_localTeam = 0;
    s_localPos = {};
    float sensValue = 1.0f;

    mem.AddScatterReadRequest(handle, s_localPawn + schemas::C_BaseEntity::m_iTeamNum, &s_localTeam, sizeof(int));
    mem.AddScatterReadRequest(handle, s_localPawn + schemas::C_BasePlayerPawn::m_vOldOrigin, &s_localPos, sizeof(Vector3));
    mem.AddScatterReadRequest(handle, entityList + 0x10, &listEntry, sizeof(uintptr_t));
    if (sensPtr)
        mem.AddScatterReadRequest(handle, sensPtr + offsets::dwSensitivity_sensitivity, &sensValue, sizeof(float));
    mem.ExecuteReadScatter(handle);

    if (sensValue > 0.0f && sensValue < 100.0f)
        s_sensitivity = sensValue;

    if (!listEntry) {
        mem.CloseScatterHandle(handle);
        return;
    }

    // ------------------------------------------------------------------
    // Fase 3: Leggi tutti i 64 controller pointers (scatter batch)
    // ------------------------------------------------------------------
    uintptr_t controllers[64] = {};
    for (int i = 0; i < 64; i++) {
        int idx = i + 1; // player indices 1-64
        mem.AddScatterReadRequest(handle, listEntry + 0x70 * (idx & 0x1FF), &controllers[i], sizeof(uintptr_t));
    }
    mem.ExecuteReadScatter(handle);

    // Find local controller index (for vis check)
    if (localController) {
        for (int i = 0; i < 64; i++) {
            if (controllers[i] == localController) {
                s_localControllerIndex = i + 1; // 1-based
                break;
            }
        }
    }

    // ------------------------------------------------------------------
    // Fase 4: Leggi pawn handles + nomi da ogni controller valido
    // ------------------------------------------------------------------
    uint32_t pawnHandles[64] = {};
    char     names[64][128] = {};

    for (int i = 0; i < 64; i++) {
        if (!controllers[i]) continue;
        mem.AddScatterReadRequest(handle, controllers[i] + schemas::CCSPlayerController::m_hPlayerPawn, &pawnHandles[i], sizeof(uint32_t));
        mem.AddScatterReadRequest(handle, controllers[i] + schemas::CBasePlayerController::m_iszPlayerName, &names[i], 128);
    }
    mem.ExecuteReadScatter(handle);

    // ------------------------------------------------------------------
    // Fase 5a: Risolvi pawn entries (secondo livello entity list)
    // ------------------------------------------------------------------
    uintptr_t pawnEntries[64] = {};
    for (int i = 0; i < 64; i++) {
        if (!pawnHandles[i]) continue;
        uint32_t block = (pawnHandles[i] & 0x7FFF) >> 9;
        mem.AddScatterReadRequest(handle, entityList + 0x10 + 8 * block, &pawnEntries[i], sizeof(uintptr_t));
    }
    mem.ExecuteReadScatter(handle);

    // ------------------------------------------------------------------
    // Fase 5b: Leggi pawn pointers
    // ------------------------------------------------------------------
    uintptr_t pawns[64] = {};
    for (int i = 0; i < 64; i++) {
        if (!pawnEntries[i] || !pawnHandles[i]) continue;
        uint32_t slot = pawnHandles[i] & 0x1FF;
        mem.AddScatterReadRequest(handle, pawnEntries[i] + 0x70 * slot, &pawns[i], sizeof(uintptr_t));
    }
    mem.ExecuteReadScatter(handle);

    // ------------------------------------------------------------------
    // Fase 6: Leggi dati pawn (health, team, lifeState, position, gameSceneNode, spottedMask)
    // ------------------------------------------------------------------
    int       healths[64] = {};
    int       teams[64] = {};
    uint8_t   lifeStates[64] = {};
    Vector3   positions[64] = {};
    uintptr_t sceneNodes[64] = {};
    uint32_t  spottedMasks[64][2] = {};

    for (int i = 0; i < 64; i++) {
        if (!pawns[i] || pawns[i] == s_localPawn) continue;
        mem.AddScatterReadRequest(handle, pawns[i] + schemas::C_BaseEntity::m_iHealth, &healths[i], sizeof(int));
        mem.AddScatterReadRequest(handle, pawns[i] + schemas::C_BaseEntity::m_iTeamNum, &teams[i], sizeof(int));
        mem.AddScatterReadRequest(handle, pawns[i] + schemas::C_BaseEntity::m_lifeState, &lifeStates[i], sizeof(uint8_t));
        mem.AddScatterReadRequest(handle, pawns[i] + schemas::C_BasePlayerPawn::m_vOldOrigin, &positions[i], sizeof(Vector3));
        mem.AddScatterReadRequest(handle, pawns[i] + schemas::C_BaseEntity::m_pGameSceneNode, &sceneNodes[i], sizeof(uintptr_t));
        mem.AddScatterReadRequest(handle,
            pawns[i] + schemas::C_CSPlayerPawn::m_entitySpottedState + schemas::EntitySpottedState_t::m_bSpottedByMask,
            reinterpret_cast<void*>(&spottedMasks[i]), sizeof(uint32_t) * 2);
    }
    mem.ExecuteReadScatter(handle);

    // ------------------------------------------------------------------
    // Fase 7: Leggi bone array pointers
    // ------------------------------------------------------------------
    uintptr_t boneArrays[64] = {};
    for (int i = 0; i < 64; i++) {
        if (!sceneNodes[i]) continue;
        mem.AddScatterReadRequest(handle, sceneNodes[i] + schemas::CSkeletonInstance::m_modelState + 0x80, &boneArrays[i], sizeof(uintptr_t));
    }
    mem.ExecuteReadScatter(handle);

    // ------------------------------------------------------------------
    // Fase 8: Leggi le posizioni delle bones
    // ------------------------------------------------------------------
    Vector3 allBones[64][28] = {};
    bool    hasBoneData[64] = {};

    for (int i = 0; i < 64; i++) {
        if (!boneArrays[i]) continue;
        hasBoneData[i] = true;
        for (int b = 0; b < neededBoneCount; b++) {
            int boneIdx = neededBones[b];
            mem.AddScatterReadRequest(handle, boneArrays[i] + boneIdx * 32, &allBones[i][boneIdx], sizeof(Vector3));
        }
    }
    mem.ExecuteReadScatter(handle);

    mem.CloseScatterHandle(handle);

    // ------------------------------------------------------------------
    // Salva i risultati nel cache s_players[]
    // Track which players got refreshed so we only age the rest.
    // ------------------------------------------------------------------
    bool refreshed[64] = {};

    for (int i = 0; i < 64; i++) {
        if (!pawns[i] || pawns[i] == s_localPawn) {
            continue;
        }

        refreshed[i] = true;

        // Dead, disconnected, or same team — invalidate immediately
        if (healths[i] <= 0 || lifeStates[i] != 0 || teams[i] == s_localTeam) {
            s_players[i].valid = false;
            s_players[i].staleFrames = 0;
            continue;
        }

        esp::PlayerData& p = s_players[i];
        p.valid       = true;
        p.staleFrames = 0; // fresh data received
        p.health      = healths[i];
        p.team        = teams[i];
        p.position    = positions[i];
        p.hasBones    = hasBoneData[i];
        p.spottedMask = (uint64_t)spottedMasks[i][1] << 32 | (uint64_t)spottedMasks[i][0];

        memcpy(p.name, names[i], 128);
        p.name[127] = '\0';

        if (hasBoneData[i])
            memcpy(p.bones, allBones[i], sizeof(p.bones));
    }

    // Increment stale frames ONLY for players that were valid but not refreshed
    // in this successful update cycle.
    for (int i = 0; i < 64; i++) {
        if (s_players[i].valid && !refreshed[i]) {
            s_players[i].staleFrames++;
            if (s_players[i].staleFrames > MAX_STALE_FRAMES) {
                s_players[i].valid = false;
                s_players[i].staleFrames = 0;
            }
        }
    }
}

// ============================================================================
// Helpers di rendering
// ============================================================================

static ImU32 ColorToImU32(const float c[4]) {
    return IM_COL32(
        (int)(c[0] * 255.0f), (int)(c[1] * 255.0f),
        (int)(c[2] * 255.0f), (int)(c[3] * 255.0f));
}

// Corner-style box: draws only the corners of the rectangle
static void DrawCornerBox(ImDrawList* dl, float x, float y, float w, float h,
                          ImU32 color, ImU32 outline, float cornerLen, float thickness)
{
    float cl = cornerLen;
    if (cl > w * 0.5f) cl = w * 0.5f;
    if (cl > h * 0.5f) cl = h * 0.5f;

    float ot = thickness + 2.0f; // outline thickness

    // --- Outline (black, drawn first) ---
    // Top-left
    dl->AddLine(ImVec2(x - 1, y), ImVec2(x + cl, y), outline, ot);
    dl->AddLine(ImVec2(x, y - 1), ImVec2(x, y + cl), outline, ot);
    // Top-right
    dl->AddLine(ImVec2(x + w - cl, y), ImVec2(x + w + 1, y), outline, ot);
    dl->AddLine(ImVec2(x + w, y - 1), ImVec2(x + w, y + cl), outline, ot);
    // Bottom-left
    dl->AddLine(ImVec2(x - 1, y + h), ImVec2(x + cl, y + h), outline, ot);
    dl->AddLine(ImVec2(x, y + h - cl), ImVec2(x, y + h + 1), outline, ot);
    // Bottom-right
    dl->AddLine(ImVec2(x + w - cl, y + h), ImVec2(x + w + 1, y + h), outline, ot);
    dl->AddLine(ImVec2(x + w, y + h - cl), ImVec2(x + w, y + h + 1), outline, ot);

    // --- Color lines ---
    // Top-left
    dl->AddLine(ImVec2(x, y), ImVec2(x + cl, y), color, thickness);
    dl->AddLine(ImVec2(x, y), ImVec2(x, y + cl), color, thickness);
    // Top-right
    dl->AddLine(ImVec2(x + w - cl, y), ImVec2(x + w, y), color, thickness);
    dl->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + cl), color, thickness);
    // Bottom-left
    dl->AddLine(ImVec2(x, y + h), ImVec2(x + cl, y + h), color, thickness);
    dl->AddLine(ImVec2(x, y + h - cl), ImVec2(x, y + h), color, thickness);
    // Bottom-right
    dl->AddLine(ImVec2(x + w - cl, y + h), ImVec2(x + w, y + h), color, thickness);
    dl->AddLine(ImVec2(x + w, y + h - cl), ImVec2(x + w, y + h), color, thickness);
}

// ============================================================================
// Draw — Disegna ESP overlay usando i dati cachati
// ============================================================================

void esp::Draw()
{
    if (!g::espEnabled && !g::radarEnabled) return;

    float screenW = (float)g::screenWidth;
    float screenH = (float)g::screenHeight;
    if (screenW <= 0 || screenH <= 0) return;

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    ImU32 boxCol  = ColorToImU32(g::espBoxColor);
    ImU32 nameCol = ColorToImU32(g::espNameColor);
    ImU32 skelCol = ColorToImU32(g::espSkeletonColor);
    ImU32 snapCol = ColorToImU32(g::espSnaplineColor);

    // === ESP ===
    if (g::espEnabled) {
        for (int i = 0; i < 64; i++) {
            const esp::PlayerData& p = s_players[i];
            if (!p.valid) continue;

            // Calculate box from real bones when available
            Vector3 feetPos, headPos;

            if (p.hasBones && !(p.bones[6].x == 0 && p.bones[6].y == 0 && p.bones[6].z == 0)) {
                headPos = p.bones[6];
                headPos.z += 8.0f;

                Vector3 leftFoot  = p.bones[24];
                Vector3 rightFoot = p.bones[27];
                bool hasLeft  = !(leftFoot.x == 0 && leftFoot.y == 0 && leftFoot.z == 0);
                bool hasRight = !(rightFoot.x == 0 && rightFoot.y == 0 && rightFoot.z == 0);

                if (hasLeft && hasRight)
                    feetPos = (leftFoot.z < rightFoot.z) ? leftFoot : rightFoot;
                else if (hasLeft)
                    feetPos = leftFoot;
                else if (hasRight)
                    feetPos = rightFoot;
                else
                    feetPos = p.position;
                feetPos.z -= 4.0f;
            } else {
                feetPos = p.position;
                headPos = feetPos;
                headPos.z += 72.0f;
            }

            ScreenPos screenFeet = WorldToScreen(feetPos, s_viewMatrix, screenW, screenH);
            ScreenPos screenHead = WorldToScreen(headPos, s_viewMatrix, screenW, screenH);
            if (!screenFeet.onScreen || !screenHead.onScreen) continue;

            float boxHeight = screenFeet.y - screenHead.y;
            if (boxHeight < 4.0f) continue;
            float boxWidth  = boxHeight * 0.5f;
            float boxLeft   = screenHead.x - boxWidth * 0.5f;
            float boxTop    = screenHead.y;

            // --- Corner Box ---
            if (g::espBox) {
                float cornerLen = boxHeight * 0.25f;
                if (cornerLen < 5.0f) cornerLen = 5.0f;
                DrawCornerBox(drawList, boxLeft, boxTop, boxWidth, boxHeight,
                              boxCol, IM_COL32(0, 0, 0, 220), cornerLen, 2.0f);
            }

            // --- Health bar (outlined, gradient green-yellow-red) ---
            if (g::espHealth) {
                float healthFrac = (float)p.health / 100.0f;
                if (healthFrac > 1.0f) healthFrac = 1.0f;

                float barWidth  = 3.0f;
                float barLeft   = boxLeft - barWidth - 4.0f;
                float barHeight = boxHeight * healthFrac;

                // Health color: green(100) -> yellow(50) -> red(0)
                int r, green;
                if (healthFrac > 0.5f) {
                    float t = (healthFrac - 0.5f) * 2.0f;
                    r = (int)(255.0f * (1.0f - t));
                    green = 255;
                } else {
                    float t = healthFrac * 2.0f;
                    r = 255;
                    green = (int)(255.0f * t);
                }

                // Outline
                drawList->AddRectFilled(
                    ImVec2(barLeft - 1, boxTop - 1),
                    ImVec2(barLeft + barWidth + 1, boxTop + boxHeight + 1),
                    IM_COL32(0, 0, 0, 200));
                // Background
                drawList->AddRectFilled(
                    ImVec2(barLeft, boxTop),
                    ImVec2(barLeft + barWidth, boxTop + boxHeight),
                    IM_COL32(20, 20, 20, 180));
                // Health fill
                drawList->AddRectFilled(
                    ImVec2(barLeft, boxTop + boxHeight - barHeight),
                    ImVec2(barLeft + barWidth, boxTop + boxHeight),
                    IM_COL32(r, green, 0, 255));

                // HP text if damaged
                if (p.health < 100) {
                    char hpText[8];
                    snprintf(hpText, sizeof(hpText), "%d", p.health);
                    ImVec2 hpSize = ImGui::CalcTextSize(hpText);
                    float hpX = barLeft + barWidth * 0.5f - hpSize.x * 0.5f;
                    float hpY = boxTop + boxHeight - barHeight - hpSize.y - 2.0f;
                    if (hpY < boxTop - hpSize.y) hpY = boxTop - hpSize.y;
                    drawList->AddText(ImVec2(hpX + 1, hpY + 1), IM_COL32(0, 0, 0, 200), hpText);
                    drawList->AddText(ImVec2(hpX, hpY), IM_COL32(255, 255, 255, 230), hpText);
                }
            }

            // --- Name (centered, with shadow) ---
            if (g::espName && p.name[0] != '\0') {
                ImVec2 textSize = ImGui::CalcTextSize(p.name);
                float textX = screenHead.x - textSize.x * 0.5f;
                float textY = boxTop - textSize.y - 4.0f;

                // Double shadow for better readability
                drawList->AddText(ImVec2(textX + 1, textY + 1), IM_COL32(0, 0, 0, 255), p.name);
                drawList->AddText(ImVec2(textX - 1, textY + 1), IM_COL32(0, 0, 0, 150), p.name);
                drawList->AddText(ImVec2(textX, textY), nameCol, p.name);
            }

            // --- Skeleton (outlined) ---
            if (g::espSkeleton && p.hasBones) {
                for (int b = 0; b < skeletonPairCount; b++) {
                    Vector3 boneFrom = p.bones[skeletonPairs[b].from];
                    Vector3 boneTo   = p.bones[skeletonPairs[b].to];

                    if (boneFrom.x == 0 && boneFrom.y == 0 && boneFrom.z == 0) continue;
                    if (boneTo.x == 0 && boneTo.y == 0 && boneTo.z == 0) continue;

                    ScreenPos sFrom = WorldToScreen(boneFrom, s_viewMatrix, screenW, screenH);
                    ScreenPos sTo   = WorldToScreen(boneTo,   s_viewMatrix, screenW, screenH);
                    if (!sFrom.onScreen || !sTo.onScreen) continue;

                    // Black outline
                    drawList->AddLine(
                        ImVec2(sFrom.x, sFrom.y), ImVec2(sTo.x, sTo.y),
                        IM_COL32(0, 0, 0, 180), 3.0f);
                    // Color line
                    drawList->AddLine(
                        ImVec2(sFrom.x, sFrom.y), ImVec2(sTo.x, sTo.y),
                        skelCol, 1.5f);
                }
            }

            // --- Snap lines ---
            if (g::espSnaplines) {
                float originY = g::espSnaplineFromTop ? 0.0f : screenH;
                // Outline
                drawList->AddLine(
                    ImVec2(screenW * 0.5f, originY),
                    ImVec2(screenFeet.x, screenFeet.y),
                    IM_COL32(0, 0, 0, 120), 2.5f);
                drawList->AddLine(
                    ImVec2(screenW * 0.5f, originY),
                    ImVec2(screenFeet.x, screenFeet.y),
                    snapCol, 1.0f);
            }
        }
    }

    // === Radar ===
    if (g::radarEnabled) {
        float yawRad = s_viewAngles.y * (3.14159265358979f / 180.0f);
        float cosYaw = cosf(yawRad);
        float sinYaw = sinf(yawRad);

        float radarSz = g::radarSize;
        float halfRad = radarSz * 0.5f;
        float margin  = 15.0f;

        ImVec2 radarPos(screenW - radarSz - margin, margin);
        ImVec2 radarCenter(radarPos.x + halfRad, radarPos.y + halfRad);

        ImU32 bgCol  = ColorToImU32(g::radarBgColor);
        ImU32 dotCol = ColorToImU32(g::radarDotColor);

        drawList->AddRectFilled(radarPos, ImVec2(radarPos.x + radarSz, radarPos.y + radarSz), bgCol, 6.0f);
        drawList->AddRect(radarPos, ImVec2(radarPos.x + radarSz, radarPos.y + radarSz), IM_COL32(255,255,255,40), 6.0f, 0, 1.0f);
        drawList->AddLine(ImVec2(radarCenter.x, radarPos.y), ImVec2(radarCenter.x, radarPos.y + radarSz), IM_COL32(255,255,255,25));
        drawList->AddLine(ImVec2(radarPos.x, radarCenter.y), ImVec2(radarPos.x + radarSz, radarCenter.y), IM_COL32(255,255,255,25));
        drawList->AddCircleFilled(radarCenter, 3.0f, IM_COL32(255,255,255,220));

        for (int i = 0; i < 64; i++) {
            const esp::PlayerData& p = s_players[i];
            if (!p.valid) continue;

            float dx = p.position.x - s_localPos.x;
            float dy = p.position.y - s_localPos.y;

            float relRight   =  dx * sinYaw - dy * cosYaw;
            float relForward =  dx * cosYaw + dy * sinYaw;

            float scale = halfRad / g::radarRange;
            float px = radarCenter.x + relRight   * scale;
            float py = radarCenter.y - relForward * scale;

            // Clamp dentro il radar
            if (px < radarPos.x + 3.0f) px = radarPos.x + 3.0f;
            if (px > radarPos.x + radarSz - 3.0f) px = radarPos.x + radarSz - 3.0f;
            if (py < radarPos.y + 3.0f) py = radarPos.y + 3.0f;
            if (py > radarPos.y + radarSz - 3.0f) py = radarPos.y + radarSz - 3.0f;

            drawList->AddCircleFilled(ImVec2(px, py), g::radarDotSize, dotCol);
        }
    }
}

// ============================================================================
// DrawMenu — ImGui menu per le impostazioni ESP + Radar
// ============================================================================

void esp::DrawMenu()
{
    if (!g::menuOpen) return;

    ImGui::SetNextWindowSize(ImVec2(350, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("CS2 DMA ESP", &g::menuOpen);

    if (ImGui::CollapsingHeader("ESP", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enabled", &g::espEnabled);
        ImGui::Checkbox("Box", &g::espBox);
        ImGui::Checkbox("Health Bar", &g::espHealth);
        ImGui::Checkbox("Name", &g::espName);
        ImGui::Checkbox("Skeleton", &g::espSkeleton);
        ImGui::Checkbox("Snap Lines", &g::espSnaplines);
        if (g::espSnaplines)
            ImGui::Checkbox("  From Top", &g::espSnaplineFromTop);

        ImGui::ColorEdit4("Box Color", g::espBoxColor);
        ImGui::ColorEdit4("Name Color", g::espNameColor);
        ImGui::ColorEdit4("Skeleton Color", g::espSkeletonColor);
        ImGui::ColorEdit4("Snapline Color", g::espSnaplineColor);
    }

    if (ImGui::CollapsingHeader("Radar")) {
        ImGui::Checkbox("Enabled##radar", &g::radarEnabled);
        ImGui::SliderFloat("Range", &g::radarRange, 500.0f, 5000.0f);
        ImGui::SliderFloat("Size", &g::radarSize, 100.0f, 400.0f);
        ImGui::SliderFloat("Dot Size", &g::radarDotSize, 2.0f, 8.0f);
    }

    ImGui::Separator();
    ImGui::Text("INSERT = toggle menu | END = exit");
    ImGui::Text("Sensitivity: %.2f", s_sensitivity);

    ImGui::End();
}
