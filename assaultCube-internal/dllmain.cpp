#include "stdafx.h"
#include <iostream>
#include <string>
#include "mem.h"

#pragma pack(push, 1)
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
    char name[19]; //0x0205
    char pad_0210[264]; //0x0210 (adjusted padding)
    bool is_dead; //0x0318
    char pad_0319[75]; //0x0319
    void* weapon_in_hand; //0x0364 - Cambiato da class N000002EE* a void*
    char pad_0368[480]; //0x0368
}; //Size: 0x0548
#pragma pack(pop)

// Variabili globali necessarie
uintptr_t moduleBase = 0;
ent* localPlayer = nullptr;
bool showBotHealth = false;
HANDLE botConsoleThread = nullptr;

DWORD WINAPI BotConsole(LPVOID)
{
    // Alloca una nuova console separata per i bot
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    // Imposta il titolo della finestra console
    SetConsoleTitle(L"Bot List - assaultMe");

    std::cout << "===== Bot List Console =====\n";
    std::cout << "========= by clouqs =========\n";
    std::cout << "Press [F] to close this console\n";
    std::cout << "=============================\n\n";

    while (showBotHealth) // Changed condition to check the global flag
    {
        // Controlla se F è premuto per chiudere SOLO la console bot
        if (GetAsyncKeyState('F') & 1)
        {
            showBotHealth = false; // Disattiva la visualizzazione bot
            break;
        }

        // Clear screen and redraw header
        system("cls");
        std::cout << "===== Bot List Console =====\n";
        std::cout << "========= by clouqs =========\n";
        std::cout << "Press [F] to close this console\n";
        std::cout << "=============================\n\n";

        // Validate moduleBase before using it
        if (!moduleBase) {
            std::cout << "Module base not initialized!\n";
            Sleep(1000);
            continue;
        }

        // Safe memory reading with error checking
        uintptr_t entityListPtr = 0;
        uintptr_t entityListSize = 0;

        try {
            // Use safer memory reading
            if (!IsBadReadPtr((void*)(moduleBase + 0x18AC04), sizeof(uintptr_t))) {
                entityListPtr = *(uintptr_t*)(moduleBase + 0x18AC04);
            }
            if (!IsBadReadPtr((void*)(moduleBase + 0x18AC0C), sizeof(uintptr_t))) {
                entityListSize = *(uintptr_t*)(moduleBase + 0x18AC0C);
            }
        }
        catch (...) {
            std::cout << "Error reading entity list pointers!\n";
            Sleep(1000);
            continue;
        }

        int botcount = 0;
        int validEntities = 0;

        if (entityListPtr && entityListSize > 0 && entityListSize < 1000) // Add sanity check
        {
            if (IsBadReadPtr((void*)entityListPtr, entityListSize * sizeof(void*))) {
                std::cout << "Invalid entity list pointer!\n";
                Sleep(1000);
                continue;
            }

            ent** entityList = (ent**)entityListPtr;

            for (int i = 1; i < (int)entityListSize; i++)
            {
                if (!showBotHealth) break; // Exit if flag changed during loop

                // Safer entity validation
                ent* pEntity = nullptr;
                try {
                    if (!IsBadReadPtr(&entityList[i], sizeof(ent*))) {
                        pEntity = entityList[i];
                    }
                }
                catch (...) {
                    continue;
                }

                if (!pEntity || pEntity == localPlayer || IsBadReadPtr(pEntity, sizeof(ent)))
                    continue;

                try
                {
                    // Read health safely
                    int health = 0;
                    if (!IsBadReadPtr(&pEntity->player_health, sizeof(int32_t))) {
                        health = pEntity->player_health;
                    }
                    else {
                        continue;
                    }

                    // bot names needs a fix!
                    char safeName[20] = { 0 }; // Initialize with zeros
                    if (!IsBadReadPtr(pEntity->name, sizeof(pEntity->name))) {
                        // Copy byte by byte to handle potential bad memory
                        bool nameValid = false;
                        for (int nameIdx = 0; nameIdx < sizeof(pEntity->name) - 1; nameIdx++) {
                            if (!IsBadReadPtr(&pEntity->name[nameIdx], 1)) {
                                char c = pEntity->name[nameIdx];
                                if (c == '\0') {
                                    break; // End of string
                                }
                                if (c >= 32 && c <= 126) { // Printable ASCII
                                    safeName[nameIdx] = c;
                                    nameValid = true;
                                }
                                else {
                                    safeName[nameIdx] = '_'; // Replace invalid chars
                                }
                            }
                            else {
                                break; // Stop on bad memory
                            }
                        }
                        safeName[19] = '\0'; // Always null terminate

                        // If no valid characters found, use default name
                        if (!nameValid || safeName[0] == '\0') {
                            strcpy_s(safeName, "Bot");
                        }
                    }
                    else {
                        strcpy_s(safeName, "Unknown");
                    }

                    validEntities++;

                    // Check if health is in reasonable range
                    if (health > 0 && health <= 1000) // Increased upper limit for modded health
                    {
                        std::cout << "Bot #" << i << " | Name: " << safeName << " | Health: " << health << " | ALIVE\n";
                        botcount++;
                    }
                    else if (health <= 0)
                    {
                        std::cout << "Bot #" << i << " | Name: " << safeName << " | DEAD\n";
                    }
                    else
                    {
                        // Skip entities with unreasonable health values (likely invalid)
                        continue;
                    }
                }
                catch (...)
                {
                    std::cout << "Bot #" << i << " | Error reading entity data\n";
                }
            }

            std::cout << "\n=============================\n";
            std::cout << "Total bots alive: " << botcount << "\n";
            std::cout << "Valid entities processed: " << validEntities << "\n";
            std::cout << "Total entities: " << (int)entityListSize - 1 << " (excluding player)\n";
        }
        else
        {
            std::cout << "Entity list not found, empty, or invalid size!\n";
            std::cout << "EntityListPtr: 0x" << std::hex << entityListPtr << std::dec << "\n";
            std::cout << "EntityListSize: " << entityListSize << "\n";
        }

        Sleep(1000); // Increased sleep time to reduce flicker
    }

    // Cleanup
    if (f) fclose(f);
    FreeConsole();
    botConsoleThread = nullptr; // Reset thread handle
    return 0;
}

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
    std::cout << "[F6] Show Bot list\n";
    std::cout << "================================\n";
    std::cout << "[INS] Exit\n";
    std::cout << "\n";
    std::cout << "\n";
    std::cout << R"(
   ____ _                 _     
  / ___| | ___  _   _  __| |___ 
 | |   | |/ _ \| | | |/ _` / __|
 | |___| | (_) | |_| | (_| \__ \                       
  \____|_|\___/ \__,_|\__,_|___/
)" << std::endl;

    moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
    if (!moduleBase) {
        moduleBase = (uintptr_t)GetModuleHandle(NULL);
    }

    bool bHealth = false, bAmmo = false, bRecoil = false, bNoReload = false;
    bool updateDisplay = false;

    while (true)
    {
        localPlayer = *(ent**)(moduleBase + 0x0017E0A8); // Aggiornato localPlayer globale

        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            // Clean shutdown of bot console if it's running
            if (showBotHealth) {
                showBotHealth = false;
                if (botConsoleThread) {
                    WaitForSingleObject(botConsoleThread, 2000); // Wait up to 2 seconds
                    CloseHandle(botConsoleThread);
                    botConsoleThread = nullptr;
                }
            }
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

        if (GetAsyncKeyState(VK_F6) & 1)
        {
            if (!showBotHealth && !botConsoleThread) {
                // Start bot console
                showBotHealth = true;
                botConsoleThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)BotConsole, nullptr, 0, nullptr);
                updateDisplay = true;
            }
            else if (showBotHealth) {
                // Stop bot console
                showBotHealth = false;
                updateDisplay = true;
                // The thread will terminate itself when showBotHealth becomes false
            }
        }

        if (updateDisplay)
        {
            system("cls");
            std::cout << "===== assaultMe - internal =====\n";
            std::cout << "[F1]  Health Hack : <" << (bHealth ? "ON" : "OFF") << ">\n";
            std::cout << "[F2]  Ammo Hack   : <" << (bAmmo ? "ON" : "OFF") << ">\n";
            std::cout << "[F3]  No Recoil   : <" << (bRecoil ? "ON" : "OFF") << ">\n";
            std::cout << "[F4]  No Reload   : <" << (bNoReload ? "ON" : "OFF") << ">\n";
            std::cout << "[F5]  Add 10 Grenades\n";
            std::cout << "[F6] Show Bot list : <" << (showBotHealth ? "ON" : "OFF") << ">\n";
            std::cout << "================================\n";
            std::cout << "[INS] Exit\n";

            updateDisplay = false;
        }

        if (localPlayer)
        {
            if (bHealth)
            {
                localPlayer->player_health = 1000;
            }

            if (bAmmo)
            {
                mem::Nop((BYTE*)(moduleBase + 0xC73EF), 2);
            }
            else
            {
                mem::Patch((BYTE*)(moduleBase + 0xC73EF), (BYTE*)"\xFF\x08", 2);
            }

            if (bNoReload)
            {
                mem::Nop((BYTE*)(moduleBase + 0xC8FC7), 2);
            }
            else
            {
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

    if (f) fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}