#include "menu.h"
#include "../core/globals.h"
#include "../game/GameState.h"
#include "../game/EntityList.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/imgui_internal.h"
#include "../features/cubescript.h"
#include "../core/memory.h"
#include "../features/speed.h"
#include "../features/logic.h"
#include "../features/DistanceCalc.h"
#include "../features/ESP.h"
#include <iostream>

bool Menu::bHealth = false;
bool Menu::bAmmo = false;
bool Menu::bRecoil = false;
bool Menu::bNoReload = false;
bool Menu::bNoClip = false;
bool Menu::bSpeed = false;
bool Menu::bHealed = false;
bool Menu::bSuicide = false;
bool Menu::bName = false;
bool Menu::bRapidFireEnabled = false;
int64_t Menu::bRapidFire = 60;
float Menu::bFov = 90.0f;

Menu& Menu::Get()
{
    static Menu instance;
    return instance;
}

void Menu::Initialize(HWND window)
{
    LoadSettings();
    ApplyTheme(menuTheme);
}

void Menu::Render()
{
    if (!showMenu) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.WantCaptureKeyboard = false;
    io.WantCaptureMouse = false;

    ImGui::SetNextWindowPos(ImVec2(10.f, 10.f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(480.f, 365.f), ImGuiCond_Once);

    ImGui::Begin("Majorana - clouqs", &showMenu,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(colors.accent[0], colors.accent[1], colors.accent[2], 1.0f));
    ImGui::Text("Press INSERT to toggle menu");
    ImGui::Text("Use arrows to navigate, ENTER to select, TAB to switch tabs");
    ImGui::PopStyleColor();
    ImGui::Separator();

    if (ImGui::BeginTabBar("Tabs")) {
        RenderMainTab();
        RenderVisualsTab();
        RenderESPTab();
        RenderAimbotTab();
        RenderEntitiesTab();
        RenderCustomizationTab();  // NEW TAB
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Menu::RenderMainTab()
{
    if (ImGui::BeginTabItem("Main", nullptr, currentTab == 0 ? ImGuiTabItemFlags_SetSelected : 0)) {

        DrawMenuOption(0, "God Mode", &bHealth);
        DrawMenuOption(1, "Infinite Ammo", &bAmmo);
        DrawMenuOption(2, "No Recoil", &bRecoil);
        DrawMenuOption(3, "No Reload", &bNoReload);
        DrawMenuOption(4, "No Clip", &bNoClip);
        DrawMenuOption(5, "Speed Hack", &bSpeed);
        DrawMenuOption(6, "Heal Player");
        DrawMenuOption(7, "Kill Yourself");
        ImGui::Separator();
        DrawMenuOption(8, "Set Player Name to maxyboo");
        ImGui::Separator();
        DrawMenuOption(9, "Assault rifle Rapid Fire", &bRapidFireEnabled);

        ImGui::Separator();
        ImGui::SliderInt("Fire Rate(ms)", (int*)&bRapidFire, 30, 120);

        auto& gameState = GameState::Get();
        Entity* localPlayer = gameState.GetLocalPlayer();
        int64_t* bRapidFirePtr = gameState.GetRapidFirePtr();

        if (bRapidFirePtr && localPlayer) {
            ImGui::Text("Current Fire Rate: %lld", *bRapidFirePtr);
        }

        ImGui::Separator();

        if (localPlayer) {
            ImGui::Text("HP: %d  | Armor: %d",
                localPlayer->player_health,
                localPlayer->armor_quantity);

            ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f),
                "Position  -  X: %.1f, Y: %.1f, Z: %.1f",
                localPlayer->FeetPos.x, localPlayer->FeetPos.y, localPlayer->FeetPos.z);

            ImGui::TextColored(ImVec4(0.65f, 0.95f, 0.2f, 1.f),
                "Head angles  -  Yaw: %.2f, Pitch: %.2f",
                localPlayer->lookleft_right,
                localPlayer->lookup_down);

            ImGui::Text("Speed: %.1f", CheatManager::Get().currentSpeed);

            char* namePtr = gameState.GetNamePtr();
            if (namePtr) {
                ImGui::Text("Name: %s", namePtr);
            }

            ImGui::Text("Kills: %d", localPlayer->number_of_kills);
        }

        ImGui::EndTabItem();
    }
}

void Menu::RenderVisualsTab()
{
    if (ImGui::BeginTabItem("Visuals", nullptr, currentTab == 1 ? ImGuiTabItemFlags_SetSelected : 0)) {

        ImGui::SliderFloat("Change Fov", &bFov, 60.0f, 170.0f);

        if (fovPtr) {
            ImGui::Text("Current Game FOV: %.1f", *fovPtr);
        }
        ImGui::EndTabItem();
    }
}

void Menu::RenderESPTab()
{
    if (ImGui::BeginTabItem("ESP", nullptr, currentTab == 2 ? ImGuiTabItemFlags_SetSelected : 0)) {

        auto& esp = ESP::Get();

        ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "=== ESP SETTINGS ===");
        ImGui::Separator();

        DrawMenuOption(0, "Enable ESP", &esp.enabled);

        if (esp.enabled) {
            ImGui::Indent(20.0f);
            DrawMenuOption(1, "Draw Boxes", &esp.drawBoxes);
            DrawMenuOption(2, "Draw Lines", &esp.drawLines);
            DrawMenuOption(3, "Draw Health Bars", &esp.drawHealth);
            DrawMenuOption(4, "Draw Names", &esp.drawNames);
            DrawMenuOption(5, "Draw Distance", &esp.drawDistance);
            ImGui::Unindent(20.0f);
        }

        ImGui::Separator();
        ImGui::SliderFloat("Max Distance", &esp.maxDistance, 100.0f, 1000.0f);

        ImGui::Separator();
        float enemyColor[3] = { esp.colors.enemy.r, esp.colors.enemy.g, esp.colors.enemy.b };
        if (ImGui::ColorEdit3("Enemy Color", enemyColor)) {
            esp.colors.enemy.r = enemyColor[0];
            esp.colors.enemy.g = enemyColor[1];
            esp.colors.enemy.b = enemyColor[2];
        }

        ImGui::EndTabItem();
    }
}

