#pragma once
#include <Windows.h>
#include <cstdint>

struct MenuColors {
    float background[4] = { 0.06f, 0.05f, 0.07f, 0.95f };
    float titleBg[4] = { 0.10f, 0.09f, 0.12f, 1.00f };
    float titleBgActive[4] = { 0.15f, 0.14f, 0.17f, 1.00f };
    float button[4] = { 0.20f, 0.19f, 0.22f, 1.00f };
    float buttonHovered[4] = { 0.30f, 0.29f, 0.32f, 1.00f };
    float buttonActive[4] = { 0.40f, 0.39f, 0.42f, 1.00f };
    float text[4] = { 0.80f, 0.80f, 0.83f, 1.00f };
    float accent[4] = { 0.70f, 0.34f, 1.00f, 1.00f };
    float selected[4] = { 1.00f, 1.00f, 0.00f, 1.00f };
};

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

    // Customization
    MenuColors colors;
    int menuTheme = 0; // 0=Dark, 1=Light, 2=Blue, 3=Custom

private:
    Menu() = default;

    void RenderMainTab();
    void RenderVisualsTab();
    void RenderESPTab();
    void RenderAimbotTab();
    void RenderEntitiesTab();
    void RenderCustomizationTab();
    void DrawMenuOption(int idx, const char* label, bool* toggle = nullptr, float* slider = nullptr);

    bool showMenu = true;
    int currentTab = 0;
    int currentSelection = 0;

    //menu editing
    bool editingColor = false;
    int editingColorComponent = 0;

    void ApplyTheme(int theme);
    void ApplyColors();
    void SaveSettings();
    void LoadSettings();

    static constexpr int MainCheatSelections = 10;
    static constexpr int VisualsSelections = 1;
    static constexpr int ESPSelections = 6;
    static constexpr int AimbotSelections = 10;
    static constexpr int CustomizationSelections = 9;
    int GetCustomizationSelections() const {
        return menuTheme == 3 ? 9 : 5; // 9 if custom theme, 5 otherwise
    }
};