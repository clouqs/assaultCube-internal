#include "Aimbot.h"
#include "DistanceCalc.h"
#include "../core/memory.h"
#include <Windows.h>
#include <cmath>
#include <algorithm>
#include <iostream>

#define M_PI 3.14159265358979323846

Aimbot& Aimbot::Get()
{
    static Aimbot instance;
    return instance;
}

// Helper struct for angles
struct Angle {
    float yaw;
    float pitch;
};

// Calculate angle to enemy - based on working example
Angle AngleToEnemy(Entity* localPlayer, Vec3 absolutePos) {
    Angle result;

    // AssaultCube uses a different coordinate system
    // Calculate yaw (horizontal angle)
    float azimuth_xy = atan2f(absolutePos.y, absolutePos.x);
    result.yaw = azimuth_xy * (180.0f / (float)M_PI);

    // Calculate pitch (vertical angle)
    float distance = sqrtf(absolutePos.x * absolutePos.x + absolutePos.y * absolutePos.y);
    result.pitch = -atan2f(absolutePos.z, distance) * (180.0f / (float)M_PI);

    return result;
}

void Aimbot::Update()
{
    static int updateCount = 0;
    updateCount++;

    if (!enabled) {
        if (updateCount % 120 == 0) {
            std::cout << "[Aimbot] Update called but DISABLED\n";
        }
        return;
    }

    if (updateCount % 60 == 0) {
        std::cout << "[Aimbot] ===== UPDATE CALLED (Enabled) =====\n";
        std::cout << "[Aimbot] AimKey required: " << aimKey << " KeyCode: " << aimKeyCode << "\n";
    }

    // If aim key is required, check if it's held
    if (aimKey) {
        bool keyPressed = (GetAsyncKeyState(aimKeyCode) & 0x8000) != 0;
        if (updateCount % 60 == 0) {
            std::cout << "[Aimbot] Key check - Pressed: " << keyPressed << "\n";
        }
        if (!keyPressed) {
            return;
        }
    }

    auto& gameState = GameState::Get();
    Entity* localPlayer = gameState.GetLocalPlayer();

    if (!localPlayer || localPlayer->player_health <= 0) {
        if (updateCount % 120 == 0) {
            std::cout << "[Aimbot] No local player or dead\n";
        }
        return;
    }

    // Get best target
    Entity* target = GetBestTarget();
    if (target) {
        std::cout << "[Aimbot] *** TARGET FOUND: " << target->name << " - AIMING NOW ***\n";
        AimAtTarget(target);
    }
    else {
        if (updateCount % 120 == 0) {
            std::cout << "[Aimbot] No valid target found\n";
        }
    }
}

