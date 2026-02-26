#pragma once
#include <cstdint>

namespace g {
    // === Risoluzione del gioco sul PC di gioco (DEVE corrispondere!) ===
    inline int screenWidth  = 1920;
    inline int screenHeight = 1080;

    // === Client.dll base (impostato a runtime) ===
    inline uintptr_t clientBase = 0;

    // === ESP ===
    inline bool espEnabled      = true;
    inline bool espBox          = true;
    inline bool espHealth       = true;
    inline bool espName         = true;
    inline bool espSkeleton     = true;
    inline bool espSnaplines    = false;
    inline bool espSnaplineFromTop = false;

    inline float espBoxColor[4]      = { 1.0f, 0.20f, 0.20f, 1.0f };
    inline float espNameColor[4]     = { 1.0f, 1.0f, 1.0f, 1.0f };
    inline float espSkeletonColor[4] = { 0.0f, 1.0f, 0.5f, 1.0f };
    inline float espSnaplineColor[4] = { 1.0f, 1.0f, 0.0f, 0.80f };

    // === Radar ===
    inline bool  radarEnabled  = true;
    inline float radarRange    = 2000.0f;
    inline float radarSize     = 180.0f;
    inline float radarDotSize  = 4.0f;
    inline float radarBgColor[4]  = { 0.05f, 0.05f, 0.10f, 0.75f };
    inline float radarDotColor[4] = { 1.0f, 0.25f, 0.25f, 1.0f };

    // === Aimbot ===
    inline bool  aimbotEnabled  = false;
    inline float aimbotFov      = 5.0f;
    inline float aimbotSmooth   = 5.0f;
    inline int   aimbotBone     = 6;         // head
    inline bool  aimbotVisCheck = true;
    inline bool  aimbotDrawFov  = true;

    inline bool  aimbotInstantLock    = false;
    inline int   aimbotInstantFrames = 3;      // frames without smoothing on new target

    inline int   aimbotKey          = 0;          // 0=RMB(game), 1=LMB(game), 2=Mouse4, 3=Mouse5

    inline float fovCircleColor[4] = { 0.55f, 0.24f, 0.90f, 0.70f };

    // === Stato applicazione ===
    inline bool running    = true;
    inline bool menuOpen   = true;
}