void Menu::RenderAimbotTab()
{
    if (ImGui::BeginTabItem("Aimbot", nullptr, currentTab == 3 ? ImGuiTabItemFlags_SetSelected : 0)) {

        ImGui::Text("Aimbot features coming soon...");
        DrawMenuOption(0, "Enable Aimbot", &dummy);
        DrawMenuOption(1, "FOV", nullptr, &dummyfloat);

        ImGui::SliderFloat("FOV", &dummyfloat, 1.0f, 180.0f);

        ImGui::EndTabItem();
    }
}

void Menu::RenderEntitiesTab()
{
    static bool hasErrored = false;

    if (ImGui::BeginTabItem("Entities", nullptr, currentTab == 4 ? ImGuiTabItemFlags_SetSelected : 0)) {

        if (hasErrored) {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Entity tab crashed - disabled for safety");
            if (ImGui::Button("Reset Error Flag")) {
                hasErrored = false;
            }
            ImGui::EndTabItem();
            return;
        }

        ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "=== ENTITY LIST ===");
        ImGui::Separator();

        auto& gameState = GameState::Get();
        uintptr_t moduleBase = gameState.GetModuleBase();
        Entity* localPlayer = gameState.GetLocalPlayer();

        if (moduleBase == 0) {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "ERROR: Module base is NULL!");
            ImGui::EndTabItem();
            return;
        }

        ImGui::Text("Module Base: 0x%llX", (unsigned long long)moduleBase);
        ImGui::Text("LocalPlayer: 0x%p", (void*)localPlayer);
        ImGui::Separator();

        uintptr_t entityListPtr = 0;
        __try {
            uintptr_t* firstLevelPtr = (uintptr_t*)(moduleBase + 0x18AC04);
            if (firstLevelPtr && *firstLevelPtr != 0) {
                entityListPtr = *firstLevelPtr;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "CRASH: Cannot read entity list pointer!");
            hasErrored = true;
            ImGui::EndTabItem();
            return;
        }

        if (entityListPtr == 0) {
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Entity list pointer is NULL");
            ImGui::EndTabItem();
            return;
        }

        ImGui::Text("Entity List Pointer: 0x%llX", (unsigned long long)entityListPtr);

        int32_t entityCount = 0;
        __try {
            entityCount = *(int32_t*)(moduleBase + 0x18AC0C);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "CRASH: Cannot read entity count!");
            hasErrored = true;
            ImGui::EndTabItem();
            return;
        }

        int32_t otherPlayersCount = entityCount - 1;

        ImGui::Separator();
        ImGui::Text("Total Entities (including you): %d", entityCount);
        ImGui::Text("Other Players: %d", otherPlayersCount);
        ImGui::Separator();

        int found = 0;
        for (int i = 1; i <= otherPlayersCount && i < 32; i++) {
            __try {
                uintptr_t entityAddr = entityListPtr + (i * 4);
                uint32_t entityPtr32 = *(uint32_t*)entityAddr;
                Entity* entity = (Entity*)(uintptr_t)entityPtr32;

                if (entity == nullptr || IsBadReadPtr(entity, sizeof(Entity))) {
                    continue;
                }

                found++;
                bool isLocalPlayer = (entity == localPlayer);

                char nameCopy[20] = { 0 };
                memcpy(nameCopy, entity->name, 19);
                nameCopy[19] = '\0';

                ImGui::PushID(i);
                ImGui::Text("[%d] %s", i, nameCopy);
                ImGui::Indent(20.0f);
                ImGui::Text("Health: %d | Armor: %d | Kills: %d",
                    entity->player_health, entity->armor_quantity, entity->number_of_kills);

                if (!isLocalPlayer && localPlayer) {
                    ImGui::Text("Distance: %.1f units", CalculateDistance(localPlayer, entity));
                }
                ImGui::Unindent(20.0f);
                ImGui::Separator();
                ImGui::PopID();

            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "[%d] CRASH reading entity", i);
                hasErrored = true;
                break;
            }
        }

        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Total: %d players", found);
        ImGui::EndTabItem();
    }
}

