#pragma once
#include "../game/Entity.h"
#include "../game/GameState.h"
#include <Windows.h>
#include <vector>

class Aimbot {
public:
    static Aimbot& Get();

    void Update();
    Entity* GetBestTarget();
    void AimAtTarget(Entity* target);

    // Settings
    bool enabled = false;
    bool smoothAim = true;
    float smoothAmount = 5.0f;  // Lower = smoother
    float fov = 90.0f;
    bool visibilityCheck = true;
    bool teamCheck = true;
    bool aimKey = false;  // Requires holding a key
    int aimKeyCode = VK_RBUTTON;  // Right mouse button

    // Targeting priority
    enum TargetPriority {
        CLOSEST_TO_CROSSHAIR = 0,
        CLOSEST_DISTANCE = 1,
        LOWEST_HEALTH = 2
    };
    TargetPriority priority = CLOSEST_TO_CROSSHAIR;

    // Aim bone
    enum AimBone {
        HEAD = 0,
        NECK = 1,
        CHEST = 2
    };
    AimBone targetBone = HEAD;

private:
    Aimbot() = default;

    float GetDistanceToCrosshair(Entity* local, Entity* target);
    float GetDistance(Entity* local, Entity* target);
    bool IsVisible(Entity* local, Entity* target);
    bool IsInFOV(Entity* local, Entity* target);
    void SmoothAim(float targetYaw, float targetPitch);
};