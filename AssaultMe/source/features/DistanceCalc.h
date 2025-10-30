#pragma once
#include "Vec3.h"
#include "../game/Entity.h"
#include <cmath>

inline float CalculateDistance(Entity* local, Entity* target) {
    if (!local || !target) return 0.0f;

    float dx = target->FeetPos.x - local->FeetPos.x;
    float dy = target->FeetPos.y - local->FeetPos.y;
    float dz = target->FeetPos.z - local->FeetPos.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}