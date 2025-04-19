#include "stdafx.h"
#include <iostream>
#include "mem.h"

// https://guidedhacking.com/threads/how-to-hack-any-game-first-internal-hack-dll-tutorial.12142/

DWORD WINAPI HackThread(HMODULE hModule)
{
    // Create Console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "assaultMe - internal\n";

    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");

    // calling it with NULL also gives you the address of the .exe module
    moduleBase = (uintptr_t)GetModuleHandle(NULL);

    bool bHealth = false, bAmmo = false, bRecoil = false;

    while (true)
    {
        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            break;
        }

        if (GetAsyncKeyState(VK_F1) & 1)
            bHealth = !bHealth;

        if (GetAsyncKeyState(VK_F2) & 1)
        {
            bAmmo = !bAmmo;
        }

        // no recoil NOP
        if (GetAsyncKeyState(VK_F3) & 1)
        {
            bRecoil = !bRecoil;

            if (bRecoil)
            {
                mem::Nop((BYTE*)(moduleBase + 0xC2EC3), 5);
            }
            else
            {
                // 50 8D 4C 24 1C 51 8B CE FF D2 the original stack setup and call
                mem::Patch((BYTE*)(moduleBase + 0xC2EC3), (BYTE*)"\xF3\x0F\x11\x56\x38", 5);
            }
        }

        uintptr_t* localPlayerPtr = (uintptr_t*)(moduleBase + 0x0017E0A8);

        // continuous writes / freeze
        if (localPlayerPtr)
        {
            if (bHealth)
            {
                *(int*)(*localPlayerPtr + 0xEC) = 1337;
            }

            if (bAmmo)
            {
                uintptr_t ammoAddr = mem::FindDMAAddy(moduleBase + 0x0017E0A8, { 0x140 });
                int* ammo = (int*)ammoAddr;
                *ammo = 1337;

                *(int*)mem::FindDMAAddy(moduleBase + 0x0017E0A8, { 0x140 }) = 1337;
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