void Menu::ApplyTheme(int theme)
{
    switch (theme) {
    case 0: // Dark Purple (default)
        colors.background[0] = 0.06f; colors.background[1] = 0.05f; colors.background[2] = 0.07f; colors.background[3] = 0.95f;
        colors.titleBg[0] = 0.10f; colors.titleBg[1] = 0.09f; colors.titleBg[2] = 0.12f; colors.titleBg[3] = 1.00f;
        colors.button[0] = 0.20f; colors.button[1] = 0.19f; colors.button[2] = 0.22f; colors.button[3] = 1.00f;
        colors.text[0] = 0.80f; colors.text[1] = 0.80f; colors.text[2] = 0.83f; colors.text[3] = 1.00f;
        colors.accent[0] = 0.70f; colors.accent[1] = 0.34f; colors.accent[2] = 1.00f; colors.accent[3] = 1.00f;
        break;

    case 1: // Light
        colors.background[0] = 0.94f; colors.background[1] = 0.94f; colors.background[2] = 0.94f; colors.background[3] = 0.95f;
        colors.titleBg[0] = 0.80f; colors.titleBg[1] = 0.80f; colors.titleBg[2] = 0.83f; colors.titleBg[3] = 1.00f;
        colors.button[0] = 0.70f; colors.button[1] = 0.70f; colors.button[2] = 0.73f; colors.button[3] = 1.00f;
        colors.text[0] = 0.10f; colors.text[1] = 0.10f; colors.text[2] = 0.10f; colors.text[3] = 1.00f;
        colors.accent[0] = 0.26f; colors.accent[1] = 0.59f; colors.accent[2] = 0.98f; colors.accent[3] = 1.00f;
        colors.selected[0] = 0.10f; colors.selected[1] = 0.10f; colors.selected[2] = 0.80f; colors.selected[3] = 1.00f;
        break;

    case 2: // Blue
        colors.background[0] = 0.10f; colors.background[1] = 0.12f; colors.background[2] = 0.18f; colors.background[3] = 0.95f;
        colors.titleBg[0] = 0.15f; colors.titleBg[1] = 0.17f; colors.titleBg[2] = 0.23f; colors.titleBg[3] = 1.00f;
        colors.button[0] = 0.20f; colors.button[1] = 0.25f; colors.button[2] = 0.35f; colors.button[3] = 1.00f;
        colors.text[0] = 0.85f; colors.text[1] = 0.90f; colors.text[2] = 1.00f; colors.text[3] = 1.00f;
        colors.accent[0] = 0.26f; colors.accent[1] = 0.59f; colors.accent[2] = 0.98f; colors.accent[3] = 1.00f;
        break;

    case 3: // Custom - don't change colors
        break;
    }

    ApplyColors();
}

