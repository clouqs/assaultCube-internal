#include "stdafx.h"
#include <iostream>
#include <string>
#include <cmath>
#include <windows.h>
#include <gl/GL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl2.h"
#include "minhook/MinHook.h"
#include "mem.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "minhook.x32.lib")

// -------------------------------------
// Entity structure (same as your original)
// -------------------------------------
class ent {
public:
    char   pad_0000[4];
    float  X;
    float  Y;
    float  Z;
    char   pad_0010[36];
    float  lookleft_right;
    float  lookup_down;
    char   pad_003C[176];
    int32_t player_health;
    int32_t armor_quantity;
    char   pad_00F4[20];
    int32_t pistol_stored_ammo;
    char   pad_010C[16];
    int32_t assault_rifle_stored_ammo;
    char   pad_0120[32];
    int32_t assault_rifle_ammo;
    int32_t grenade_number;
    char   pad_0148[8];
    int32_t pistol_shot_reload_delay;
    char   pad_0154[16];
    int64_t assault_rifle_shot_reload_delay;
    char   pad_016C[8];
    int32_t pistol_total_shots;
    char   pad_0178[16];
    int32_t assault_rifle_total_shots;
    char   pad_018C[80];
    int32_t number_of_kills;
    char   pad_01E0[32];
    char   name[19];
    char   pad_0210[264];
    bool   is_dead;
    char   pad_0319[75];
    void* weapon_in_hand;
    char   pad_0368[480];
};

// -------------------------------------
// Globals
// -------------------------------------
static HWND       g_GameWindow = nullptr;
static uintptr_t  moduleBase = 0;
static ent* localPlayer = nullptr;
static bool       imguiInitialized = false;

static bool showMenu = true;
static bool bHealth = false;
static bool bAmmo = false;
static bool bRecoil = false;
static bool bNoReload = false;

// Menu navigation
static int currentSelection = 0;
static const int maxSelections = 5; // 4 checkboxes + 1 button

// OpenGL hooks
using wglSwapBuffersFn = BOOL(WINAPI*)(HDC);
static wglSwapBuffersFn owglSwapBuffers = nullptr;

// ImGui Win32 WndProc
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
static WNDPROC oWndProc = nullptr;

// -------------------------------------
// WndProc
// -------------------------------------
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Handle all our keys first, before ImGui or the game gets them
    if (msg == WM_KEYDOWN)
    {
        if (wParam == VK_INSERT)
        {
            showMenu = !showMenu;
            std::cout << "[WndProc] Toggled menu: " << showMenu << "\n";
            return 0; // eat key
        }

        if (showMenu)
        {
            switch (wParam)
            {
            case VK_UP:
                currentSelection = (currentSelection - 1 + maxSelections) % maxSelections;
                std::cout << "[WndProc Navigation] Selection: " << currentSelection << "\n";
                return 0; // eat key
            case VK_DOWN:
                currentSelection = (currentSelection + 1) % maxSelections;
                std::cout << "[WndProc Navigation] Selection: " << currentSelection << "\n";
                return 0; // eat key
            case VK_RETURN:
            case VK_SPACE:
                // Toggle selected option
                switch (currentSelection)
                {
                case 0: bHealth = !bHealth; std::cout << "[WndProc Toggle] God Mode: " << bHealth << "\n"; break;
                case 1: bAmmo = !bAmmo; std::cout << "[WndProc Toggle] Infinite Ammo: " << bAmmo << "\n"; break;
                case 2: bRecoil = !bRecoil; std::cout << "[WndProc Toggle] No Recoil: " << bRecoil << "\n"; break;
                case 3: bNoReload = !bNoReload; std::cout << "[WndProc Toggle] No Reload: " << bNoReload << "\n"; break;
                case 4: // Heal button
                    if (localPlayer)
                    {
                        localPlayer->player_health = 100;
                        localPlayer->armor_quantity = 100;
                        std::cout << "[WndProc Action] Player healed!\n";
                    }
                    break;
                }
                return 0; // eat key
            }
        }
    }

    // Let ImGui handle other input only if menu is open
    if (showMenu && imguiInitialized)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return 1;
    }

    return CallWindowProcW(oWndProc, hWnd, msg, wParam, lParam);
}

