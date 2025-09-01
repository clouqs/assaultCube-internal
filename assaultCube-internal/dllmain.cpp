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

//public:
//    char pad_0000[4];					//0x0000 - 0x04
//    Vector3 HeadPos;					//0x0004 - 0x10
//    Vector3 Velocity;					//0x0010 - 0x1C
//    char pad_001C[12];					//0x001C - 0x28
//    Vector3 PlayerPos;					//0x0028 - 0x34
//    Vector3 ViewAngles;					//0x0034 - 0x40
//    char pad_idk[25];					//0x40 - 0x5D
//    int32_t OnGround;					//0x5D - 0x61
//    char pad_idk2[19];					//0x61 -0x74
//    uint16_t SpeedHacks;				//0x74 - 0x76
//    uint16_t NoClip;					//0x76 - 0x78
//    char pad_0040[116];					//0x78 - 0xEC
//    int32_t PlayerHealth;				//0xEC - 0xF0
//    int32_t Armor;						//0x00F0
//    char pad_00F4[20];					//0x00F4
//    int32_t PistolAmmo2;				//0x0108
//    char pad_010C[16];					//0x010C
//    int32_t AssaultRifleAmmo2;			//0x011C
//    char pad_0120[12];					//0x0120
//    int32_t PistolAmmo1;				//0x012C
//    char pad_0130[16];					//0x0130
//    int32_t AssaultRifleAmmo1;			//0x0140
//    char pad_0144[12];					//0x0144
//    int32_t PistolReloadDelay;			//0x0150
//    char pad_0154[16];					//0x0154
//    int32_t AssaultRifleReloadDelay;	//0x0164
//    char pad_0168[12];					//0x0168
//    int32_t AmountOfShotsFired;			//0x0174
//    char pad_0178[100];					//0x0178
//    int32_t BlueTeamScore;				//0x01DC
//    char pad_01E0[37];					//0x01E0
//    char Name[16];						//0x0205
//    char pad_0215[247];					//0x0215
//    int32_t Team;						//0x030C
//    char pad_030C[8];					//0x314
//    int32_t IsAlive;					//0x318 - 0x31C
//    char pad_0302[76];					//0x368
//    OInventory* Inventory;				//0x368 - 0x37C

// -------------------------------------
// Globals
// -------------------------------------
static HWND       g_GameWindow = nullptr;
static uintptr_t  moduleBase = 0;
static ent* localPlayer = nullptr;
static bool       imguiInitialized = false;
static std::string cachedName = "Loading...";
static bool nameRetrieved = false;
static char setName[32] = "Dummy";
static bool nameInputActive = false;
static int nameInputCursor = 0;

static bool showMenu = true;
static bool bHealth = false;
static bool bAmmo = false;
static bool bRecoil = false;
static bool bNoReload = false;
static bool bNoClip = false;


//delete later:
bool dummy = false;
float dummyfloat;

