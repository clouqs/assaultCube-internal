#include "cubescript.h"
#include "stdafx.h"
#include <iostream>
#include <string>
#include <cmath>
#include <windows.h>
#include <algorithm>
#include <gl/GL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl2.h"
#include "minhook/MinHook.h"
#include "mem.h"
#include "Speed.h"

#include <chrono>
#include <thread>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "minhook.x32.lib")


//THINGS TO ADD:
//- Aimbot :)
//- ESP boxes
//Configs
// weapon changer
//menu style customization (color picker etc)
//rapid fire
//handle cubescript injection from own tab
//custom name input field in cubescript tab



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
static HWND g_GameWindow = nullptr;
uintptr_t moduleBase = 0;
ent* localPlayer = nullptr;
float* fovPtr = nullptr;
float* velocity_x = nullptr;
float* velocity_y = nullptr;
char* namePtr = nullptr;
int64_t* bRapidFirePtr = nullptr;
//float* fovPtr = (float*)((uintptr_t)moduleBase + 0x18A7CC);
//float* velocity_x = (float*)((uintptr_t)localPlayer + 0x10);
//float* velocity_y = (float*)((uintptr_t)localPlayer + 0x14);
//char* namePtr = (char*)(localPlayer)+0x205;
//float* bRapidFirePtr = (float*)((uintptr_t)localPlayer + 0x164);

static BYTE localPlayer_check = 0;
static bool imguiInitialized = false;
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
static bool bSuicide = false;
static bool bName = false;
static bool bRapidFireEnabled = false;  // Toggle for rapid fire on/off
static int64_t bRapidFire = 60;
static float bFov = 90;

//Dummy Globals
bool dummy = false;
float dummyfloat;
static float currentSpeed = 0.0f; // Added missing variable

// Menu navigation

static int currentTab = 0;
static int currentSelection = 0;
static const int MainCheatSelections = 10;  //Main tab has 8 options
static const int VisualsSelections = 1;   //Visuals tab has 1 option (Apply FOV)
static const int ESPSelections = 2;      //ESP tab has 2 options
static const int AimbotSelections = 2;  //Aimbot tab has 2 options

// OpenGL hooks
using wglSwapBuffersFn = BOOL(WINAPI*)(HDC);
static wglSwapBuffersFn owglSwapBuffers = nullptr;

// ImGui Win32 WndProc
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
static WNDPROC oWndProc = nullptr;



