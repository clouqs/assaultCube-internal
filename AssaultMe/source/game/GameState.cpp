#include <windows.h>
#include "GameState.h"
#include "Offsets.h"

GameState& GameState::Get() {
    static GameState instance;
    return instance;
}

void GameState::Initialize(uintptr_t base) {
    moduleBase = base;
}

void GameState::Update() {
    if (moduleBase == 0) return;

    localPlayer = *(Entity**)(moduleBase + Offsets::LocalPlayer);

    if (localPlayer) {
        if (!fovPtr) fovPtr = (float*)(moduleBase + Offsets::FOV);
        if (!velocityX) velocityX = (float*)((uintptr_t)localPlayer + Offsets::VelocityX);
        if (!velocityY) velocityY = (float*)((uintptr_t)localPlayer + Offsets::VelocityY);
        if (!namePtr) namePtr = (char*)((uintptr_t)localPlayer + Offsets::PlayerName);
        if (!rapidFirePtr) rapidFirePtr = (int64_t*)((uintptr_t)localPlayer + Offsets::RapidFire);
    }
}

bool GameState::IsPlayerValid() const {
    if (!localPlayer) return false;
    BYTE check = *(BYTE*)((uintptr_t)localPlayer + Offsets::LocalPlayerCheck);
    return check == 1;
}