// Menu navigation
static int currentSelection = 0;
static const int maxSelections = 6; 
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
            // Handle name input when it's active
            if (nameInputActive && currentSelection == 5)
            {
                switch (wParam)
                {
                case VK_ESCAPE:
                    nameInputActive = false;
                    std::cout << "[WndProc] Name input deactivated\n";
                    return 0;
                case VK_RETURN:
                    nameInputActive = false;
                    std::cout << "[WndProc] Name input confirmed\n";
                    return 0;
                case VK_BACK:
                    if (nameInputCursor > 0)
                    {
                        nameInputCursor--;
                        setName[nameInputCursor] = '\0';
                        // Shift remaining characters
                        for (int i = nameInputCursor; i < 31; i++)
                        {
                            setName[i] = setName[i + 1];
                        }
                    }
                    return 0;
                case VK_LEFT:
                    if (nameInputCursor > 0) nameInputCursor--;
                    return 0;
                case VK_RIGHT:
                    if (nameInputCursor < strlen(setName) && nameInputCursor < 31) nameInputCursor++;
                    return 0;
                default:
                    // Handle character input
                    if (wParam >= 32 && wParam <= 126 && nameInputCursor < 31) // Printable ASCII
                    {
                        // Shift characters to make room
                        for (int i = 31; i > nameInputCursor; i--)
                        {
                            setName[i] = setName[i - 1];
                        }
                        setName[nameInputCursor] = (char)wParam;
                        nameInputCursor++;
                        setName[31] = '\0'; // Ensure null termination
                    }
                    return 0;
                }
            }
            else
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
                        if (localPlayer)
                        {
                            localPlayer->player_health = 100;
                            localPlayer->armor_quantity = 100;
                            std::cout << "[WndProc Action] Player healed!\n";
                        }
                        break;
                    case 5:
                        bNoClip = !bNoClip;
                        std::cout << "[WndProc Toggle] NoClip: " << bNoClip << "\n";
                        break;
                    }
                    return 0;
                }
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
        // Try to cache the name if not already retrieved
        if (!nameRetrieved)
        {
            //// Method 1: Try struct field first
            //if (localPlayer->name[0] != '\0' && isprint((unsigned char)localPlayer->name[0]))
            //{
            //    const char* raw = localPlayer->name;
            //    size_t len = 0;
            //    while (len < 19 && raw[len] != '\0' && isprint((unsigned char)raw[len])) {
            //        ++len;
            //    }
            //    if (len > 0) {
            //        cachedName.assign(raw, len);
            //        nameRetrieved = true;
            //        std::cout << "[Cache] Player name retrieved from struct: " << cachedName << "\n";
            //    }
            //}

            // Method 2: Try offset 0x205 if struct field failed
            if (!nameRetrieved)
            {
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
        if (ImGui::BeginTabBar("MainTabs"))
        {
            // Cycle tabs with TAB key
            if (ImGui::IsKeyPressed(ImGuiKey_Tab))
                currentTab = (currentTab + 1) % 3;

            if (ImGui::BeginTabItem("Misc", nullptr,currentTab == 0 ? ImGuiTabItemFlags_SetSelected : 0))    // misc first
            {
                ImGui::Text("Combat Hacks:");

                auto drawOption = [&](int idx, const char* label, bool enabled = false) {
                    if (currentSelection == idx)
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

                    if (idx == 4)
                        ImGui::Text("%s[Heal Player]", currentSelection == idx ? "> " : "  ");
                    else
                        ImGui::Text("%s%s: %s", currentSelection == idx ? "> " : "  ", label, enabled ? "[ON]" : "[OFF]");

                    if (currentSelection == idx)
                        ImGui::PopStyleColor();
                    };

                drawOption(0, "God Mode (Health)", bHealth);
                drawOption(1, "Infinite Ammo", bAmmo);
                drawOption(2, "No Recoil", bRecoil);
                drawOption(3, "No Reload", bNoReload);
                drawOption(4, "");
				drawOption(5, "No Clip", bNoClip);
                ImGui::Spacing();
				ImGui::Separator();
                if (localPlayer)
                {
                    ImGui::Text("HP: %d  | Armor: %d", localPlayer->player_health, localPlayer->armor_quantity);
					ImGui::Spacing();
                    ImGui::Text("Position  -  X: %.1f, Y: %.1f, Z: %.1f", localPlayer->X, localPlayer->Y, localPlayer->Z);
                    ImGui::Text("Head angles  -  Yaw: %.2f, Pitch: %.2f", localPlayer->lookleft_right, localPlayer->lookup_down);
                    ImGui::Spacing();
                    ImGui::Text("Name: %s", cachedName.c_str());
                    ImGui::Text("Kills: %d", localPlayer->number_of_kills);
                    ImGui::NewLine();
					
                }

				// Name changer - will re-implement later
                // 
                //if (currentSelection == 5)            //change global 
                //{
                //    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                //    ImGui::Text("> Change player name: %s%s",
                //        nameInputActive ? setName : setName,
                //        nameInputActive ? "_" : "");
                //    ImGui::PopStyleColor();
                //    if (nameInputActive)
                //    {
                //        ImGui::SameLine();
                //        ImGui::Text("(ESC to exit, ENTER to confirm)");
                //    }
                //}
                //else
                //{
                //    ImGui::Text("  Change player name: %s", setName);
                //}

                //// Apply Name button with keyboard navigation
                //if (currentSelection == 6)
                //{
                //    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                //    ImGui::Text("> [APPLY NAME]");
                //    ImGui::PopStyleColor();
                //}
                //else
                //{
                //    ImGui::Text("  [APPLY NAME]");
                //}


                ImGui::EndTabItem();  
            }

            if (ImGui::BeginTabItem("ESP", nullptr,currentTab == 1 ? ImGuiTabItemFlags_SetSelected : 0)) // esp second
            {
                ImGui::Checkbox("Enable ESP", &dummy);
                ImGui::Checkbox("Draw Boxes", &dummy);
                ImGui::EndTabItem();   
            }

            if (ImGui::BeginTabItem("Aimbot", nullptr,currentTab == 2 ? ImGuiTabItemFlags_SetSelected : 0)) // aimbot last
            {
                ImGui::Checkbox("Enable Aimbot", &dummy);
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