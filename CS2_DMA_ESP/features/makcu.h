#pragma once

namespace makcu {
    // If comPort is nullptr, auto-detects by scanning COM1-COM20
    bool Init(const char* comPort = nullptr, int baudRate = 115200);
    void MoveMouse(int dx, int dy);
    void Shutdown();
    bool IsConnected();
    const char* GetConnectedPort();
}
