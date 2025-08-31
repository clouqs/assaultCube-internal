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
    if (msg == WM_KEYDOWN && wParam == VK_INSERT)
    {
        showMenu = !showMenu;
        std::cout << "[WndProc] Toggled menu: " << showMenu << "\n";
        return 0; // eat key
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
        ImGui::SetNextWindowPos(ImVec2(200.f, 200.f), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(420.f, 320.f), ImGuiCond_Once);

        ImGui::Begin("AssaultCube Internal - by clouqs", &showMenu, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Press INSERT to toggle this menu");
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
        ImGui::Checkbox("God Mode (Health)", &bHealth);
        ImGui::Checkbox("Infinite Ammo", &bAmmo);
        ImGui::Checkbox("No Recoil", &bRecoil);
        ImGui::Checkbox("No Reload", &bNoReload);

        if (ImGui::Button("Heal Now") && localPlayer)
        {
            localPlayer->player_health = 100;
            localPlayer->armor_quantity = 100;
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

    std::cout << "OpenGL hooks installed. Press INSERT to toggle menu, END to exit.\n";

    // Async key polling
    while (true)
    {
        if (GetAsyncKeyState(VK_END) & 1) break;
        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            showMenu = !showMenu;
            std::cout << "[AsyncKeyState] Toggled menu: " << showMenu << "\n";
        }
        Sleep(10);
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