static void DrawMenuOption(int idx, const char* label, bool* toggle = nullptr, float* slider = nullptr) {
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
                case 0: maxSelections = MainCheatSelections; break;
                case 1: maxSelections = VisualsSelections; break;
                case 2: maxSelections = ESPSelections; break;
                case 3: maxSelections = AimbotSelections; break;
                }
                if (maxSelections > 0) {
                    currentSelection = (currentSelection - 1 + maxSelections) % maxSelections;
                }
            }
            return 0;

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
            }
            return 0;

            case VK_RETURN:
                switch (currentTab)
                {
                case 0: // Main tab
                    switch (currentSelection)
                    {
                    case 0: bHealth = !bHealth; break;
                    case 1: bAmmo = !bAmmo; break;
                    case 2: bRecoil = !bRecoil; break;
                    case 3: bNoReload = !bNoReload; break;
                    case 4: bNoClip = !bNoClip; break;
                    case 5: bSpeed = !bSpeed; break;
                    case 6: bHealed = true; break;
                    case 7: bSuicide = true; break;
                    case 8: bName = true; break;                                                    // Rapid Fire toggle
                    case 9: bRapidFireEnabled = !bRapidFireEnabled;
                        std::cout << "[WndProc Toggle] Rapid Fire: " << bRapidFireEnabled << "\n";  
                        break;
                    }
                    break;

                case 1: // Visuals tab
                    if (currentSelection == 0) {
                        // Apply FOV
                        if (fovPtr && moduleBase != 0) {
                            *fovPtr = bFov;
                            std::cout << "[WndProc] FOV applied: " << bFov << "\n";
                        }
                    }
                    break;
                }
                return 0;

            case VK_LEFT:
                if (currentTab == 0 && currentSelection == 9) { // Rapid fire adjustment
                    bRapidFire = max(30LL, bRapidFire - 5LL);
                    std::cout << "[WndProc] RapidFire decreased to: " << bRapidFire << "\n";
                }
                else if (currentTab == 1 && currentSelection == 0) { // FOV adjustment
                    bFov = max(60.0f, bFov - 5.0f);
                    std::cout << "[WndProc] FOV decreased to: " << bFov << "\n";
                }
                return 0;

            case VK_RIGHT:
                if (currentTab == 0 && currentSelection == 9) { // Rapid fire adjustment
                    bRapidFire = max(30LL, bRapidFire + 5LL);
                    std::cout << "[WndProc] RapidFire increased to: " << bRapidFire << "\n";
                }
                else if (currentTab == 1 && currentSelection == 0) { // FOV adjustment
                    bFov = min(170.0f, bFov + 5.0f);
                    std::cout << "[WndProc] FOV increased to: " << bFov << "\n";
                }
                return 0;

            case VK_TAB:
                currentTab = (currentTab + 1) % 4;
                currentSelection = 0;
                std::cout << "[WndProc Navigation] Changed to tab: " << currentTab << "\n";
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

    // Get localplayer pointers
    localPlayer = *(ent**)(moduleBase + 0x0017E0A8);

    if (localPlayer)
    {
        localPlayer_check = *(BYTE*)((uintptr_t)localPlayer + 0x104);

        // Initialize pointers safely when localPlayer is valid
        if (!fovPtr && moduleBase != 0) {
            fovPtr = (float*)((uintptr_t)moduleBase + 0x18A7CC);
        }

        if (!velocity_x) {
            velocity_x = (float*)((uintptr_t)localPlayer + 0x10);
        }

        if (!velocity_y) {
            velocity_y = (float*)((uintptr_t)localPlayer + 0x14);
        }

        if (!namePtr) {
            namePtr = (char*)((uintptr_t)localPlayer + 0x205);
        }

        if (!bRapidFirePtr) {
            bRapidFirePtr = (int64_t*)((uintptr_t)localPlayer + 0x164);
        }
    }

    if (localPlayer && localPlayer_check == 1)
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
            static float speedMultiplier = 3.0f;

            if (velocity_x && velocity_y)
            {
                float forwardX, forwardY;
                float yaw = localPlayer->lookleft_right;
                CalculateDirection(yaw, forwardX, forwardY);
                NormalizeVector(forwardX, forwardY);

                *velocity_x = forwardX * speedMultiplier;
                *velocity_y = forwardY * speedMultiplier;

                currentSpeed = sqrtf(*velocity_x * *velocity_x + *velocity_y * *velocity_y);
            }
        }
        else if (!bSpeed && localPlayer)
        {
            if (velocity_x && velocity_y)
            {
                float speed = sqrtf(*velocity_x * *velocity_x + *velocity_y * *velocity_y);
                currentSpeed = speed;

                if (speed > 10.0f)
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

        if (bSuicide)
        {
            ExecuteSuicide();
            bSuicide = false;
        }

        if (bName && localPlayer)
        {
            ChangeName();
            bName = false;
        }

        if (bRapidFireEnabled && bRapidFirePtr)
        {
            // Validate pointer before writing
            if (!IsBadWritePtr(bRapidFirePtr, sizeof(int64_t))) {
                *bRapidFirePtr = bRapidFire;
            }
            else {
                std::cout << "[WARNING] Invalid rapid fire pointer detected\n";
                bRapidFirePtr = nullptr;
            }
        }
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (showMenu) {
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
            // MAIN TAB
            if (ImGui::BeginTabItem("Main", nullptr,
                currentTab == 0 ? ImGuiTabItemFlags_SetSelected : 0)) {

                DrawMenuOption(0, "God Mode", &bHealth);
                DrawMenuOption(1, "Infinite Ammo", &bAmmo);
                DrawMenuOption(2, "No Recoil", &bRecoil);
                DrawMenuOption(3, "No Reload", &bNoReload);
                DrawMenuOption(4, "No Clip", &bNoClip);
                DrawMenuOption(5, "Speed Hack", &bSpeed);
                DrawMenuOption(6, "Heal Player"); //action only
                DrawMenuOption(7, "Kill Yourself");//action only
                ImGui::Separator();
                DrawMenuOption(8, "Set Player Name to maxyboo");//action only
                //ImGui::InputText("Set name", )
                ImGui::Separator();
                DrawMenuOption(9, "Rapid Fire", &bRapidFireEnabled);  

                ImGui::Separator();
                ImGui::SliderInt("Fire Rate(ms) - lower = shoot faster", (int*)&bRapidFire, 30, 120);  // Correct label

                // Show correct information
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
                    ImGui::Text("Speed: %.1f", currentSpeed);
                    if (namePtr) {
                        ImGui::Text("Name: %s", namePtr);
                    }
                    ImGui::Text("Kills: %d", localPlayer->number_of_kills);
                }

                ImGui::EndTabItem();
            }

            // VISUALS TAB
            if (ImGui::BeginTabItem("Visuals", nullptr,
                currentTab == 1 ? ImGuiTabItemFlags_SetSelected : 0)) {

                ImGui::SliderFloat("Change Fov", &bFov, 60.0f, 170.0f);

                if (fovPtr) {
                    ImGui::Text("Current Game FOV: %.1f", *fovPtr);
                }

                DrawMenuOption(0, "Apply", nullptr, &bFov);

                ImGui::EndTabItem();
            }

            // ESP TAB
            if (ImGui::BeginTabItem("ESP", nullptr,
                currentTab == 2 ? ImGuiTabItemFlags_SetSelected : 0)) {

                ImGui::Text("ESP features coming soon...");
                DrawMenuOption(0, "Enable ESP", &dummy);
                DrawMenuOption(1, "Draw Boxes", &dummy);

                ImGui::EndTabItem();
            }

            // AIMBOT TAB
            if (ImGui::BeginTabItem("Aimbot", nullptr,
                currentTab == 3 ? ImGuiTabItemFlags_SetSelected : 0)) {

                ImGui::Text("Aimbot features coming soon...");
                DrawMenuOption(0, "Enable Aimbot", &dummy);
                DrawMenuOption(1, "FOV", nullptr, &dummyfloat);

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

    if (!moduleBase) {
        std::cout << "[ERROR] Could not find ac_client.exe module!\n";
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    AllocConsole();
    FILE* f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "Module Base: 0x" << std::hex << moduleBase << std::dec << "\n";

    // Wait for OpenGL
    while (!GetModuleHandleA("opengl32.dll")) {
        std::cout << "[Wait] Waiting for OpenGL...\n";
        Sleep(1000);
    }

    if (!HookOpenGL())
    {
        std::cout << "Failed to hook OpenGL!\n";
        if (f) fclose(f);
        FreeConsole();
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    // Wait for game initialization
    std::cout << "[Wait] Waiting for game initialization (10 seconds)...\n";
    Sleep(10000);

    // Initialize CubeScript 
    if (InitializeCubeScript()) {
        std::cout << "[CubeScript] Initialization successful\n";

        // Optional: Test commands after a delay
        /*Sleep(5000);
        std::cout << "[CubeScript] Starting command tests...\n";
        TestCubeScriptCommands();*/

    }
    else {
        std::cout << "[CubeScript] Initialization failed\n";
    }

    // Main loop
    while (true)
    {
        if (GetAsyncKeyState(VK_END) & 1) break;
        Sleep(16);
    }

    // Cleanup
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