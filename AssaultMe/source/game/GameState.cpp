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

    // EntityList: [[ac_client.exe + 0x18AC04] + 0] -> structure with entities at +4
    // ALWAYS update entityList, not just when it's null
    __try {
        uintptr_t* firstLevelPtr = (uintptr_t*)(moduleBase + Offsets::EntityList);
        if (firstLevelPtr && *firstLevelPtr != 0) {
            uintptr_t secondLevelPtr = *firstLevelPtr;
            entityList = (EntityList_t*)secondLevelPtr;  // Now points to the actual array
        }
        else {
            entityList = nullptr;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        entityList = nullptr;
    }

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