#include "makcu.h"
#include <Windows.h>
#include <cstdio>
#include <cstring>

static HANDLE s_serial = INVALID_HANDLE_VALUE;
static char   s_connectedPort[16] = {};

// Try to open a COM port and check if it's a MAKCU device
static bool TryPort(const char* comPort, int baudRate)
{
    char path[32];
    snprintf(path, sizeof(path), "\\\\.\\%s", comPort);

    HANDLE h = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE)
        return false;

    DCB dcb = {};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(h, &dcb)) {
        CloseHandle(h);
        return false;
    }

    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity   = NOPARITY;

    if (!SetCommState(h, &dcb)) {
        CloseHandle(h);
        return false;
    }

    COMMTIMEOUTS timeouts = {};
    timeouts.WriteTotalTimeoutConstant = 5;
    timeouts.ReadTotalTimeoutConstant  = 150; // 150ms read timeout for detection
    SetCommTimeouts(h, &timeouts);

    // Send version query
    const char* cmd = "km.version()\r";
    DWORD written;
    WriteFile(h, cmd, (DWORD)strlen(cmd), &written, nullptr);

    // Read response
    char buf[256] = {};
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, nullptr);
    buf[bytesRead] = '\0';

    // Check if response contains "km" (MAKCU identifier)
    bool found = false;
    for (int i = 0; i < (int)bytesRead - 1; i++) {
        if ((buf[i] == 'k' || buf[i] == 'K') && (buf[i+1] == 'm' || buf[i+1] == 'M')) {
            found = true;
            break;
        }
    }

    if (!found) {
        CloseHandle(h);
        return false;
    }

    // Reset timeouts for normal operation (fast writes only)
    timeouts.ReadTotalTimeoutConstant = 0;
    SetCommTimeouts(h, &timeouts);

    s_serial = h;
    return true;
}

bool makcu::Init(const char* comPort, int baudRate)
{
    s_connectedPort[0] = '\0';

    if (comPort) {
        // Direct connection to specified port
        if (TryPort(comPort, baudRate)) {
            snprintf(s_connectedPort, sizeof(s_connectedPort), "%s", comPort);
            return true;
        }
        return false;
    }

    // Auto-detect: scan COM1 through COM20
    for (int i = 1; i <= 20; i++) {
        char port[8];
        snprintf(port, sizeof(port), "COM%d", i);

        if (TryPort(port, baudRate)) {
            snprintf(s_connectedPort, sizeof(s_connectedPort), "%s", port);
            return true;
        }
    }

    return false;
}

void makcu::MoveMouse(int dx, int dy)
{
    if (s_serial == INVALID_HANDLE_VALUE) return;
    if (dx == 0 && dy == 0) return;

    char cmd[64];
    int len = snprintf(cmd, sizeof(cmd), "km.move(%d,%d)\r", dx, dy);

    DWORD written;
    WriteFile(s_serial, cmd, len, &written, nullptr);
}

void makcu::Shutdown()
{
    if (s_serial != INVALID_HANDLE_VALUE) {
        CloseHandle(s_serial);
        s_serial = INVALID_HANDLE_VALUE;
    }
    s_connectedPort[0] = '\0';
}

bool makcu::IsConnected()
{
    return s_serial != INVALID_HANDLE_VALUE;
}

const char* makcu::GetConnectedPort()
{
    return s_connectedPort;
}
