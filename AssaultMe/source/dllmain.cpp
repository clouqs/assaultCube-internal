#include <Windows.h>
#include <iostream>
#include "core/Hooks.h"
#include "game/GameState.h"
#include "features/logic.h"
#include "features/CubeScript.h"

DWORD WINAPI MainThread(HMODULE hModule) {
    // Setup console
    AllocConsole();
    FILE* f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);

    // Get module base
    uintptr_t moduleBase = (uintptr_t)GetModuleHandleW(L"ac_client.exe");
    if (!moduleBase) {
        std::cout << "[ERROR] Could not find ac_client.exe!\n";
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }
    std::cout << "Module Base: 0x" << std::hex << moduleBase << std::dec << "\n";

    // Initialize game state
    GameState::Get().Initialize(moduleBase);

    // Wait for OpenGL
    while (!GetModuleHandleA("opengl32.dll")) {
        std::cout << "[Wait] Waiting for OpenGL...\n";
        Sleep(1000);
    }

    // Hook OpenGL
    if (!Hooks::Initialize()) {
        std::cout << "[ERROR] Failed to initialize hooks!\n";
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    // Wait for game init
    Sleep(10000);

    // Initialize CubeScript
    if (InitializeCubeScript()) {
        std::cout << "[CubeScript] Initialized\n";
    }

    // Main loop
    while (!(GetAsyncKeyState(VK_END) & 1)) {
        Sleep(16);
    }

    // Cleanup
    Hooks::Shutdown();
    if (f) fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread,
            hModule, 0, nullptr);
        if (hThread) CloseHandle(hThread);
    }
    return TRUE;
}