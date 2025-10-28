#pragma once
#include "Entity.h"

struct EntityList_t {
    char pad_0000[4];
    Entity* entities[32];
};