#pragma once
#include <cmath>

struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vector3 operator-(const Vector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vector3 operator*(float s)          const { return { x * s, y * s, z * s }; }

    float Length()   const { return sqrtf(x * x + y * y + z * z); }
    float Length2D() const { return sqrtf(x * x + y * y); }
};

using view_matrix_t = float[4][4];

struct ScreenPos {
    float x, y;
    bool onScreen;
};

inline ScreenPos WorldToScreen(const Vector3& world, const view_matrix_t& matrix,
                                float screenW, float screenH)
{
    float w = matrix[3][0] * world.x + matrix[3][1] * world.y + matrix[3][2] * world.z + matrix[3][3];
    if (w < 0.001f)
        return { 0, 0, false };

    float x = matrix[0][0] * world.x + matrix[0][1] * world.y + matrix[0][2] * world.z + matrix[0][3];
    float y = matrix[1][0] * world.x + matrix[1][1] * world.y + matrix[1][2] * world.z + matrix[1][3];

    float nx = x / w;
    float ny = y / w;

    float sx = (screenW * 0.5f) + (nx * screenW * 0.5f);
    float sy = (screenH * 0.5f) - (ny * screenH * 0.5f);

    return { sx, sy, true };
}

inline Vector3 CalcAngle(const Vector3& src, const Vector3& dst) {
    Vector3 delta = dst - src;
    float hyp = delta.Length2D();

    Vector3 angles;
    angles.x = -atan2f(delta.z, hyp) * (180.0f / 3.14159265358979f);
    angles.y = atan2f(delta.y, delta.x) * (180.0f / 3.14159265358979f);
    angles.z = 0.0f;
    return angles;
}

inline void NormalizeAngles(Vector3& angles) {
    if (angles.x >  89.0f) angles.x =  89.0f;
    if (angles.x < -89.0f) angles.x = -89.0f;

    while (angles.y >  180.0f) angles.y -= 360.0f;
    while (angles.y < -180.0f) angles.y += 360.0f;
}

inline float GetFov(const Vector3& viewAngles, const Vector3& aimAngles) {
    Vector3 delta;
    delta.x = aimAngles.x - viewAngles.x;
    delta.y = aimAngles.y - viewAngles.y;
    delta.z = 0.0f;

    if (delta.y >  180.0f) delta.y -= 360.0f;
    if (delta.y < -180.0f) delta.y += 360.0f;

    return delta.Length2D();
}
