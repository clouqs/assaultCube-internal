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
#include <iostream>

//speed hack needs better implementation later.
//implement aimbot and esp later.
//fix when menu is open game can't receive input.
//add custom name input box.


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
    // Initialization handled by main ImGui setup
}

void Menu::Render()
{
    if (!showMenu) { return; }

    // IMPORTANT: Allow game to receive input even when menu is open
    ImGuiIO& io = ImGui::GetIO();
    io.WantCaptureKeyboard = false;  // Let game receive keyboard input
    io.WantCaptureMouse = false;     // Let game receive mouse input

    // SYNC MENU STATE WITH CHEATMANAGER
    auto& cheatMgr = CheatManager::Get();
    bHealth = cheatMgr.godMode;
    bAmmo = cheatMgr.infiniteAmmo;
    bRecoil = cheatMgr.noRecoil;
    bNoReload = cheatMgr.noReload;
    bNoClip = cheatMgr.noClip;
    bSpeed = cheatMgr.speedHack;
    bRapidFireEnabled = cheatMgr.rapidFire;
    bRapidFire = cheatMgr.rapidFireValue;
    bFov = cheatMgr.fovValue;

    ImGui::SetNextWindowPos(ImVec2(10.f, 10.f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(480.f, 365.f), ImGuiCond_Once);

    ImGui::Begin("Majorana - clouqs", &showMenu,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.34f, 1.f, 1.f));
    ImGui::Text("Press INSERT to toggle this menu");
    ImGui::Text("Use arrows to navigate, ENTER to select, TAB to switch tabs");
    ImGui::PopStyleColor();
    ImGui::Separator();

    if (ImGui::BeginTabBar("Tabs")) {
        RenderMainTab();
        RenderVisualsTab();
        RenderESPTab();
        RenderAimbotTab();
        RenderEntitiesTab();
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
        DrawMenuOption(8, "Set Player Name to maxyboo"); //need to add chatbox for custom input.
        ImGui::Separator();
        DrawMenuOption(9, "Assault rifle Rapid Fire", &bRapidFireEnabled);

        ImGui::Separator();
        ImGui::SliderInt("Fire Rate(ms) - lower = shoot faster", (int*)&bRapidFire, 30, 120);

        if (bRapidFirePtr && localPlayer) {
            ImGui::Text("Current Fire Rate: %d", *bRapidFirePtr);
        }
        else {
            ImGui::Text("Fire Rate: %.1f (Not Applied)", bRapidFire);
        }

        ImGui::Separator();

        if (localPlayer) {
            ImGui::Text("HP: %d  | Armor: %d",
                localPlayer->player_health,
                localPlayer->armor_quantity);

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f),
                "Position  -  X: %.1f, Y: %.1f, Z: %.1f",
                localPlayer->X, localPlayer->Y, localPlayer->Z);

            ImGui::TextColored(ImVec4(0.65f, 0.95f, 0.2f, 1.f),
                "Head angles  -  Yaw: %.2f, Pitch: %.2f",
                localPlayer->lookleft_right,
                localPlayer->lookup_down);

            ImGui::Spacing();
            ImGui::Text("Speed: %.1f", CheatManager::Get().currentSpeed);
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

        ImGui::Text("ESP features coming soon...");
        DrawMenuOption(0, "Enable ESP", &dummy);
        DrawMenuOption(1, "Draw Boxes", &dummy);

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
        uintptr_t moduleBase = 0;
        Entity* localPlayer = nullptr;

        __try {
            moduleBase = gameState.GetModuleBase();
            localPlayer = gameState.GetLocalPlayer();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "CRASH: Cannot access game state!");
            hasErrored = true;
            ImGui::EndTabItem();
            return;
        }

        if (moduleBase == 0) {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "ERROR: Module base is NULL!");
            ImGui::EndTabItem();
            return;
        }

        ImGui::Text("Module Base: 0x%llX", (unsigned long long)moduleBase);
        ImGui::Text("LocalPlayer: 0x%p", (void*)localPlayer);
        ImGui::Separator();

        // Read entity list pointer carefully
        uintptr_t entityListPtr = 0;
        __try {
            entityListPtr = *(uintptr_t*)(moduleBase + 0x18AC04);
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
        ImGui::Separator();
        ImGui::Text("Reading entity data...");
        ImGui::Separator();

        // Scan and read entity data
        int found = 0;
        for (int i = 0; i < 32; i++) {
            __try {
                // Read 32-bit pointer (AssaultCube is 32-bit)
                uintptr_t entityAddr = entityListPtr + 4 + (i * 4);
                uint32_t entityPtr32 = *(uint32_t*)entityAddr;
                Entity* entity = (Entity*)(uintptr_t)entityPtr32;

                if (entity == nullptr) {
                    continue;
                }

                // Use IsBadReadPtr to check if memory is readable (Windows specific)
                if (IsBadReadPtr(entity, sizeof(Entity))) {
                    ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f),
                        "[%d] Pointer 0x%p - INVALID (bad memory)", i, (void*)entity);
                    continue;
                }

                found++;

                // Try to read entity data
                bool isLocalPlayer = (entity == localPlayer);

                // Read basic info
                float x = entity->X;
                float y = entity->Y;
                float z = entity->Z;
                int32_t health = entity->player_health;
                int32_t armor = entity->armor_quantity;
                int32_t kills = entity->number_of_kills;
                bool isDead = entity->is_dead;

                // Try to read name safely
                char nameCopy[20] = { 0 };
                memcpy(nameCopy, entity->name, 19);
                nameCopy[19] = '\0'; // Ensure null termination

                // Display entity info
                ImGui::PushID(i);

                if (isLocalPlayer) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 1.f, 1.f, 1.f));
                    ImGui::Text("[%d] === YOU === 0x%p", i, (void*)entity);
                    ImGui::PopStyleColor();
                }
                else {
                    ImGui::Text("[%d] Entity 0x%p", i, (void*)entity);
                }

                ImGui::Indent(20.0f);

                ImGui::Text("Name: %s", nameCopy);
                ImGui::Text("Health: %d | Armor: %d | Kills: %d", health, armor, kills);
                ImGui::Text("Position: X=%.1f, Y=%.1f, Z=%.1f", x, y, z);

                if (isDead) {
                    ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Status: DEAD");
                }
                else {
                    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Status: ALIVE");
                }

                // Calculate distance from local player if this isn't local player
                if (!isLocalPlayer && localPlayer) {
                    float dx = x - localPlayer->X;
                    float dy = y - localPlayer->Y;
                    float dz = z - localPlayer->Z;
                    float distance = sqrtf(dx * dx + dy * dy + dz * dz);
                    ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Distance: %.1f units", distance);
                }

                ImGui::Unindent(20.0f);
                ImGui::Separator();

                ImGui::PopID();

            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f),
                    "[%d] CRASH reading entity data at 0x%llX",
                    i, (unsigned long long)(entityListPtr + 4 + (i * 4)));
                hasErrored = true;
                break;
            }
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Total entities found: %d", found);

        ImGui::EndTabItem();
    }
}

