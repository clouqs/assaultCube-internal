#pragma once
#include <cstdint>

class CheatManager {
public:
    static CheatManager& Get();
    void Update();

    // Toggles
    bool godMode = false;
    bool infiniteAmmo = false;
    bool noRecoil = false;
    bool noReload = false;
    bool noClip = false;
    bool speedHack = false;
    bool rapidFire = false;

    // Actions
    bool doHeal = false;
    bool doSuicide = false;
    bool doChangeName = false;

    // Values
    float fovValue = 90.0f;
    int64_t rapidFireValue = 60;
    float currentSpeed = 0.0f;

private:
    CheatManager() = default;
    void UpdateGodMode();
    void UpdateInfiniteAmmo();
    void UpdateNoRecoil();
    void UpdateNoReload();
    void UpdateNoClip();
    void UpdateSpeedHack();
    void UpdateRapidFire();
    void UpdateFOV();  // ADD THIS
    void UpdateActions();
};