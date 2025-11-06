#pragma once
#include <cstdint>
#include "../features/Vec3.h"


class Entity {
public:
    char   pad_0000[4];                    // 0x0000 - 0x0003: 4 bytes padding
    Vec3   HeadPos;                        // 0x0004 - 0x000F: Head position (X, Y, Z)
    char   pad_0010[24];                   // 0x0010 - 0x0027: 24 bytes padding
    Vec3   FeetPos;                        // 0x0028 - 0x0033: Feet/Body position (X, Y, Z)
    float  lookleft_right;                 // 0x0034 - 0x0037: Horizontal look direction
    float  lookup_down;                    // 0x0038 - 0x003B: Vertical look direction
    char   pad_003C[176];                  // 0x003C - 0x00EB: 176 bytes padding
    int32_t player_health;                 // 0x00EC - 0x00EF: Player health value
    int32_t armor_quantity;                // 0x00F0 - 0x00F3: Armor quantity
    char   pad_00F4[20];                   // 0x00F4 - 0x0107: 20 bytes padding
    int32_t pistol_stored_ammo;            // 0x0108 - 0x010B: Stored pistol ammo
    char   pad_010C[16];                   // 0x010C - 0x011B: 16 bytes padding
    int32_t assault_rifle_stored_ammo;     // 0x011C - 0x011F: Stored assault rifle ammo
    char   pad_0120[32];                   // 0x0120 - 0x013F: 32 bytes padding
    int32_t assault_rifle_ammo;            // 0x0140 - 0x0143: Current assault rifle ammo
    int32_t grenade_number;                // 0x0144 - 0x0147: Number of grenades
    char   pad_0148[8];                    // 0x0148 - 0x014F: 8 bytes padding
    int32_t pistol_shot_reload_delay;      // 0x0150 - 0x0153: Pistol shot/reload delay
    char   pad_0154[16];                   // 0x0154 - 0x0163: 16 bytes padding
    int64_t assault_rifle_shot_reload_delay; // 0x0164 - 0x016B: Assault rifle shot/reload delay
    char   pad_016C[8];                    // 0x016C - 0x0173: 8 bytes padding
    int32_t pistol_total_shots;            // 0x0174 - 0x0177: Total pistol shots fired
    char   pad_0178[16];                   // 0x0178 - 0x0187: 16 bytes padding
    int32_t assault_rifle_total_shots;     // 0x0188 - 0x018B: Total assault rifle shots fired
    char   pad_018C[80];                   // 0x018C - 0x01DB: 80 bytes padding
    int32_t number_of_kills;               // 0x01DC - 0x01DF: Number of kills
    char   pad_01E0[32];                   // 0x01E0 - 0x01FF: 32 bytes padding
    char   name[19];                       // 0x0200 - 0x0212: Player name (19 characters)
    char   pad_0213[249];                  // 0x0213 - 0x030B: 249 bytes padding (adjusted)
    int8_t TeamID;                         // 0x030C - 0x030C: Team ID
    char   pad_030D[14];                   // 0x030D - 0x031A: 14 bytes padding (adjusted)
    bool   is_dead;                        // 0x031B - 0x031B: Is dead flag
    char   pad_031C[75];                   // 0x031C - 0x0366: 75 bytes padding
    void* weapon_in_hand;                  // 0x0367 - 0x036E: Pointer to current weapon
    char   pad_036F[480];                  // 0x036F - 0x054E: 480 bytes padding
};


