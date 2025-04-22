#include "stdafx.h"
#include <iostream>
#include "mem.h"





// Created with ReClass.NET 1.2 by KN4CK3R

class ent
{
public:
    char pad_0000[4]; //0x0000
    float X; //0x0004
    float Y; //0x0008
    float Z; //0x000C
    char pad_0010[36]; //0x0010
    float lookleft_right; //0x0034
    float lookup_down; //0x0038
    char pad_003C[176]; //0x003C
    int32_t player_health; //0x00EC
    int32_t armor_quantity; //0x00F0
    char pad_00F4[20]; //0x00F4
    int32_t pistol_stored_ammo; //0x0108
    char pad_010C[16]; //0x010C
    int32_t assault_rifle_stored_ammo; //0x011C
    char pad_0120[32]; //0x0120
    int32_t assault_rifle_ammo; //0x0140
    int32_t grenade_number; //0x0144
    char pad_0148[8]; //0x0148
    int32_t pistol_shot_reload_delay; //0x0150
    char pad_0154[16]; //0x0154
    int64_t assault_rifle_shot_reload_delay; //0x0164
    char pad_016C[8]; //0x016C
    int32_t pistol_total_shots; //0x0174
    char pad_0178[16]; //0x0178
    int32_t assault_rifle_total_shots; //0x0188
    char pad_018C[80]; //0x018C
    int32_t number_of_kills; //0x01DC
    char pad_01E0[32]; //0x01E0
    //char name ? [4]; //0x0200
    //char name1[4]; //0x0204
    //char name2[4]; //0x0208
    char pad_020C[268]; //0x020C
    bool is_dead; //0x0318
    char pad_0319[75]; //0x0319
    class N000002EE* weapon_in_hand; //0x0364
    char pad_0368[480]; //0x0368
}; //Size: 0x0548


class N00000291
{
public:
    char pad_0000[68]; //0x0000
}; //Size: 0x0044
static_assert(sizeof(N00000291) == 0x44);

class N000002EE
{
public:
    char pad_0000[68]; //0x0000
}; //Size: 0x0044
static_assert(sizeof(N000002EE) == 0x44);



//above is for padding


DWORD WINAPI HackThread(HMODULE hModule)
{
    // Create Console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "===== assaultMe - internal =====\n";
    std::cout << "=========== by clouqs ===========\n";
    std::cout << "[F1]  Health Hack : <OFF>\n";
    std::cout << "[F2]  Ammo Hack   : <OFF>\n";
    std::cout << "[F3]  No Recoil   : <OFF>\n";
    std::cout << "[F4]  No Reload   : <OFF>\n";
    std::cout << "[F5]  Add 10 Grenades\n";
    std::cout << "================================\n"
    std::cout << "[INS] Exit\n";
    std::cout << "\n";
    std::cout << "\n";
    std::cout << R"(

   ____ _                 _     
  / ___| | ___  _   _  __| |___ 
 | |   | |/ _ \| | | |/ _` / __|
 | |___| | (_) | |_| | (_| \__ \                       
  \____|_|\___/ \__,_|\__,_|___/

)" << std::endl; //idk it just looks cool


    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
    moduleBase = (uintptr_t)GetModuleHandle(NULL);

    bool bHealth = false, bAmmo = false, bRecoil = false, bNoReload = false,
        bGrenade = false;
        
    //random declarations
    bool updateDisplay = false;
    while (true)
    {
        ent* localPlayer = *(ent**)(moduleBase + 0x0017E0A8);
        
        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            break;
        }
        if (GetAsyncKeyState(VK_F1) & 1)
        {
            bHealth = !bHealth;
            updateDisplay = true;
        }

        if (GetAsyncKeyState(VK_F2) & 1)
        {
            bAmmo = !bAmmo;
            updateDisplay = true;
        }

        if (GetAsyncKeyState(VK_F3) & 1)
        {
            bRecoil = !bRecoil;
            updateDisplay = true;

        }
        if (GetAsyncKeyState(VK_F4) & 1)
        {
            bNoReload = !bNoReload;
            updateDisplay = true;
        }
        if (GetAsyncKeyState(VK_F5) & 1)
        {
            if (localPlayer) {
                localPlayer->grenade_number += 10;
                updateDisplay = true;
            }
        }

        if (localPlayer)
        {
            if (updateDisplay)
            {
                system("cls");
                std::cout << "===== assaultMe - internal =====\n";
                std::cout << "[F1]  Health Hack : <" << (bHealth ? "ON" : "OFF") << ">\n";
                std::cout << "[F2]  Ammo Hack   : <" << (bAmmo ? "ON" : "OFF") << ">\n";
                std::cout << "[F3]  No Recoil   : <" << (bRecoil ? "ON" : "OFF") << ">\n";
                std::cout << "[F4]  No Reload   : <" << (bNoReload ? "ON" : "OFF") << ">\n";
                std::cout << "[F5]  Add 10 Grenades\n";
                std::cout << "================================\n";
                std::cout << "[INS] Exit\n";

                updateDisplay = false;
            }
            if (bHealth)
            {
                localPlayer->player_health = 1000;
            }
            else 
            {
				localPlayer->player_health = 100;
            }

            if (bAmmo)
            {
                mem::Nop((BYTE*)(moduleBase + 0xC73EF), 2);
            }
            else {
                mem::Patch((BYTE*)(moduleBase + 0xC73EF), (BYTE*)"\xFF\x08", 2);
            }

            if (bNoReload)
            {
                mem::Nop((BYTE*)(moduleBase + 0xC8FC7), 2);
            }
            else {
                mem::Patch((BYTE*)(moduleBase + 0xC8FC7), (BYTE*)"\x01\x01", 2);
            }
            if (bRecoil)
            {
                mem::Nop((BYTE*)(moduleBase + 0xC2EC3), 5);
            }
            else
            {
                mem::Patch((BYTE*)(moduleBase + 0xC2EC3), (BYTE*)"\xF3\x0F\x11\x56\x38", 5);
            }
            
            
        }
        Sleep(5);
        
    }
    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Correct thread creation without CloseHandle
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
