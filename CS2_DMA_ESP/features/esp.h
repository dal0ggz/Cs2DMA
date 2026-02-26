#pragma once
#include "../sdk/structs.h"
#include <cstdint>

namespace esp {

    struct PlayerData {
        bool     valid = false;
        int      health = 0;
        int      team = 0;
        Vector3  position;
        char     name[128] = {};
        Vector3  bones[28] = {};
        bool     hasBones = false;
        uint64_t spottedMask = 0;
        int      staleFrames = 0;
    };

    // Legge tutti i dati dal gioco via DMA (scatter reads)
    void UpdateData();

    // Disegna ESP + Radar usando i dati letti da UpdateData()
    void Draw();

    // Disegna il menu ImGui per le impostazioni
    void DrawMenu();

    // Accessors for aimbot
    const PlayerData* GetPlayers();
    int         GetLocalTeam();
    Vector3     GetLocalPos();
    Vector3     GetViewAngles();
    const view_matrix_t& GetViewMatrix();
    int         GetLocalControllerIndex();
    float       GetSensitivity();
    bool        IsAimbotKeyHeld();
}
