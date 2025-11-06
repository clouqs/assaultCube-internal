#include "Aimbot.h"
#include "DistanceCalc.h"
#include "../core/memory.h"
#include <Windows.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <gl/GL.h>

#define M_PI 3.14159265358979323846

Aimbot& Aimbot::Get()
{
    static Aimbot instance;
    return instance;
}

void Aimbot::Update()
{
    if (!enabled) return;

    if (aimKey && !(GetAsyncKeyState(aimKeyCode) & 0x8000)) {
        return;
    }

    auto& gameState = GameState::Get();
    Entity* localPlayer = gameState.GetLocalPlayer();

    if (!localPlayer || localPlayer->player_health <= 0) {
        return;
    }

    // Read view matrix for visibility checks
    if (visibilityCheck) {
        uintptr_t moduleBase = gameState.GetModuleBase();
        __try {
            float* gameViewMatrix = (float*)(moduleBase + 0x17DFD0);
            memcpy(viewMatrix, gameViewMatrix, sizeof(float) * 16);

            int viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            screenWidth = viewport[2];
            screenHeight = viewport[3];
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            // Continue without visibility check if matrix read fails
        }
    }

    Entity* target = GetBestTarget();
    if (target == nullptr) {
        return;
    }

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

    // Calculate direction vector
    Vec3 delta;
    delta.x = targetPos.x - localPlayer->HeadPos.x;
    delta.y = targetPos.y - localPlayer->HeadPos.y;
    delta.z = targetPos.z - localPlayer->HeadPos.z;

    // Calculate angles using AssaultCube's coordinate system
    float targetYaw = -atan2f(delta.x, delta.y) * (180.0f / (float)M_PI) + 180.0f;
    float hypotenuse = sqrtf(delta.x * delta.x + delta.y * delta.y);
    float targetPitch = atan2f(delta.z, hypotenuse) * (180.0f / (float)M_PI);

    if (smoothAim) {
        float deltaYaw = targetYaw - localPlayer->lookleft_right;
        float deltaPitch = targetPitch - localPlayer->lookup_down;

        // Normalize yaw delta
        if (deltaYaw > 180.0f) deltaYaw -= 360.0f;
        else if (deltaYaw < -180.0f) deltaYaw += 360.0f;

        // Apply smooth aim
        float lerpFactor = 1.0f / smoothAmount;
        localPlayer->lookleft_right += deltaYaw / lerpFactor;
        localPlayer->lookup_down += deltaPitch / lerpFactor;
    }
    else {
        localPlayer->lookleft_right = targetYaw;
        localPlayer->lookup_down = targetPitch;
    }

    // Clamp pitch
    if (localPlayer->lookup_down > 89.0f) localPlayer->lookup_down = 89.0f;
    if (localPlayer->lookup_down < -89.0f) localPlayer->lookup_down = -89.0f;
}