Entity* Aimbot::GetBestTarget()
{
    auto& gameState = GameState::Get();
    Entity* localPlayer = gameState.GetLocalPlayer();

    if (!localPlayer) return nullptr;

    Entity* bestTarget = nullptr;
    float bestScore = FLT_MAX;

    // Get entity list
    uintptr_t moduleBase = gameState.GetModuleBase();

    uintptr_t entityListPtr = 0;
    __try {
        uintptr_t* firstLevelPtr = (uintptr_t*)(moduleBase + 0x18AC04);
        if (firstLevelPtr && *firstLevelPtr != 0) {
            entityListPtr = *firstLevelPtr;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }

    if (entityListPtr == 0) return nullptr;

    int32_t entityCount = 0;
    __try {
        entityCount = *(int32_t*)(moduleBase + 0x18AC0C);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }

    if (entityCount <= 1) return nullptr;

    int32_t otherPlayersCount = entityCount - 1;

    static int debugFrame = 0;
    bool shouldDebug = (debugFrame++ % 120 == 0);

    if (shouldDebug) {
        std::cout << "\n[Aimbot] ========== TARGET SCAN ==========\n";
        std::cout << "[Aimbot] Total players: " << otherPlayersCount << "\n";
        std::cout << "[Aimbot] Settings - FOV: " << fov << " | TeamCheck: " << teamCheck
            << " | VisCheck: " << visibilityCheck << "\n";
        std::cout << "[Aimbot] Local Team ID: " << (int)localPlayer->TeamID << "\n";
    }

    int validEntities = 0;
    int deadFiltered = 0;
    int teamFiltered = 0;
    int fovFiltered = 0;

    for (int i = 1; i <= otherPlayersCount && i < 32; i++) {
        __try {
            uintptr_t entityAddr = entityListPtr + (i * 4);
            uint32_t entityPtr32 = *(uint32_t*)entityAddr;
            Entity* entity = (Entity*)(uintptr_t)entityPtr32;

            if (entity == nullptr || IsBadReadPtr(entity, sizeof(Entity))) {
                if (shouldDebug) std::cout << "[Aimbot] [" << i << "] Invalid pointer\n";
                continue;
            }

            if (entity == localPlayer) {
                if (shouldDebug) std::cout << "[Aimbot] [" << i << "] Is local player\n";
                continue;
            }

            if (shouldDebug) {
                std::cout << "[Aimbot] [" << i << "] " << entity->name
                    << " | HP: " << entity->player_health
                    << " | Dead: " << (int)entity->is_dead
                    << " | Team: " << (int)entity->TeamID << "\n";
            }

            if (entity->player_health <= 0) {
                deadFiltered++;
                if (shouldDebug) std::cout << "    -> FILTERED: Dead or no HP\n";
                continue;
            }

            validEntities++;

            // Team check
            if (teamCheck && entity->TeamID == localPlayer->TeamID) {
                teamFiltered++;
                if (shouldDebug) {
                    std::cout << "    -> FILTERED: Same team (Local: " << (int)localPlayer->TeamID
                        << " Enemy: " << (int)entity->TeamID << ")\n";
                }
                continue;
            }

            // Calculate angle to target for FOV check
            Vec3 targetPos = entity->HeadPos;
            Vec3 absolutePos;
            absolutePos.x = targetPos.x - localPlayer->HeadPos.x;
            absolutePos.y = targetPos.y - localPlayer->HeadPos.y;
            absolutePos.z = targetPos.z - localPlayer->HeadPos.z;

            Angle angleToTarget = AngleToEnemy(localPlayer, absolutePos);

            // Calculate angle difference
            float deltaYaw = angleToTarget.yaw - localPlayer->lookleft_right;
            float deltaPitch = angleToTarget.pitch - localPlayer->lookup_down;

            // Normalize angles
            while (deltaYaw > 180.0f) deltaYaw -= 360.0f;
            while (deltaYaw < -180.0f) deltaYaw += 360.0f;

            float angleDifference = sqrtf(deltaYaw * deltaYaw + deltaPitch * deltaPitch);

            if (shouldDebug) {
                std::cout << "    Angle to enemy: " << angleDifference << "° (limit: " << fov << "°)\n";
                std::cout << "    Target angles - Yaw: " << angleToTarget.yaw << " Pitch: " << angleToTarget.pitch << "\n";
                std::cout << "    Local angles - Yaw: " << localPlayer->lookleft_right << " Pitch: " << localPlayer->lookup_down << "\n";
                std::cout << "    Delta - Yaw: " << deltaYaw << " Pitch: " << deltaPitch << "\n";
            }

            // FOV check
            if (angleDifference > fov) {
                fovFiltered++;
                if (shouldDebug) std::cout << "    -> FILTERED: Outside FOV\n";
                continue;
            }

            // Visibility check (placeholder)
            if (visibilityCheck && !IsVisible(localPlayer, entity)) {
                if (shouldDebug) std::cout << "    -> FILTERED: Not visible\n";
                continue;
            }

            // Calculate score based on priority
            float score = 0.0f;
            switch (priority) {
            case CLOSEST_TO_CROSSHAIR:
                score = angleDifference;
                break;
            case CLOSEST_DISTANCE:
                score = CalculateDistance(localPlayer, entity);
                break;
            case LOWEST_HEALTH:
                score = (float)entity->player_health;
                break;
            }

            if (shouldDebug) {
                std::cout << "    -> *** VALID TARGET *** Score: " << score << "\n";
            }

            if (score < bestScore) {
                bestScore = score;
                bestTarget = entity;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            if (shouldDebug) std::cout << "[Aimbot] [" << i << "] Exception reading entity\n";
            continue;
        }
    }

    if (shouldDebug) {
        std::cout << "[Aimbot] RESULTS:\n";
        std::cout << "  Valid entities: " << validEntities << "\n";
        std::cout << "  Dead filtered: " << deadFiltered << "\n";
        std::cout << "  Team filtered: " << teamFiltered << "\n";
        std::cout << "  FOV filtered: " << fovFiltered << "\n";
        std::cout << "  Best target: " << (bestTarget ? bestTarget->name : "NONE") << "\n";
        std::cout << "[Aimbot] ==================================\n\n";
    }

    return bestTarget;
}

void Aimbot::AimAtTarget(Entity* target)
{
    auto& gameState = GameState::Get();
    Entity* localPlayer = gameState.GetLocalPlayer();

    if (!localPlayer || !target) return;

    // Get target position based on bone
    Vec3 targetPos;
    switch (targetBone) {
    case HEAD:
        targetPos = target->HeadPos;
        break;
    case NECK:
        targetPos = target->HeadPos;
        targetPos.z -= 1.88f;
        break;
    case CHEST:
        targetPos = target->FeetPos;
        targetPos.z += 0.5f;
        break;
    }

    // Calculate absolute position difference
    Vec3 delta;
    delta.x = targetPos.x - localPlayer->HeadPos.x;
    delta.y = targetPos.y - localPlayer->HeadPos.y;
    delta.z = targetPos.z - localPlayer->HeadPos.z;

    // Calculate angles
    float distance = sqrtf(delta.x * delta.x + delta.y * delta.y);
    float targetYaw = atan2f(delta.y, delta.x) * (180.0f / (float)M_PI);
    float targetPitch = -atan2f(delta.z, distance) * (180.0f / (float)M_PI);

    // Calculate delta angles
    float deltaYaw = targetYaw - localPlayer->lookleft_right;
    float deltaPitch = targetPitch - localPlayer->lookup_down;

    // Normalize angles
    if (deltaYaw > 180.0f) deltaYaw -= 360.0f;
    else if (deltaYaw < -180.0f) deltaYaw += 360.0f;

    if (deltaPitch > 180.0f) deltaPitch -= 360.0f;
    else if (deltaPitch < -180.0f) deltaPitch += 360.0f;

    if (smoothAim) {
        // FIXED: Use division like the working example
        // Higher smoothAmount = smoother (slower)
        // Lower smoothAmount = faster
        float lerpFactor = 1.0f / smoothAmount;

        localPlayer->lookleft_right += deltaYaw / lerpFactor;
        localPlayer->lookup_down += deltaPitch / lerpFactor;
    }
    else {
        // Snap aim
        localPlayer->lookleft_right = targetYaw;
        localPlayer->lookup_down = targetPitch;
    }

    // Clamp pitch
    if (localPlayer->lookup_down > 89.0f) localPlayer->lookup_down = 89.0f;
    if (localPlayer->lookup_down < -89.0f) localPlayer->lookup_down = -89.0f;
}

float Aimbot::GetDistanceToCrosshair(Entity* local, Entity* target)
{
    if (!local || !target) return FLT_MAX;

    Vec3 absolutePos;
    absolutePos.x = target->HeadPos.x - local->HeadPos.x;
    absolutePos.y = target->HeadPos.y - local->HeadPos.y;
    absolutePos.z = target->HeadPos.z - local->HeadPos.z;

    Angle angleToTarget = AngleToEnemy(local, absolutePos);

    float deltaYaw = angleToTarget.yaw - local->lookleft_right;
    float deltaPitch = angleToTarget.pitch - local->lookup_down;

    while (deltaYaw > 180.0f) deltaYaw -= 360.0f;
    while (deltaYaw < -180.0f) deltaYaw += 360.0f;

    return sqrtf(deltaYaw * deltaYaw + deltaPitch * deltaPitch);
}

float Aimbot::GetDistance(Entity* local, Entity* target)
{
    if (!local || !target) return FLT_MAX;
    return CalculateDistance(local, target);
}

bool Aimbot::IsVisible(Entity* local, Entity* target)
{
    // Placeholder - always return true for now
    return true;
}

bool Aimbot::IsInFOV(Entity* local, Entity* target)
{
    if (!local || !target) return false;
    float angleToCrosshair = GetDistanceToCrosshair(local, target);
    return angleToCrosshair <= fov;
}

void Aimbot::SmoothAim(float targetYaw, float targetPitch)
{
    // This function is now integrated into AimAtTarget
}