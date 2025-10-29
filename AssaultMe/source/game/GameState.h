#pragma once
#include "Entity.h"
#include "EntityList.h"
#include <cstdint>

class GameState {
public:
    static GameState& Get();
    void Initialize(uintptr_t base);
    void Update();

    Entity* GetLocalPlayer() const { return localPlayer; }
    EntityList_t* GetEntityList() const { return entityList; }
    uintptr_t GetModuleBase() const { return moduleBase; }
    float* GetFOVPtr() const { return fovPtr; }
    float* GetVelocityX() const { return velocityX; }
    float* GetVelocityY() const { return velocityY; }
    char* GetNamePtr() const { return namePtr; }
    int64_t* GetRapidFirePtr() const { return rapidFirePtr; }
    bool IsPlayerValid() const;

private:
    GameState() = default;
    uintptr_t moduleBase = 0;
    Entity* localPlayer = nullptr;
    EntityList_t* entityList = nullptr;
    float* fovPtr = nullptr;
    float* velocityX = nullptr;
    float* velocityY = nullptr;
    char* namePtr = nullptr;
    int64_t* rapidFirePtr = nullptr;
};