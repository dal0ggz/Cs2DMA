#include "config.h"
#include "globals.h"
#include <Windows.h>
#include <cstdio>
#include <cstring>

// INI file path — next to the executable
static char s_iniPath[MAX_PATH] = {};

static const char* GetIniPath()
{
    if (s_iniPath[0] == '\0') {
        GetModuleFileNameA(nullptr, s_iniPath, MAX_PATH);
        // Replace .exe with .ini (or append if no extension)
        char* dot = strrchr(s_iniPath, '.');
        if (dot)
            strcpy(dot, ".ini");
        else
            strcat(s_iniPath, ".ini");
    }
    return s_iniPath;
}

// --- Helpers ---

static void WriteBool(const char* section, const char* key, bool val)
{
    WritePrivateProfileStringA(section, key, val ? "1" : "0", GetIniPath());
}

static bool ReadBool(const char* section, const char* key, bool def)
{
    return GetPrivateProfileIntA(section, key, def ? 1 : 0, GetIniPath()) != 0;
}

static void WriteInt(const char* section, const char* key, int val)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", val);
    WritePrivateProfileStringA(section, key, buf, GetIniPath());
}

static int ReadInt(const char* section, const char* key, int def)
{
    return GetPrivateProfileIntA(section, key, def, GetIniPath());
}

static void WriteFloat(const char* section, const char* key, float val)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%.4f", val);
    WritePrivateProfileStringA(section, key, buf, GetIniPath());
}

static float ReadFloat(const char* section, const char* key, float def)
{
    char buf[32];
    char defStr[32];
    snprintf(defStr, sizeof(defStr), "%.4f", def);
    GetPrivateProfileStringA(section, key, defStr, buf, sizeof(buf), GetIniPath());
    return (float)atof(buf);
}

static void WriteColor(const char* section, const char* key, const float c[4])
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%.3f,%.3f,%.3f,%.3f", c[0], c[1], c[2], c[3]);
    WritePrivateProfileStringA(section, key, buf, GetIniPath());
}

static void ReadColor(const char* section, const char* key, float c[4], const float def[4])
{
    char buf[64];
    char defStr[64];
    snprintf(defStr, sizeof(defStr), "%.3f,%.3f,%.3f,%.3f", def[0], def[1], def[2], def[3]);
    GetPrivateProfileStringA(section, key, defStr, buf, sizeof(buf), GetIniPath());
    if (sscanf(buf, "%f,%f,%f,%f", &c[0], &c[1], &c[2], &c[3]) != 4) {
        memcpy(c, def, sizeof(float) * 4);
    }
}

// --- Save / Load ---

void config::Save()
{
    // ESP
    WriteBool("ESP", "Enabled",       g::espEnabled);
    WriteBool("ESP", "Box",           g::espBox);
    WriteBool("ESP", "Health",        g::espHealth);
    WriteBool("ESP", "Name",          g::espName);
    WriteBool("ESP", "Skeleton",      g::espSkeleton);
    WriteBool("ESP", "Snaplines",     g::espSnaplines);
    WriteBool("ESP", "SnapFromTop",   g::espSnaplineFromTop);
    WriteColor("ESP", "BoxColor",      g::espBoxColor);
    WriteColor("ESP", "NameColor",     g::espNameColor);
    WriteColor("ESP", "SkeletonColor", g::espSkeletonColor);
    WriteColor("ESP", "SnaplineColor", g::espSnaplineColor);

    // Radar
    WriteBool("Radar",  "Enabled",  g::radarEnabled);
    WriteFloat("Radar", "Range",    g::radarRange);
    WriteFloat("Radar", "Size",     g::radarSize);
    WriteFloat("Radar", "DotSize",  g::radarDotSize);
    WriteColor("Radar", "BgColor",  g::radarBgColor);
    WriteColor("Radar", "DotColor", g::radarDotColor);

    // Aimbot
    WriteBool("Aimbot",  "Enabled",      g::aimbotEnabled);
    WriteFloat("Aimbot", "FOV",          g::aimbotFov);
    WriteFloat("Aimbot", "Smooth",       g::aimbotSmooth);
    WriteInt("Aimbot",   "Bone",         g::aimbotBone);
    WriteBool("Aimbot",  "VisCheck",     g::aimbotVisCheck);
    WriteBool("Aimbot",  "DrawFov",      g::aimbotDrawFov);
    WriteColor("Aimbot", "FovColor",     g::fovCircleColor);
    WriteBool("Aimbot",  "InstantLock",  g::aimbotInstantLock);
    WriteInt("Aimbot",   "InstantFrames", g::aimbotInstantFrames);
    WriteInt("Aimbot",   "AimKey",       g::aimbotKey);

    // Screen
    WriteInt("Screen", "Width",  g::screenWidth);
    WriteInt("Screen", "Height", g::screenHeight);
}

