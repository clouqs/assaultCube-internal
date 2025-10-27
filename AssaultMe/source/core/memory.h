#pragma once
#include <Windows.h>
#include <vector>
#include <cstdint>

namespace Memory {
    // Patch memory bytes (internal process)
    void Patch(BYTE* dst, BYTE* src, unsigned int size);

    // Patch memory bytes (external process)
    void PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess);

    // NOP (0x90) bytes (internal process)
    void Nop(BYTE* dst, unsigned int size);

    // NOP bytes (external process)
    void NopEx(BYTE* dst, unsigned int size, HANDLE hProcess);

    // Follow a pointer chain (DMA - Dynamic Memory Address)
    uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);

    // Scan for byte pattern in module
    uintptr_t FindPattern(const char* moduleName, const char* pattern, const char* mask);
}