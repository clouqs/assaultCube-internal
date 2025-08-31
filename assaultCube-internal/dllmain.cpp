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
// Entity structure
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
    if (msg == WM_KEYDOWN)
    {
        if (wParam == VK_INSERT)
        {
            showMenu = !showMenu;
            std::cout << "[WndProc] Toggled menu: " << showMenu << "\n";
            return 0;
        }

        if (showMenu)
        {
            switch (wParam)
            {
            case VK_UP:
                currentSelection = (currentSelection - 1 + maxSelections) % maxSelections;
                std::cout << "[WndProc Navigation] Selection: " << currentSelection << "\n";
                return 0;
            case VK_DOWN:
                currentSelection = (currentSelection + 1) % maxSelections;
                std::cout << "[WndProc Navigation] Selection: " << currentSelection << "\n";
                return 0;
            case VK_RETURN:
                switch (currentSelection)
                {
                case 0: bHealth = !bHealth; std::cout << "[WndProc Toggle] God Mode: " << bHealth << "\n"; break;
                case 1: bAmmo = !bAmmo; std::cout << "[WndProc Toggle] Infinite Ammo: " << bAmmo << "\n"; break;
                case 2: bRecoil = !bRecoil; std::cout << "[WndProc Toggle] No Recoil: " << bRecoil << "\n"; break;
                case 3: bNoReload = !bNoReload; std::cout << "[WndProc Toggle] No Reload: " << bNoReload << "\n"; break;
                case 4:
                    if (localPlayer)
                    {
                        localPlayer->player_health = 100;
                        localPlayer->armor_quantity = 100;
                        std::cout << "[WndProc Action] Player healed!\n";
                    }
                    break;
                }
                return 0;
            }
        }
    }

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
    if (!imguiInitialized)
    {
        g_GameWindow = WindowFromDC(hdc);
        if (!g_GameWindow)
            return owglSwapBuffers(hdc);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        io.IniFilename = nullptr;

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(g_GameWindow);
        ImGui_ImplOpenGL2_Init();

        oWndProc = (WNDPROC)SetWindowLongPtrW(g_GameWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
        imguiInitialized = true;
        std::cout << "[Init] ImGui initialized and WndProc hooked (wglSwapBuffers)\n";
    }

    localPlayer = *(ent**)(moduleBase + 0x0017E0A8);

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

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (showMenu)
    {
        ImGui::SetNextWindowPos(ImVec2(10.f, 10.f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(420.f, 320.f), ImGuiCond_Once);

        ImGui::Begin("AssaultCube Internal - by clouqs", &showMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("Press INSERT to toggle this menu");
        ImGui::Text("Use UP/DOWN arrows to navigate, ENTER to select");
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

        auto drawOption = [&](int idx, const char* label, bool enabled = false) {
            if (currentSelection == idx)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

            if (idx == 4)
                ImGui::Text("%s[HEAL NOW]", currentSelection == idx ? "> " : "  ");
            else
                ImGui::Text("%s%s: %s", currentSelection == idx ? "> " : "  ", label, enabled ? "[ON]" : "[OFF]");

            if (currentSelection == idx)
                ImGui::PopStyleColor();
            };

        drawOption(0, "God Mode (Health)", bHealth);
        drawOption(1, "Infinite Ammo", bAmmo);
        drawOption(2, "No Recoil", bRecoil);
        drawOption(3, "No Reload", bNoReload);
        ImGui::Separator();
        drawOption(4, "");

        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

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
        return false;

    void* wglSwapBuffersAddr = GetProcAddress(hOpenGL32, "wglSwapBuffers");
    if (!wglSwapBuffersAddr)
        return false;

    if (MH_Initialize() != MH_OK) return false;
    if (MH_CreateHook(wglSwapBuffersAddr, &hkwglSwapBuffers, reinterpret_cast<LPVOID*>(&owglSwapBuffers)) != MH_OK) return false;
    if (MH_EnableHook(wglSwapBuffersAddr) != MH_OK) return false;

    std::cout << "[MinHook] wglSwapBuffers hooked at " << wglSwapBuffersAddr << "\n";
    return true;
}

// -------------------------------------
// Main thread
// -------------------------------------
static DWORD WINAPI HackThread(HMODULE hModule)
{
    moduleBase = (uintptr_t)GetModuleHandleW(L"ac_client.exe");
	//allocconsole can be removed. add log page to menu instead.
    AllocConsole();
    FILE* f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "AssaultCube Internal Hack Loaded (OpenGL Version)\n";

    while (!GetModuleHandleA("opengl32.dll")) Sleep(100);

    if (!HookOpenGL())
    {
        std::cout << "Failed to hook OpenGL!\n";
        if (f) fclose(f);
        FreeConsole();
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    std::cout << "OpenGL hooks installed. Press INSERT to toggle menu, UP/DOWN to navigate, ENTER to select, END to exit.\n";

    // Async loop only checks END to exit
    while (true)
    {
        if (GetAsyncKeyState(VK_END) & 1) break;
        Sleep(16);
    }

    if (imguiInitialized)
    {
        if (g_GameWindow && oWndProc)
            SetWindowLongPtrW(g_GameWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
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