void config::Load()
{
    // Check if file exists
    if (GetFileAttributesA(GetIniPath()) == INVALID_FILE_ATTRIBUTES)
        return; // no config file, use defaults

    // ESP
    g::espEnabled       = ReadBool("ESP", "Enabled",     g::espEnabled);
    g::espBox           = ReadBool("ESP", "Box",         g::espBox);
    g::espHealth        = ReadBool("ESP", "Health",      g::espHealth);
    g::espName          = ReadBool("ESP", "Name",        g::espName);
    g::espSkeleton      = ReadBool("ESP", "Skeleton",    g::espSkeleton);
    g::espSnaplines     = ReadBool("ESP", "Snaplines",   g::espSnaplines);
    g::espSnaplineFromTop = ReadBool("ESP", "SnapFromTop", g::espSnaplineFromTop);
    ReadColor("ESP", "BoxColor",      g::espBoxColor,      g::espBoxColor);
    ReadColor("ESP", "NameColor",     g::espNameColor,     g::espNameColor);
    ReadColor("ESP", "SkeletonColor", g::espSkeletonColor, g::espSkeletonColor);
    ReadColor("ESP", "SnaplineColor", g::espSnaplineColor, g::espSnaplineColor);

    // Radar
    g::radarEnabled = ReadBool("Radar",  "Enabled", g::radarEnabled);
    g::radarRange   = ReadFloat("Radar", "Range",   g::radarRange);
    g::radarSize    = ReadFloat("Radar", "Size",    g::radarSize);
    g::radarDotSize = ReadFloat("Radar", "DotSize", g::radarDotSize);
    ReadColor("Radar", "BgColor",  g::radarBgColor,  g::radarBgColor);
    ReadColor("Radar", "DotColor", g::radarDotColor, g::radarDotColor);

    // Aimbot
    g::aimbotEnabled      = ReadBool("Aimbot",  "Enabled",      g::aimbotEnabled);
    g::aimbotFov          = ReadFloat("Aimbot", "FOV",          g::aimbotFov);
    g::aimbotSmooth       = ReadFloat("Aimbot", "Smooth",       g::aimbotSmooth);
    g::aimbotBone         = ReadInt("Aimbot",   "Bone",         g::aimbotBone);
    g::aimbotVisCheck     = ReadBool("Aimbot",  "VisCheck",     g::aimbotVisCheck);
    g::aimbotDrawFov      = ReadBool("Aimbot",  "DrawFov",      g::aimbotDrawFov);
    ReadColor("Aimbot", "FovColor", g::fovCircleColor, g::fovCircleColor);
    g::aimbotInstantLock  = ReadBool("Aimbot",  "InstantLock",  g::aimbotInstantLock);
    g::aimbotInstantFrames = ReadInt("Aimbot",  "InstantFrames", g::aimbotInstantFrames);
    g::aimbotKey           = ReadInt("Aimbot",  "AimKey",        g::aimbotKey);
    if (g::aimbotKey < 0 || g::aimbotKey > 3) g::aimbotKey = 0;

    // Screen
    g::screenWidth  = ReadInt("Screen", "Width",  g::screenWidth);
    g::screenHeight = ReadInt("Screen", "Height", g::screenHeight);
}
