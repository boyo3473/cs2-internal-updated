#include <system_error>
#pragma ones
float calculateDistance(const Vector3& pointA, const Vector3& pointB) {
    float dx = pointA.x - pointB.x;
    float dy = pointA.y - pointB.y;
    float dz = pointA.z - pointB.z;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

Vector3 CalcAngle(Vector3 src, Vector3 dst) {
    Vector3 angles;
    angles.x = atan2(dst.z - src.z, sqrt(pow(dst.x - src.x, 2) + pow(dst.y - src.y, 2))) * 180.0f / 3.14159265358979323846f;
    angles.y = atan2(dst.y - src.y, dst.x - src.x) * 180.0f / 3.14159265358979323846f;
    angles.z = 0.0f;
    return angles;
}
Vector3 Lerp(const Vector3& start, const Vector3& end, float factor) {
    return {
        start.x + factor * (end.x - start.x),
        start.y + factor * (end.y - start.y),
        start.z + factor * (end.z - start.z)
    };
}

template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


float MapSmoothingFactor(float factor) {
    return clamp(factor, 0.01f, 1.0f);
}

float CalculateFOV(const Vector3& viewAngles, const Vector3& targetAngles) {
    // Calculate FOV in degrees
    float deltaX = targetAngles.x - viewAngles.x;
    float deltaY = targetAngles.y - viewAngles.y;
    return sqrt(deltaX * deltaX + deltaY * deltaY);
}