Entity* Aimbot::GetBestTarget()
{
    auto& gameState = GameState::Get();
    Entity* localPlayer = gameState.GetLocalPlayer();

    if (!localPlayer) return nullptr;

    Entity* bestTarget = nullptr;
    float bestDistance = FLT_MAX;

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

    for (int i = 1; i <= otherPlayersCount && i < 32; i++) {
        __try {
            uintptr_t entityAddr = entityListPtr + (i * 4);
            uint32_t entityPtr32 = *(uint32_t*)entityAddr;
            Entity* entity = (Entity*)(uintptr_t)entityPtr32;

            if (entity == nullptr || IsBadReadPtr(entity, sizeof(Entity))) continue;
            if (entity == localPlayer) continue;
            if (entity->player_health <= 0) continue;

            // Team check - Same TeamID = same team
            if (teamCheck && localPlayer->TeamID == entity->TeamID) {
                continue;
            }

            // Visibility check - uses WorldToScreen like the working code
            if (visibilityCheck && !IsVisible(localPlayer, entity)) {
                continue;
            }

            // Calculate distance
            float dx = entity->HeadPos.x - localPlayer->HeadPos.x;
            float dy = entity->HeadPos.y - localPlayer->HeadPos.y;
            float distance = sqrtf(dx * dx + dy * dy);

            // FOV check
            if (fov < 360.0f) {
                Vec3 delta;
                delta.x = entity->HeadPos.x - localPlayer->HeadPos.x;
                delta.y = entity->HeadPos.y - localPlayer->HeadPos.y;
                delta.z = entity->HeadPos.z - localPlayer->HeadPos.z;

                float targetYaw = -atan2f(delta.x, delta.y) * (180.0f / (float)M_PI) + 180.0f;
                float hypotenuse = sqrtf(delta.x * delta.x + delta.y * delta.y);
                float targetPitch = atan2f(delta.z, hypotenuse) * (180.0f / (float)M_PI);

                float deltaYaw = targetYaw - localPlayer->lookleft_right;
                float deltaPitch = targetPitch - localPlayer->lookup_down;

                if (deltaYaw > 180.0f) deltaYaw -= 360.0f;
                else if (deltaYaw < -180.0f) deltaYaw += 360.0f;

                float angleDifference = sqrtf(deltaYaw * deltaYaw + deltaPitch * deltaPitch);

                if (angleDifference > fov) {
                    continue;
                }
            }

            if (distance < bestDistance) {
                bestDistance = distance;
                bestTarget = entity;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            continue;
        }
    }

    return bestTarget;
}

bool Aimbot::WorldToScreen(const Vec3& worldPos, Vec2& screenPos)
{
    // Exact implementation from working code
    struct Vector4 {
        float x, y, z, w;
    };

    Vector4 clipCoords;

    clipCoords.x = worldPos.x * viewMatrix[0] + worldPos.y * viewMatrix[4] + worldPos.z * viewMatrix[8] + viewMatrix[12];
    clipCoords.y = worldPos.x * viewMatrix[1] + worldPos.y * viewMatrix[5] + worldPos.z * viewMatrix[9] + viewMatrix[13];
    clipCoords.z = worldPos.x * viewMatrix[2] + worldPos.y * viewMatrix[6] + worldPos.z * viewMatrix[10] + viewMatrix[14];
    clipCoords.w = worldPos.x * viewMatrix[3] + worldPos.y * viewMatrix[7] + worldPos.z * viewMatrix[11] + viewMatrix[15];

    if (clipCoords.w < 2.0f) {  // Same as working code
        return false;
    }

    Vec3 NDC;
    NDC.x = clipCoords.x / clipCoords.w;
    NDC.y = clipCoords.y / clipCoords.w;
    NDC.z = clipCoords.z / clipCoords.w;

    // Same formula as working code
    screenPos.x = (NDC.x + 1.0f) * 0.5f * (float)screenWidth;
    screenPos.y = (1.0f - NDC.y) * 0.5f * (float)screenHeight;

    return true;
}

bool Aimbot::IsVisible(Entity* local, Entity* target)
{
    if (!local || !target) return false;

    Vec2 headScreen, feetScreen;

    // Check HEAD_AND_FEET (both must be visible, like working code)
    if (!WorldToScreen(target->HeadPos, headScreen)) {
        return false;
    }

    if (!WorldToScreen(target->FeetPos, feetScreen)) {
        return false;
    }

    // Both head and feet are on screen - target is visible
    return true;
}

float Aimbot::GetDistanceToCrosshair(Entity* local, Entity* target)
{
    return 0.0f;
}

float Aimbot::GetDistance(Entity* local, Entity* target)
{
    if (!local || !target) return FLT_MAX;
    return CalculateDistance(local, target);
}

bool Aimbot::IsInFOV(Entity* local, Entity* target)
{
    return true;
}

void Aimbot::SmoothAim(float targetYaw, float targetPitch)
{
    // Integrated into Update
}