// -------------------------------------
// wglSwapBuffers hook
// -------------------------------------
static BOOL WINAPI hkwglSwapBuffers(HDC hdc)
{
    // One-time ImGui init
    if (!imguiInitialized)
    {
        g_GameWindow = WindowFromDC(hdc);
        if (!g_GameWindow)
            return owglSwapBuffers(hdc);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        io.IniFilename = nullptr; // don't write imgui.ini

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(g_GameWindow);
        ImGui_ImplOpenGL2_Init();

        oWndProc = (WNDPROC)SetWindowLongPtrW(g_GameWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
        imguiInitialized = true;
        std::cout << "[Init] ImGui initialized and WndProc hooked (wglSwapBuffers)\n";
    }

    // Update local player pointer
    localPlayer = *(ent**)(moduleBase + 0x0017E0A8);

    // Apply patches
    if (localPlayer)
    {
        if (bHealth)
        {
            localPlayer->player_health = 1000;
            localPlayer->armor_quantity = 100;
        }

        if (bAmmo)
            mem::Nop((BYTE*)(moduleBase + 0xC73EF), 2);
        else
            mem::Patch((BYTE*)(moduleBase + 0xC73EF), (BYTE*)"\xFF\x08", 2);

        if (bNoReload)
            mem::Nop((BYTE*)(moduleBase + 0xC8FC7), 2);
        else
            mem::Patch((BYTE*)(moduleBase + 0xC8FC7), (BYTE*)"\x01\x01", 2);

        if (bRecoil)
            mem::Nop((BYTE*)(moduleBase + 0xC2EC3), 5);
        else
            mem::Patch((BYTE*)(moduleBase + 0xC2EC3), (BYTE*)"\xF3\x0F\x11\x56\x38", 5);
    }

    // Backup OpenGL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    // Draw ImGui
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (showMenu)
    {
        ImGui::SetNextWindowPos(ImVec2(10.f, 10.f), ImGuiCond_Always); // Top-left corner
        ImGui::SetNextWindowSize(ImVec2(420.f, 320.f), ImGuiCond_Once);

        ImGui::Begin("AssaultCube Internal - by clouqs", &showMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("Press INSERT to toggle this menu");
        ImGui::Text("Use UP/DOWN arrows to navigate, ENTER/SPACE to select");
        ImGui::Separator();

        if (localPlayer)
        {
            ImGui::Text("HP: %d  | Armor: %d", localPlayer->player_health, localPlayer->armor_quantity);
            ImGui::Text("Pos: %.1f, %.1f, %.1f", localPlayer->X, localPlayer->Y, localPlayer->Z);
            ImGui::Text("Name: %s", localPlayer->name);
            ImGui::Text("Kills: %d", localPlayer->number_of_kills);
            ImGui::Separator();
        }

        ImGui::Text("Combat Hacks:");

        // God Mode checkbox with selection highlight
        if (currentSelection == 0)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
            ImGui::Text("> God Mode (Health): %s", bHealth ? "[ON]" : "[OFF]");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::Text("  God Mode (Health): %s", bHealth ? "[ON]" : "[OFF]");
        }

        // Infinite Ammo checkbox with selection highlight
        if (currentSelection == 1)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
            ImGui::Text("> Infinite Ammo: %s", bAmmo ? "[ON]" : "[OFF]");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::Text("  Infinite Ammo: %s", bAmmo ? "[ON]" : "[OFF]");
        }

        // No Recoil checkbox with selection highlight
        if (currentSelection == 2)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
            ImGui::Text("> No Recoil: %s", bRecoil ? "[ON]" : "[OFF]");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::Text("  No Recoil: %s", bRecoil ? "[ON]" : "[OFF]");
        }

        // No Reload checkbox with selection highlight
        if (currentSelection == 3)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
            ImGui::Text("> No Reload: %s", bNoReload ? "[ON]" : "[OFF]");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::Text("  No Reload: %s", bNoReload ? "[ON]" : "[OFF]");
        }

        ImGui::Separator();

        // Heal button with selection highlight
        if (currentSelection == 4)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
            ImGui::Text("> [HEAL NOW]");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::Text("  [HEAL NOW]");
        }

        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Restore OpenGL state
    glPopClientAttrib();
    glPopAttrib();

    return owglSwapBuffers(hdc);
}

// -------------------------------------
// Hook wglSwapBuffers
// -------------------------------------
static bool HookOpenGL()
{
    HMODULE hOpenGL32 = GetModuleHandleA("opengl32.dll");
    if (!hOpenGL32)
    {
        std::cout << "[Hook] opengl32.dll not found!\n";
        return false;
    }

    void* wglSwapBuffersAddr = GetProcAddress(hOpenGL32, "wglSwapBuffers");
    if (!wglSwapBuffersAddr)
    {
        std::cout << "[Hook] wglSwapBuffers not found!\n";
        return false;
    }

    if (MH_Initialize() != MH_OK) {
        std::cout << "[MinHook] Init failed!\n";
        return false;
    }

    if (MH_CreateHook(wglSwapBuffersAddr, &hkwglSwapBuffers, reinterpret_cast<LPVOID*>(&owglSwapBuffers)) != MH_OK) {
        std::cout << "[MinHook] CreateHook(wglSwapBuffers) failed!\n";
        return false;
    }

    if (MH_EnableHook(wglSwapBuffersAddr) != MH_OK) {
        std::cout << "[MinHook] EnableHook(wglSwapBuffers) failed!\n";
        return false;
    }

    std::cout << "[MinHook] wglSwapBuffers hooked at " << wglSwapBuffersAddr << "\n";
    return true;
}

