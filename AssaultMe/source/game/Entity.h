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



//class PlayerEnt
//{
//public:
//	char pad_0000[4]; //0x0000
//	Vector3 HeadPos; //0x0004
//	Vector2 VelocityByDirection; //0x0010
//	float JumpPower; //0x0018
//	char pad_001C[12]; //0x001C
//	Vector3 FeetPos; //0x0028
//	Angle ViewAngles; //0x0034
//	char pad_0040[8]; //0x0040
//	int32_t Gravity; //0x0048
//	float CharacterShape1; //0x004C
//	float CharacterHight; //0x0050
//	float CharacterShape2; //0x0054
//	float MovementRelated; //0x0058
//	int32_t IsJumping; //0x005C
//	char pad_0060[1]; //0x0060
//	int8_t IsShifting; //0x0061
//	char pad_0062[2]; //0x0062
//	int32_t IsMoving; //0x0064
//	char pad_0068[14]; //0x0068
//	int8_t PlayerMode; //0x0076
//	char pad_0077[9]; //0x0077
//	int32_t IsWASD; //0x0080
//	char pad_0084[104]; //0x0084
//	int32_t Health; //0x00EC
//	int32_t Armor; //0x00F0
//	char pad_00F4[80]; //0x00F4
//	int32_t Gernade; //0x0144
//	char pad_0148[4]; //0x0148
//	bool IsKnifing; //0x014C
//	char pad_014D[183]; //0x014D
//	int8_t AutoShoot; //0x0204
//	char Name[16]; //0x0205
//	char pad_0215[247]; //0x0215
//	int8_t TeamID; //0x030C
//	char pad_030D[87]; //0x030D
//	class GunEnt* GunEnt; //0x0364
//	char pad_0368[260]; //0x0368
//}; //Size: 0x046C
//
//class GunEnt
//{
//public:
//	char pad_0000[4]; //0x0000
//	int32_t Code; //0x0004
//	class PlayerEnt* Owner; //0x0008
//	class GunData* GeneralData; //0x000C
//	class AmmoData* AmmoData1; //0x0010
//	char pad_0014[8]; //0x0014
//	int32_t Spread; //0x001C
//	char pad_0020[8]; //0x0020
//	int8_t IsScoping; //0x0028
//	char pad_0029[1096]; //0x0029
//}; //Size: 0x0471
//
//class AmmoData
//{
//public:
//	int32_t CurrentMag; //0x0000
//	char pad_0004[32]; //0x0004
//	int32_t CurrentAmmo; //0x0024
//	char pad_0028[68]; //0x0028
//	int32_t BulletsShot; //0x006C
//	char pad_0070[148]; //0x0070
//}; //Size: 0x0104