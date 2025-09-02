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
#include "Speed.h"

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
static BYTE localPlayer_check = 0;
static bool       imguiInitialized = false;
static std::string cachedName = "Loading...";
static bool nameRetrieved = false;
static char setName[32] = "Dummy";

static bool showMenu = true;
static bool bHealth = false;
static bool bAmmo = false;
static bool bRecoil = false;
static bool bNoReload = false;
static bool bNoClip = false;
static bool bSpeed = false;
static bool bHealed = false;
static float bFov = 90;

//delete later:
bool dummy = false;
float dummyfloat;
static float currentSpeed = 0.0f; // Added missing variable

// Menu navigation
static int currentSelection = 0;
static const int MainCheatSelections = 7;
static const int ESPSelections = 2;
static int currentTab = 0;

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
            {
                int maxSelections = 0;
                switch (currentTab) {
                case 0: maxSelections = MainCheatSelections; break;  // Main: 7 options
                case 1: maxSelections = 1; break;                    // Visuals: 1 option (Apply FOV)
                case 2: maxSelections = 2; break;                    // ESP: 2 checkboxes
                case 3: maxSelections = 2; break;                    // Aimbot: 2 options
                }
                if (maxSelections > 0) {
                    currentSelection = (currentSelection - 1 + maxSelections) % maxSelections;
                }
                std::cout << "[WndProc Navigation] Tab: " << currentTab << " Selection: " << currentSelection << "\n";
            }
            return 0;

            case VK_DOWN:
            {
                int maxSelections = 0;
                switch (currentTab) {
                case 0: maxSelections = MainCheatSelections; break;
                case 1: maxSelections = 1; break;
                case 2: maxSelections = 2; break;
                case 3: maxSelections = 2; break;
                }
                if (maxSelections > 0) {
                    currentSelection = (currentSelection + 1) % maxSelections;
                }
                std::cout << "[WndProc Navigation] Tab: " << currentTab << " Selection: " << currentSelection << "\n";
            }
            return 0;

            case VK_RETURN:
                // Handle actions based on current tab
                switch (currentTab)
                {
                case 0: // Main tab
                    switch (currentSelection)
                    {
                    case 0:
                        bHealth = !bHealth;
                        std::cout << "[WndProc Toggle] God Mode: " << bHealth << "\n";
                        break;
                    case 1:
                        bAmmo = !bAmmo;
                        std::cout << "[WndProc Toggle] Infinite Ammo: " << bAmmo << "\n";
                        break;
                    case 2:
                        bRecoil = !bRecoil;
                        std::cout << "[WndProc Toggle] No Recoil: " << bRecoil << "\n";
                        break;
                    case 3:
                        bNoReload = !bNoReload;
                        std::cout << "[WndProc Toggle] No Reload: " << bNoReload << "\n";
                        break;
                    case 4:
                        bNoClip = !bNoClip;
                        std::cout << "[WndProc Toggle] NoClip: " << bNoClip << "\n";
                        break;
                    case 5:
                        bSpeed = !bSpeed;
                        std::cout << "[WndProc Toggle] Speed Hack: " << bSpeed << "\n";
                        break;
                    case 6:
                        bHealed = true;
                        std::cout << "[WndProc Toggle] Heal Player activated\n";
                        break;
                    }
                    break;

                case 1: // Visuals tab
                    switch (currentSelection)
                    {
                    case 0:
                        // Apply current FOV value to game
                        if (moduleBase != 0) {
                            float* fovPtr = (float*)((uintptr_t)moduleBase + 0x18A7CC);
                            if (fovPtr) {
                                *fovPtr = bFov;
                                std::cout << "[WndProc] FOV applied: " << bFov << "\n";
                            }
                        }
                        break;
                    }
                    break;

                case 2: // ESP tab
                    switch (currentSelection)
                    {
                    case 0:
                        dummy = !dummy; // Toggle ESP enable (placeholder)
                        std::cout << "[WndProc Toggle] ESP Enable: " << dummy << "\n";
                        break;
                    case 1:
                        // Toggle Draw Boxes (you'll need another bool for this)
                        std::cout << "[WndProc Toggle] Draw Boxes toggled\n";
                        break;
                    }
                    break;

                case 3: // Aimbot tab
                    switch (currentSelection)
                    {
                    case 0:
                        dummy = !dummy; // Toggle Aimbot enable (placeholder)
                        std::cout << "[WndProc Toggle] Aimbot Enable: " << dummy << "\n";
                        break;
                    case 1:
                        // FOV adjustment could be handled here or with left/right arrows
                        std::cout << "[WndProc] Aimbot FOV option selected\n";
                        break;
                    }
                    break;
                }
                return 0;

            case VK_TAB:
                currentTab = (currentTab + 1) % 4;
                currentSelection = 0; // Reset selection when changing tabs
                std::cout << "[WndProc Navigation] Changed to tab: " << currentTab << "\n";
                return 0;

            case VK_LEFT: // Add left/right for adjusting values
                if (currentTab == 1 && currentSelection == 0) { // FOV adjustment
                    bFov = max(60.0f, bFov - 5.0f);
                    std::cout << "[WndProc] FOV decreased to: " << bFov << "\n";
                }
                else if (currentTab == 3 && currentSelection == 1) { // Aimbot FOV
                    dummyfloat = max(1.0f, dummyfloat - 10.0f);
                    std::cout << "[WndProc] Aimbot FOV decreased to: " << dummyfloat << "\n";
                }
                return 0;

            case VK_RIGHT:
                if (currentTab == 1 && currentSelection == 0) { // FOV adjustment
                    bFov = min(170.0f, bFov + 5.0f);
                    std::cout << "[WndProc] FOV increased to: " << bFov << "\n";
                }
                else if (currentTab == 3 && currentSelection == 1) { // Aimbot FOV
                    dummyfloat = min(180.0f, dummyfloat + 10.0f);
                    std::cout << "[WndProc] Aimbot FOV increased to: " << dummyfloat << "\n";
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
////get game pointers: 
    localPlayer = *(ent**)(moduleBase + 0x0017E0A8);
	localPlayer_check = *(BYTE*)((uintptr_t)localPlayer + 0x104);
    
    float* fovPtr = (float*)((uintptr_t)moduleBase + 0x18A7CC);
    float* velocity_x = (float*)((uintptr_t)localPlayer + 0x10);
    float* velocity_y = (float*)((uintptr_t)localPlayer + 0x14);

    if (localPlayer && localPlayer_check == 1)
    {
        // Try to cache the name if not already retrieved
        if (!nameRetrieved)
        {
            // Method 2: Try offset 0x205 if struct field failed
            char* namePtr = (char*)(localPlayer)+0x205;
            if (namePtr && namePtr[0] != '\0' && isprint((unsigned char)namePtr[0])) //Checks the first character isn't a null terminator (empty string)
            {
                size_t len = 0;
                while (len < 19 && namePtr[len] != '\0' && isprint((unsigned char)namePtr[len])) {
                    ++len;
                }
                if (len > 0) {
                    cachedName.assign(namePtr, len);
                    nameRetrieved = true;
                    std::cout << "[Cache] Player name retrieved from offset 0x205: " << cachedName << "\n";
                }
            }
        }

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
        if (bNoClip)
        {
            *(DWORD*)((char*)localPlayer + 0x76) = 00000004;
        }
        else
        {
            *(DWORD*)((char*)localPlayer + 0x76) = 00000000;
        }
        if (bSpeed && localPlayer)
        {
            static float speedMultiplier = 3.0f; // speed multiplier

            if (velocity_x && velocity_y)
            {
                // Calculate direction based on view angles
                float forwardX, forwardY;
                float yaw = localPlayer->lookleft_right;
                CalculateDirection(yaw, forwardX, forwardY);
                NormalizeVector(forwardX, forwardY);

                *velocity_x = forwardX * speedMultiplier;
                *velocity_y = forwardY * speedMultiplier;

                // Calculate current speed for display
                currentSpeed = sqrtf(*velocity_x * *velocity_x + *velocity_y * *velocity_y);
            }
        }
        else if (!bSpeed && localPlayer)
        {
            float* velocity_x = (float*)((uintptr_t)localPlayer + 0x10);
            float* velocity_y = (float*)((uintptr_t)localPlayer + 0x14);

            if (velocity_x && velocity_y)
            {
                float speed = sqrtf(*velocity_x * *velocity_x + *velocity_y * *velocity_y);
                currentSpeed = speed;

                if (speed > 10.0f) // Threshold for normal movement (potentially useless without silder [to add])
                {
                    *velocity_x = 0.0f;
                    *velocity_y = 0.0f;
                    currentSpeed = 0.0f;
                }
            }
        }
        if (bHealed && localPlayer)
        {
            localPlayer->player_health = 100;
            localPlayer->armor_quantity = 100;
            bHealed = false;
        }
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

        ImGui::Begin("Majorana - clouqs", &showMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("Press INSERT to toggle this menu");
        ImGui::Text("Use UP/DOWN arrows to navigate, ENTER to select, TAB to switch tabs");
        ImGui::Separator();

        if (ImGui::BeginTabBar("Tabs"))
        {
            if (ImGui::BeginTabItem("Main", nullptr, currentTab == 0 ? ImGuiTabItemFlags_SetSelected : 0))
            {
                auto drawOptionMain = [&](int idx, const char* label, bool enabled, bool isAction = false) {
                    if (currentSelection == idx) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Highlight
                        if (isAction) {
                            ImGui::Text("> %s", label); // just show the action
                        }
                        else {
                            ImGui::Text("> %s: %s", label, enabled ? "[ON]" : "[OFF]");
                        }
                        ImGui::PopStyleColor();
                    }
                    else {
                        if (isAction) {
                            ImGui::Text("  %s", label);
                        }
                        else {
                            ImGui::Text("  %s: %s", label, enabled ? "[ON]" : "[OFF]");
                        }
                    }
                    };

                drawOptionMain(0, "God Mode", bHealth);
                drawOptionMain(1, "Infinite Ammo", bAmmo);
                drawOptionMain(2, "No Recoil", bRecoil);
                drawOptionMain(3, "No Reload", bNoReload);
                drawOptionMain(4, "No Clip", bNoClip);
                drawOptionMain(5, "Speed Hack", bSpeed);
				drawOptionMain(6, "Heal Player",false, true);

                ImGui::Spacing();
                ImGui::Separator();

                if (localPlayer)
                {
                    ImGui::Text("HP: %d  | Armor: %d", localPlayer->player_health, localPlayer->armor_quantity);
                    ImGui::Spacing();

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange
                    ImGui::Text("Position  -  X: %.1f, Y: %.1f, Z: %.1f", localPlayer->X, localPlayer->Y, localPlayer->Z);
                    ImGui::PopStyleColor();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.95f, 0.2f, 0.8f)); // Lime - i colori della sicilia :)
                    ImGui::Text("Head angles  -  Yaw: %.2f, Pitch: %.2f", localPlayer->lookleft_right, localPlayer->lookup_down);

                    ImGui::PopStyleColor();
                    ImGui::Spacing();
                    ImGui::Text("Speed: %.1f", currentSpeed);
                    ImGui::Text("Name: %s", cachedName.c_str());
                    ImGui::Text("Kills: %d", localPlayer->number_of_kills);
                    ImGui::NewLine();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Visuals", nullptr, currentTab == 1 ? ImGuiTabItemFlags_SetSelected : 0))
            {
                auto drawOptionVisuals = [&](int idx, const char* label, bool isAction = false) {
                    if (currentTab == 1 && currentSelection == idx) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Highlight
                        if (isAction) {
                            ImGui::Text("> %s: %.1f (LEFT/RIGHT to adjust, ENTER to apply)", label, bFov);
                        }
                        ImGui::PopStyleColor();
                    }
                    else {
                        if (isAction) {
                            ImGui::Text("  %s: %.1f", label, bFov);
                        }
                    }
                    };

                ImGui::SliderFloat("Change Fov", &bFov, 60.0f, 170.0f);

                // Show current game FOV if available
                if (moduleBase != 0) {
                    float* fovPtr = (float*)((uintptr_t)moduleBase + 0x18A7CC);
                    if (fovPtr) {
                        ImGui::Text("Current Game FOV: %.1f", *fovPtr);
                    }
                }

                drawOptionVisuals(0, "Apply FOV", true);
                ImGui::Text("Tip: Use LEFT/RIGHT arrows to adjust, ENTER to apply to game");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("ESP", nullptr, currentTab == 2 ? ImGuiTabItemFlags_SetSelected : 0))
            {
                auto drawOptionESP = [&](int idx, const char* label, bool enabled) {
                    if (currentTab == 2 && currentSelection == idx) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Highlight
                        ImGui::Text("> %s: %s", label, enabled ? "[ON]" : "[OFF]");
                        ImGui::PopStyleColor();
                    }
                    else {
                        ImGui::Text("  %s: %s", label, enabled ? "[ON]" : "[OFF]");
                    }
                    };

                ImGui::Text("ESP features coming soon...");
                drawOptionESP(0, "Enable ESP", dummy);
                drawOptionESP(1, "Draw Boxes", dummy); // You'll need a separate bool for this
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Aimbot", nullptr, currentTab == 3 ? ImGuiTabItemFlags_SetSelected : 0))
            {
                auto drawOptionAimbot = [&](int idx, const char* label, bool enabled, bool isSlider = false) {
                    if (currentTab == 3 && currentSelection == idx) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Highlight
                        if (isSlider) {
                            ImGui::Text("> %s: %.1f (Use LEFT/RIGHT arrows)", label, dummyfloat);
                        }
                        else {
                            ImGui::Text("> %s: %s", label, enabled ? "[ON]" : "[OFF]");
                        }
                        ImGui::PopStyleColor();
                    }
                    else {
                        if (isSlider) {
                            ImGui::Text("  %s: %.1f", label, dummyfloat);
                        }
                        else {
                            ImGui::Text("  %s: %s", label, enabled ? "[ON]" : "[OFF]");
                        }
                    }
                    };

                ImGui::Text("Aimbot features coming soon...");
                drawOptionAimbot(0, "Enable Aimbot", dummy);
                drawOptionAimbot(1, "FOV", false, true);
                ImGui::SliderFloat("FOV", &dummyfloat, 1.0f, 180.0f);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
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