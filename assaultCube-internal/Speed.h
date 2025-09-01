#include <math.h>
static void CalculateDirection(float yaw, float& forwardX, float& forwardY)
{
    constexpr float PI = 3.14159265358979323846f;
    float yawRad = yaw * (PI / 180.0f);

    forwardX = sinf(yawRad);  // +X is right
    forwardY = -cosf(yawRad);  // +Y is forward
}

static void NormalizeVector(float& x, float& y)
{
    float length = sqrtf(x * x + y * y);
    if (length > 0.0f)
    {
        x /= length;
        y /= length;
    }
}