void Menu::ApplyColors()
{
    ImGuiStyle* style = &ImGui::GetStyle();

    style->Colors[ImGuiCol_WindowBg] = ImVec4(colors.background[0], colors.background[1], colors.background[2], colors.background[3]);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(colors.titleBg[0], colors.titleBg[1], colors.titleBg[2], colors.titleBg[3]);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(colors.titleBgActive[0], colors.titleBgActive[1], colors.titleBgActive[2], colors.titleBgActive[3]);
    style->Colors[ImGuiCol_Button] = ImVec4(colors.button[0], colors.button[1], colors.button[2], colors.button[3]);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(colors.buttonHovered[0], colors.buttonHovered[1], colors.buttonHovered[2], colors.buttonHovered[3]);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(colors.buttonActive[0], colors.buttonActive[1], colors.buttonActive[2], colors.buttonActive[3]);
    style->Colors[ImGuiCol_Text] = ImVec4(colors.text[0], colors.text[1], colors.text[2], colors.text[3]);
}

void Menu::SaveSettings()
{
    FILE* f = nullptr;
    fopen_s(&f, "menu_config.ini", "w");
    if (f) {
        fprintf(f, "[Theme]\n");
        fprintf(f, "theme=%d\n", menuTheme);

        fprintf(f, "\n[Colors]\n");
        fprintf(f, "background=%.2f,%.2f,%.2f,%.2f\n", colors.background[0], colors.background[1], colors.background[2], colors.background[3]);
        fprintf(f, "text=%.2f,%.2f,%.2f,%.2f\n", colors.text[0], colors.text[1], colors.text[2], colors.text[3]);
        fprintf(f, "accent=%.2f,%.2f,%.2f,%.2f\n", colors.accent[0], colors.accent[1], colors.accent[2], colors.accent[3]);
        fprintf(f, "button=%.2f,%.2f,%.2f,%.2f\n", colors.button[0], colors.button[1], colors.button[2], colors.button[3]);

        fclose(f);
        std::cout << "[Menu] Settings saved to menu_config.ini\n";
    }
}

void Menu::LoadSettings()
{
    FILE* f = nullptr;
    fopen_s(&f, "menu_config.ini", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (sscanf_s(line, "theme=%d", &menuTheme) == 1) continue;

            if (sscanf_s(line, "background=%f,%f,%f,%f",
                &colors.background[0], &colors.background[1], &colors.background[2], &colors.background[3]) == 4) continue;
            if (sscanf_s(line, "text=%f,%f,%f,%f",
                &colors.text[0], &colors.text[1], &colors.text[2], &colors.text[3]) == 4) continue;
            if (sscanf_s(line, "accent=%f,%f,%f,%f",
                &colors.accent[0], &colors.accent[1], &colors.accent[2], &colors.accent[3]) == 4) continue;
            if (sscanf_s(line, "button=%f,%f,%f,%f",
                &colors.button[0], &colors.button[1], &colors.button[2], &colors.button[3]) == 4) continue;
        }
        fclose(f);
        std::cout << "[Menu] Settings loaded from menu_config.ini\n";
    }
}

void Menu::RenderCustomizationTab()
{
    if (ImGui::BeginTabItem("Customize", nullptr, currentTab == 5 ? ImGuiTabItemFlags_SetSelected : 0)) {

        ImGui::TextColored(ImVec4(colors.accent[0], colors.accent[1], colors.accent[2], 1.0f),
            "=== MENU CUSTOMIZATION ===");
        ImGui::Separator();

        DrawMenuOption(0, "Theme: Dark Purple");
        DrawMenuOption(1, "Theme: Light");
        DrawMenuOption(2, "Theme: Blue");
        DrawMenuOption(3, "Theme: Custom");

        ImGui::Separator();
        DrawMenuOption(4, "Save Settings");

        ImGui::Separator();

        if (menuTheme == 3) {
            ImGui::Text("Custom Colors (use in-game console to edit):");
            ImGui::ColorEdit3("Background", colors.background);
            ImGui::ColorEdit3("Text", colors.text);
            ImGui::ColorEdit3("Accent", colors.accent);
            ImGui::ColorEdit3("Button", colors.button);
        }

        ImGui::Separator();
        const char* themeNames[] = { "Dark Purple", "Light", "Blue", "Custom" };
        ImGui::Text("Current Theme: %s", themeNames[menuTheme]);

        ImGui::EndTabItem();
    }
}

