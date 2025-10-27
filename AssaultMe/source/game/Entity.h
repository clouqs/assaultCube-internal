#pragma once
#include <cstdint>

class Entity {
public:
    char   pad_0000[4];
    float  X;
    float  Y;
    float  Z;
    char   pad_0010[36];
    float  lookleft_right;
    float  lookup_down;
    char   pad_003C[176];
    int32_t player_health;
    int32_t armor_quantity;
    char   pad_00F4[20];
    int32_t pistol_stored_ammo;
    char   pad_010C[16];
    int32_t assault_rifle_stored_ammo;
    char   pad_0120[32];
    int32_t assault_rifle_ammo;
    int32_t grenade_number;
    char   pad_0148[8];
    int32_t pistol_shot_reload_delay;
    char   pad_0154[16];
    int64_t assault_rifle_shot_reload_delay;
    char   pad_016C[8];
    int32_t pistol_total_shots;
    char   pad_0178[16];
    int32_t assault_rifle_total_shots;
    char   pad_018C[80];
    int32_t number_of_kills;
    char   pad_01E0[32];
    char   name[19];
    char   pad_0210[264];
    bool   is_dead;
    char   pad_0319[75];
    void* weapon_in_hand;
    char   pad_0368[480];
};
