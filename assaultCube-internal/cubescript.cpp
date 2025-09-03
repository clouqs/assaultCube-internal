#include "cubescript.h"
#include "mem.h"
#include <iostream>
#include <cstring>
#include <windows.h>

// === Globals ===
CubeScriptMain   g_CubeExecMain = nullptr;
CubeScriptSimple g_CubeExecSimple = nullptr;

// Pattern
constexpr const char* kPattern =
"\x8B\x54\x24\x04\x81\xEC\x14\x02\x00\x00\x83\x3D\xD4\xF0\x57\x00"
"\x00\x56\x74\x11\x0F\xB6\xC2\x83\xF8\x01\x74\x09\x83\xF8\x03\x0F"
"\x85\x09\x02\x00\x00\x8B\x0D\xF8\xAB\x58\x00\xF7\xC2\x00\x01\x00"
"\x00\x74";
constexpr const char* kMask = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

// === Helpers ===
static bool ValidateExecutable(uintptr_t addr) {
    MEMORY_BASIC_INFORMATION mbi{};
    if (VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi))) {
        return mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE);
    }
    return false;
}

// === API ===
bool InitializeCubeScript() {
    std::cout << "[CubeScript] Initializing...\n";

    uintptr_t execAddr = FindPattern("ac_client.exe", kPattern, kMask);
    if (!execAddr) {
        std::cout << "[CubeScript] Pattern not found!\n";
        return false;
    }

    g_CubeExecMain = reinterpret_cast<CubeScriptMain>(execAddr);
    g_CubeExecSimple = reinterpret_cast<CubeScriptSimple>(execAddr);

    std::cout << "[CubeScript] Found function at 0x" << std::hex << execAddr << std::dec << "\n";
    if (!ValidateExecutable(execAddr)) {
        std::cout << "[CubeScript] WARNING: Memory might not be executable\n";
    }
    return true;
}

bool ExecuteCubeScriptSafe(const char* command, const char* args) {
    if (!command) return false;

    try {
        if (g_CubeExecMain) {
            char cmd[64] = { 0 };
            char arg[64] = { 0 };
            strncpy_s(cmd, sizeof(cmd), command ? command : "", _TRUNCATE);
            strncpy_s(arg, sizeof(arg), args ? args : "", _TRUNCATE);

            int argc = (arg[0] != '\0') ? 2 : 0;
            g_CubeExecMain(argc, cmd, arg);
            return true;
        }

        if (g_CubeExecSimple) {
            char full[128];
            if (args && strlen(args) > 0)
                snprintf(full, sizeof(full), "%s %s", command, args);
            else
                strncpy_s(full, command, _TRUNCATE);

            g_CubeExecSimple(full);
            return true;
        }
    }
    catch (...) {
        std::cout << "[CubeScript] Exception during execution\n";
    }
    return false;
}

                // test commands 
                
//void TestCubeScriptCommands() {
//    struct Test { const char* cmd; const char* arg; const char* desc; };
//    Test tests[] = {
//        {"fov", "120", "Change FOV"},
//        {"gamma", "150", "Increase brightness"},
//        {"name", "TestName", "Change player name"},
//        {"sensitivity", "5", "Change mouse sensitivity"},
//        {"crosshairsize", "20", "Make crosshair bigger"},
//        {nullptr, nullptr, nullptr}
//    };
//
//    for (int i = 0; tests[i].cmd; ++i) {
//        std::cout << "[Test " << (i + 1) << "] " << tests[i].desc << "\n";
//        ExecuteCubeScriptSafe(tests[i].cmd, tests[i].arg);
//        Sleep(500);
//    }
//}

// === Wrappers for common commands ===
bool ExecuteSuicide() {
    return ExecuteCubeScriptSafe("suicide", "");
}

bool ChangeName() {   //specificare argomento come input da imgui inputtext field.
    return ExecuteCubeScriptSafe("name", "maxyboo");
}
//
//bool ChangeFov(const char* newFov) {
//    return ExecuteCubeScriptSafe("fov", newFov);
//}
//
//bool ChangeSensitivity(const char* newSens) {
//    return ExecuteCubeScriptSafe("sensitivity", newSens);
//}
//
//bool ChangeGamma(const char* gamma) {
//    return ExecuteCubeScriptSafe("gamma", gamma);
//}
