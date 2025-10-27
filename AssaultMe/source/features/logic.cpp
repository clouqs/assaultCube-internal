#include "logic.h"
#include "../game/GameState.h"
#include "../game/Offsets.h"
#include "../core/Memory.h"
#include "../features/Speed.h"
#include "../features/CubeScript.h"
#include <cmath>
#include <iostream>

CheatManager& CheatManager::Get() {
    static CheatManager instance;
    return instance;
}

void CheatManager::Update() {
    auto& game = GameState::Get();
    if (!game.IsPlayerValid()) return;

    UpdateGodMode();
    UpdateInfiniteAmmo();
    UpdateNoRecoil();
    UpdateNoReload();
    UpdateNoClip();
    UpdateSpeedHack();
    UpdateRapidFire();
    UpdateFOV();  // ADD THIS
    UpdateActions();
}

void CheatManager::UpdateGodMode() {
    auto player = GameState::Get().GetLocalPlayer();
    if (godMode && player) {
        player->player_health = 1000;
        player->armor_quantity = 100;
    }
}

void CheatManager::UpdateInfiniteAmmo() {
    uintptr_t base = GameState::Get().GetModuleBase();
    if (infiniteAmmo) {
        Memory::Nop((BYTE*)(base + Offsets::InfiniteAmmoPatch), 2);
    }
    else {
        Memory::Patch((BYTE*)(base + Offsets::InfiniteAmmoPatch), (BYTE*)"\xFF\x08", 2);
    }
}

void CheatManager::UpdateNoRecoil() {
    uintptr_t base = GameState::Get().GetModuleBase();
    if (noRecoil) {
        Memory::Nop((BYTE*)(base + Offsets::NoRecoilPatch), 5);
    }
    else {
        Memory::Patch((BYTE*)(base + Offsets::NoRecoilPatch),
            (BYTE*)"\xF3\x0F\x11\x56\x38", 5);
    }
}

void CheatManager::UpdateNoReload() {
    uintptr_t base = GameState::Get().GetModuleBase();
    if (noReload) {
        Memory::Nop((BYTE*)(base + Offsets::NoReloadPatch), 2);
    }
    else {
        Memory::Patch((BYTE*)(base + Offsets::NoReloadPatch), (BYTE*)"\x01\x01", 2);
    }
}

void CheatManager::UpdateNoClip() {
    auto player = GameState::Get().GetLocalPlayer();
    if (player) {
        *(DWORD*)((char*)player + Offsets::NoClipFlag) = noClip ? 0x00000004 : 0x00000000;
    }
}

void CheatManager::UpdateSpeedHack() {
    auto player = GameState::Get().GetLocalPlayer();
    auto velX = GameState::Get().GetVelocityX();
    auto velY = GameState::Get().GetVelocityY();

    if (!player || !velX || !velY) return;

    if (speedHack) {
        static float speedMultiplier = 3.0f;
        float forwardX, forwardY;
        float yaw = player->lookleft_right;

        CalculateDirection(yaw, forwardX, forwardY);
        NormalizeVector(forwardX, forwardY);

        *velX = forwardX * speedMultiplier;
        *velY = forwardY * speedMultiplier;
        currentSpeed = sqrtf(*velX * *velX + *velY * *velY);
    }
    else {
        float speed = sqrtf(*velX * *velX + *velY * *velY);
        currentSpeed = speed;

        if (speed > 10.0f) {
            *velX = 0.0f;
            *velY = 0.0f;
            currentSpeed = 0.0f;
        }
    }
}

void CheatManager::UpdateRapidFire() {
    auto rapidFirePtr = GameState::Get().GetRapidFirePtr();

    if (!rapidFirePtr) {
        std::cout << "[CheatManager] RapidFire pointer is NULL!\n";
        return;
    }

    if (rapidFire) {
        if (!IsBadWritePtr(rapidFirePtr, sizeof(int64_t))) {
            *rapidFirePtr = rapidFireValue;
            std::cout << "[CheatManager] RapidFire applied: " << rapidFireValue << "\n";
        }
        else {
            std::cout << "[CheatManager] RapidFire pointer is invalid!\n";
        }
    }
    else {
        // Reset to default when disabled (optional)
        if (!IsBadWritePtr(rapidFirePtr, sizeof(int64_t))) {
            if (*rapidFirePtr != 60) {
                *rapidFirePtr = 60;  // Reset to default
                std::cout << "[CheatManager] RapidFire reset to default\n";
            }
        }
    }
}

void CheatManager::UpdateFOV() {
    auto fovPtr = GameState::Get().GetFOVPtr();  // CHANGED: GetFOVPtr() instead of GetFovPtr()

    if (!fovPtr) {
        std::cout << "[CheatManager] FOV pointer is NULL!\n";
        return;
    }

    // Always keep FOV synced with fovValue
    if (!IsBadWritePtr(fovPtr, sizeof(float))) {
        if (*fovPtr != fovValue) {
            *fovPtr = fovValue;
            std::cout << "[CheatManager] FOV updated to: " << fovValue << "\n";
        }
    }
    else {
        std::cout << "[CheatManager] FOV pointer is invalid!\n";
    }
}

void CheatManager::UpdateActions() {
    auto player = GameState::Get().GetLocalPlayer();

    if (doHeal && player) {
        player->player_health = 100;
        player->armor_quantity = 100;
        std::cout << "[CheatManager] Player healed!\n";
        doHeal = false;
    }

    if (doSuicide) {
        ExecuteSuicide();
        std::cout << "[CheatManager] Suicide executed!\n";
        doSuicide = false;
    }

    if (doChangeName) {
        ChangeName();
        std::cout << "[CheatManager] Name changed!\n";
        doChangeName = false;
    }
}