// -------------------------------------
// Main thread
// -------------------------------------
static DWORD WINAPI HackThread(HMODULE hModule)
{
    moduleBase = (uintptr_t)GetModuleHandleW(L"ac_client.exe");

    AllocConsole();
    FILE* f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "AssaultCube Internal Hack Loaded (OpenGL Version)\n";

    // Wait for OpenGL to be loaded
    while (!GetModuleHandleA("opengl32.dll")) Sleep(100);

    if (!HookOpenGL())
    {
        std::cout << "Failed to hook OpenGL!\n";
        if (f) fclose(f);
        FreeConsole();
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    std::cout << "OpenGL hooks installed. Press INSERT to toggle menu, UP/DOWN to navigate, ENTER/SPACE to select, END to exit.\n";

    // Async key polling with more aggressive checking
    while (true)
    {
        if (GetAsyncKeyState(VK_END) & 1) break;

        // Check INSERT key
        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            showMenu = !showMenu;
            std::cout << "[AsyncKeyState] Toggled menu: " << showMenu << "\n";
        }

        // Only handle navigation if menu is open
        if (showMenu)
        {
            // Use GetAsyncKeyState with bit 1 to detect key press (not just held)
            static bool upPressed = false, downPressed = false, enterPressed = false, spacePressed = false;

            bool upState = (GetAsyncKeyState(VK_UP) & 0x8000) != 0;
            bool downState = (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0;
            bool enterState = (GetAsyncKeyState(VK_RETURN) & 0x8000) != 0;
            bool spaceState = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;

            // UP key
            if (upState && !upPressed)
            {
                currentSelection = (currentSelection - 1 + maxSelections) % maxSelections;
                std::cout << "[AsyncKeyState] UP - Selection: " << currentSelection << "\n";
            }
            upPressed = upState;

            // DOWN key  
            if (downState && !downPressed)
            {
                currentSelection = (currentSelection + 1) % maxSelections;
                std::cout << "[AsyncKeyState] DOWN - Selection: " << currentSelection << "\n";
            }
            downPressed = downState;

            // ENTER key
            if (enterState && !enterPressed)
            {
                switch (currentSelection)
                {
                case 0: bHealth = !bHealth; std::cout << "[AsyncKeyState] Toggle God Mode: " << bHealth << "\n"; break;
                case 1: bAmmo = !bAmmo; std::cout << "[AsyncKeyState] Toggle Infinite Ammo: " << bAmmo << "\n"; break;
                case 2: bRecoil = !bRecoil; std::cout << "[AsyncKeyState] Toggle No Recoil: " << bRecoil << "\n"; break;
                case 3: bNoReload = !bNoReload; std::cout << "[AsyncKeyState] Toggle No Reload: " << bNoReload << "\n"; break;
                case 4:
                    if (localPlayer)
                    {
                        localPlayer->player_health = 100;
                        localPlayer->armor_quantity = 100;
                        std::cout << "[AsyncKeyState] Player healed!\n";
                    }
                    break;
                }
            }
            enterPressed = enterState;

            // SPACE key
            if (spaceState && !spacePressed)
            {
                switch (currentSelection)
                {
                case 0: bHealth = !bHealth; std::cout << "[AsyncKeyState] Toggle God Mode: " << bHealth << "\n"; break;
                case 1: bAmmo = !bAmmo; std::cout << "[AsyncKeyState] Toggle Infinite Ammo: " << bAmmo << "\n"; break;
                case 2: bRecoil = !bRecoil; std::cout << "[AsyncKeyState] Toggle No Recoil: " << bRecoil << "\n"; break;
                case 3: bNoReload = !bNoReload; std::cout << "[AsyncKeyState] Toggle No Reload: " << bNoReload << "\n"; break;
                case 4:
                    if (localPlayer)
                    {
                        localPlayer->player_health = 100;
                        localPlayer->armor_quantity = 100;
                        std::cout << "[AsyncKeyState] Player healed!\n";
                    }
                    break;
                }
            }
            spacePressed = spaceState;
        }

        Sleep(16); // ~60fps checking
    }

    // Cleanup
    if (imguiInitialized)
    {
        if (g_GameWindow && oWndProc)
        {
            SetWindowLongPtrW(g_GameWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
            oWndProc = nullptr;
        }
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        imguiInitialized = false;
    }

    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if (f) fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

// -------------------------------------
// DllMain
// -------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr);
        if (hThread) CloseHandle(hThread);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        if (imguiInitialized && g_GameWindow && oWndProc)
            SetWindowLongPtrW(g_GameWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        MH_DisableHook(MH_ALL_HOOKS);
        MH_RemoveHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }
    return TRUE;
}