void Menu::DrawMenuOption(int idx, const char* label, bool* toggle, float* slider)
{
    bool isSelected = (currentSelection == idx);

    if (isSelected) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
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

//HandleKeyInput function
void Menu::HandleKeyInput(WPARAM key)
{
    if (!showMenu)
        return;

    auto& cheatMgr = CheatManager::Get();

    switch (key) {
    case VK_UP:
    {
        int maxSelections = 0;
        switch (currentTab) {
        case 0: maxSelections = MainCheatSelections; break;
        case 1: maxSelections = VisualsSelections; break;
        case 2: maxSelections = ESPSelections; break;
        case 3: maxSelections = AimbotSelections; break;
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
                std::cout << "[Menu] God Mode: " << (cheatMgr.godMode ? "ON" : "OFF") << "\n";
                break;
            case 1:
                cheatMgr.infiniteAmmo = !cheatMgr.infiniteAmmo;
                std::cout << "[Menu] Infinite Ammo: " << (cheatMgr.infiniteAmmo ? "ON" : "OFF") << "\n";
                break;
            case 2:
                cheatMgr.noRecoil = !cheatMgr.noRecoil;
                std::cout << "[Menu] No Recoil: " << (cheatMgr.noRecoil ? "ON" : "OFF") << "\n";
                break;
            case 3:
                cheatMgr.noReload = !cheatMgr.noReload;
                std::cout << "[Menu] No Reload: " << (cheatMgr.noReload ? "ON" : "OFF") << "\n";
                break;
            case 4:
                cheatMgr.noClip = !cheatMgr.noClip;
                std::cout << "[Menu] No Clip: " << (cheatMgr.noClip ? "ON" : "OFF") << "\n";
                break;
            case 5:
                cheatMgr.speedHack = !cheatMgr.speedHack;
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
                std::cout << "[Menu] Rapid Fire: " << (cheatMgr.rapidFire ? "ON" : "OFF") << "\n";
                break;
            }
            break;

        case 1: // Visuals tab
            if (currentSelection == 0) {
                // Apply FOV through CheatManager
                cheatMgr.fovValue = bFov;
                std::cout << "[Menu] FOV set to: " << bFov << " (will be applied by CheatManager)\n";
            }
            break;
        }
        break;

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
        break;

    case VK_TAB:
        currentTab = (currentTab + 1) % 5;
        currentSelection = 0;
        std::cout << "[Menu Navigation] Changed to tab: " << currentTab << "\n";
        break;
    }
}

void Menu::Shutdown()
{
    // Cleanup if needed
}