void Menu::DrawMenuOption(int idx, const char* label, bool* toggle, float* slider)
{
    bool isSelected = (currentSelection == idx);

    if (isSelected) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(colors.selected[0], colors.selected[1], colors.selected[2], colors.selected[3]));
    }

    if (toggle) {
        ImGui::Text("%s %s %s",
            isSelected ? ">" : " ",
            label,
            *toggle ? "[ON]" : "[OFF]");
    }
    else if (slider) {
        ImGui::Text("%s %s: %.1f",
            isSelected ? ">" : " ",
            label,
            *slider);
    }
    else {
        ImGui::Text("%s %s",
            isSelected ? ">" : " ",
            label);
    }

    if (isSelected) {
        ImGui::PopStyleColor();
    }
}

void Menu::HandleKeyInput(WPARAM key)
{
    if (!showMenu) return;

    auto& cheatMgr = CheatManager::Get();
    auto& esp = ESP::Get();

    switch (key) {
    case VK_UP:
    {
        int maxSelections = 0;
        switch (currentTab) {
        case 0: maxSelections = MainCheatSelections; break;
        case 1: maxSelections = VisualsSelections; break;
        case 2: maxSelections = ESPSelections; break;
        case 3: maxSelections = AimbotSelections; break;
        case 5: maxSelections = CustomizationSelections; break;  // ADD THIS
        }
        if (maxSelections > 0) {
            currentSelection = (currentSelection - 1 + maxSelections) % maxSelections;
        }
        break;
    }

    case VK_DOWN:
    {
        int maxSelections = 0;
        switch (currentTab) {
        case 0: maxSelections = MainCheatSelections; break;
        case 1: maxSelections = VisualsSelections; break;
        case 2: maxSelections = ESPSelections; break;
        case 3: maxSelections = AimbotSelections; break;
        case 5: maxSelections = CustomizationSelections; break;  // ADD THIS
        }
        if (maxSelections > 0) {
            currentSelection = (currentSelection + 1) % maxSelections;
        }
        break;
    }

    case VK_RETURN:
        switch (currentTab) {
        case 0: // Main tab
            switch (currentSelection) {
            case 0:
                cheatMgr.godMode = !cheatMgr.godMode;
                bHealth = cheatMgr.godMode;
                std::cout << "[Menu] God Mode: " << (cheatMgr.godMode ? "ON" : "OFF") << "\n";
                break;
            case 1:
                cheatMgr.infiniteAmmo = !cheatMgr.infiniteAmmo;
                bAmmo = cheatMgr.infiniteAmmo;
                std::cout << "[Menu] Infinite Ammo: " << (cheatMgr.infiniteAmmo ? "ON" : "OFF") << "\n";
                break;
            case 2:
                cheatMgr.noRecoil = !cheatMgr.noRecoil;
                bRecoil = cheatMgr.noRecoil;
                std::cout << "[Menu] No Recoil: " << (cheatMgr.noRecoil ? "ON" : "OFF") << "\n";
                break;
            case 3:
                cheatMgr.noReload = !cheatMgr.noReload;
                bNoReload = cheatMgr.noReload;
                std::cout << "[Menu] No Reload: " << (cheatMgr.noReload ? "ON" : "OFF") << "\n";
                break;
            case 4:
                cheatMgr.noClip = !cheatMgr.noClip;
                bNoClip = cheatMgr.noClip;
                std::cout << "[Menu] No Clip: " << (cheatMgr.noClip ? "ON" : "OFF") << "\n";
                break;
            case 5:
                cheatMgr.speedHack = !cheatMgr.speedHack;
                bSpeed = cheatMgr.speedHack;
                std::cout << "[Menu] Speed Hack: " << (cheatMgr.speedHack ? "ON" : "OFF") << "\n";
                break;
            case 6:
                cheatMgr.doHeal = true;
                std::cout << "[Menu] Healing player...\n";
                break;
            case 7:
                cheatMgr.doSuicide = true;
                std::cout << "[Menu] Suicide triggered...\n";
                break;
            case 8:
                cheatMgr.doChangeName = true;
                std::cout << "[Menu] Changing name...\n";
                break;
            case 9:
                cheatMgr.rapidFire = !cheatMgr.rapidFire;
                bRapidFireEnabled = cheatMgr.rapidFire;
                std::cout << "[Menu] Rapid Fire: " << (cheatMgr.rapidFire ? "ON" : "OFF") << "\n";
                break;
            }
            break;

        case 1: // Visuals tab
            if (currentSelection == 0) {
                cheatMgr.fovValue = bFov;
                std::cout << "[Menu] FOV set to: " << bFov << "\n";
            }
            break;

        case 2: // ESP tab
            switch (currentSelection) {
            case 0:
                esp.enabled = !esp.enabled;
                std::cout << "[Menu] ESP: " << (esp.enabled ? "ON" : "OFF") << "\n";
                break;
            case 1:
                esp.drawBoxes = !esp.drawBoxes;
                std::cout << "[Menu] Draw Boxes: " << (esp.drawBoxes ? "ON" : "OFF") << "\n";
                break;
            case 2:
                esp.drawLines = !esp.drawLines;
                std::cout << "[Menu] Draw Lines: " << (esp.drawLines ? "ON" : "OFF") << "\n";
                break;
            case 3:
                esp.drawHealth = !esp.drawHealth;
                std::cout << "[Menu] Draw Health: " << (esp.drawHealth ? "ON" : "OFF") << "\n";
                break;
            case 4:
                esp.drawNames = !esp.drawNames;
                std::cout << "[Menu] Draw Names: " << (esp.drawNames ? "ON" : "OFF") << "\n";
                break;
            case 5:
                esp.drawDistance = !esp.drawDistance;
                std::cout << "[Menu] Draw Distance: " << (esp.drawDistance ? "ON" : "OFF") << "\n";
                break;
            }
            break;

        case 5: // Customization tab - MOVED HERE (proper position)
            switch (currentSelection) {
            case 0:
                menuTheme = 0;
                ApplyTheme(0);
                std::cout << "[Menu] Applied Dark Purple theme\n";
                break;
            case 1:
                menuTheme = 1;
                ApplyTheme(1);
                std::cout << "[Menu] Applied Light theme\n";
                break;
            case 2:
                menuTheme = 2;
                ApplyTheme(2);
                std::cout << "[Menu] Applied Blue theme\n";
                break;
            case 3:
                menuTheme = 3;
                std::cout << "[Menu] Switched to Custom theme\n";
                break;
            case 4:
                SaveSettings();
                break;
            }
            break;
        }
        break;  // This closes the VK_RETURN case

    case VK_LEFT:
        if (currentTab == 0 && currentSelection == 9) {
            bRapidFire = max(30LL, bRapidFire - 5LL);
            cheatMgr.rapidFireValue = bRapidFire;
            std::cout << "[Menu] RapidFire decreased to: " << bRapidFire << "\n";
        }
        else if (currentTab == 1 && currentSelection == 0) {
            bFov = max(60.0f, bFov - 5.0f);
            cheatMgr.fovValue = bFov;
            std::cout << "[Menu] FOV decreased to: " << bFov << "\n";
        }
        else if (currentTab == 2 && currentSelection == 0) {
            esp.maxDistance = max(100.0f, esp.maxDistance - 50.0f);
            std::cout << "[Menu] Max Distance decreased to: " << esp.maxDistance << "\n";
        }
        break;

    case VK_RIGHT:
        if (currentTab == 0 && currentSelection == 9) {
            bRapidFire = min(120LL, bRapidFire + 5LL);
            cheatMgr.rapidFireValue = bRapidFire;
            std::cout << "[Menu] RapidFire increased to: " << bRapidFire << "\n";
        }
        else if (currentTab == 1 && currentSelection == 0) {
            bFov = min(170.0f, bFov + 5.0f);
            cheatMgr.fovValue = bFov;
            std::cout << "[Menu] FOV increased to: " << bFov << "\n";
        }
        else if (currentTab == 2 && currentSelection == 0) {
            esp.maxDistance = min(1000.0f, esp.maxDistance + 50.0f);
            std::cout << "[Menu] Max Distance increased to: " << esp.maxDistance << "\n";
        }
        break;

    case VK_TAB:
        currentTab = (currentTab + 1) % 6;  // Changed from 5 to 6 for new tab
        currentSelection = 0;
        std::cout << "[Menu Navigation] Changed to tab: " << currentTab << "\n";
        break;
    }
}

void Menu::Shutdown()
{
    // Cleanup if needed
}