#pragma once
#include <cmath>
#include "../game/Entity.h"
static inline float CalculateDistance(Entity* from, Entity* to)
{
    if (!from || !to) return 0.0f;

    float dx = to->X - from->X;
    float dy = to->Y - from->Y;
    float dz = to->Z - from->Z;

    return sqrtf(dx * dx + dy * dy + dz * dz);
}