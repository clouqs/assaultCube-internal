#pragma once
#include <Windows.h>
#include <cstdint>

class Menu {
public:
    static Menu& Get();
    void Initialize(HWND window);
    void Render();
    void Shutdown();

    bool IsVisible() const { return showMenu; }
    void Toggle() { showMenu = !showMenu; }
    void HandleKeyInput(WPARAM key);
    bool IsMenuOpen() const { return showMenu; }

    // menu vars
    static bool bHealth;
    static bool bAmmo;
    static bool bRecoil;
    static bool bNoReload;
    static bool bNoClip;
    static bool bSpeed;
    static bool bHealed;
    static bool bSuicide;
    static bool bName;
    static bool bRapidFireEnabled;
    static int64_t bRapidFire;
    static float bFov;

    // Dummy vars for placeholder features
    bool dummy = false;
    float dummyfloat = 90.0f;
    float* fovPtr = nullptr;

private:
    Menu() = default;

    void RenderMainTab();
    void RenderVisualsTab();
    void RenderESPTab();
    void RenderAimbotTab();
    void RenderEntitiesTab();
    void DrawMenuOption(int idx, const char* label, bool* toggle = nullptr, float* slider = nullptr);

    bool showMenu = true;
    int currentTab = 0;
    int currentSelection = 0;

    static constexpr int MainCheatSelections = 10;
    static constexpr int VisualsSelections = 1;
    static constexpr int ESPSelections = 6;
    static constexpr int AimbotSelections = 2;
};