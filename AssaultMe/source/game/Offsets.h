#pragma once
#include <cstdint>

namespace Offsets {
    constexpr uintptr_t LocalPlayer = 0x0017E0A8;
    constexpr uintptr_t FOV = 0x18A7CC;
    constexpr uintptr_t VelocityX = 0x10;
    constexpr uintptr_t VelocityY = 0x14;
    constexpr uintptr_t PlayerName = 0x205;
    constexpr uintptr_t RapidFire = 0x164;
    constexpr uintptr_t LocalPlayerCheck = 0x104;
    constexpr uintptr_t NoClipFlag = 0x76;

    // Patch offsets
    constexpr uintptr_t InfiniteAmmoPatch = 0xC73EF;
    constexpr uintptr_t NoReloadPatch = 0xC8FC7;
    constexpr uintptr_t NoRecoilPatch = 0xC2EC3;
}