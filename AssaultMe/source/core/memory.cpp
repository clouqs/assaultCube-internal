#include "memory.h"
#include <cstring> 
#include <Windows.h>
#include <psapi.h>
// ============================================
// Patch memory (internal)
// ============================================
void Memory::Patch(BYTE* dst, BYTE* src, unsigned int size) {
    DWORD oldProtect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldProtect, &oldProtect);
}

// ============================================
// Patch memory (external)
// ============================================
void Memory::PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess) {
    DWORD oldProtect;
    VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    WriteProcessMemory(hProcess, dst, src, size, nullptr);
    VirtualProtectEx(hProcess, dst, size, oldProtect, &oldProtect);
}

// ============================================
// NOP bytes (internal)
// ============================================
void Memory::Nop(BYTE* dst, unsigned int size) {
    DWORD oldProtect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memset(dst, 0x90, size);
    VirtualProtect(dst, size, oldProtect, &oldProtect);
}

// ============================================
// NOP bytes (external)
// ============================================
void Memory::NopEx(BYTE* dst, unsigned int size, HANDLE hProcess) {
    BYTE* nopArray = new BYTE[size];
    memset(nopArray, 0x90, size);
    PatchEx(dst, nopArray, size, hProcess);
    delete[] nopArray;
}

// ============================================
// Follow pointer chain
// ============================================
uintptr_t Memory::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets) {
    uintptr_t addr = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i) {
        addr = *(uintptr_t*)addr;
        addr += offsets[i];
    }
    return addr;
}

// ============================================
// Pattern scanning
// ============================================
uintptr_t Memory::FindPattern(const char* moduleName, const char* pattern, const char* mask) {
    MODULEINFO mInfo{};
    HMODULE hModule = GetModuleHandleA(moduleName);
    if (!hModule) return 0;

    if (!GetModuleInformation(GetCurrentProcess(), hModule, &mInfo, sizeof(MODULEINFO))) {
        return 0;
    }

    uintptr_t base = reinterpret_cast<uintptr_t>(hModule);
    size_t size = mInfo.SizeOfImage;
    size_t patternLength = strlen(mask);

    for (size_t i = 0; i < size - patternLength; i++) {
        bool found = true;
        for (size_t j = 0; j < patternLength; j++) {
            if (mask[j] != '?' && pattern[j] != *(char*)(base + i + j)) {
                found = false;
                break;
            }
        }
        if (found) {
            return base + i;
        }
    }

    return 0;
}