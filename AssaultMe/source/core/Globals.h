#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include "../game/Entity.h"
// -------------------------------------
// Globals
// -------------------------------------
static HWND g_GameWindow = nullptr;
uintptr_t moduleBase = 0;
Entity* localPlayer = nullptr;
float* fovPtr = nullptr;
float* velocity_x = nullptr;
float* velocity_y = nullptr;
char* namePtr = nullptr;
int64_t* bRapidFirePtr = nullptr;
//float* fovPtr = (float*)((uintptr_t)moduleBase + 0x18A7CC);
//float* velocity_x = (float*)((uintptr_t)localPlayer + 0x10);
//float* velocity_y = (float*)((uintptr_t)localPlayer + 0x14);
//char* namePtr = (char*)(localPlayer)+0x205;
//float* bRapidFirePtr = (float*)((uintptr_t)localPlayer + 0x164);

static BYTE localPlayer_check = 0;
static bool imguiInitialized = false;
static std::string cachedName = "Loading...";
static bool nameRetrieved = false;
static char setName[32] = "Dummy";

static bool showMenu = true;
static bool bHealth = false;
static bool bAmmo = false;
static bool bRecoil = false;
static bool bNoReload = false;
static bool bNoClip = false;
static bool bSpeed = false;
static bool bHealed = false;
static bool bSuicide = false;
static bool bName = false;
static bool bRapidFireEnabled = false;  // Toggle for rapid fire on/off
static int64_t bRapidFire = 60;
static float bFov = 90;

//Dummy Globals
bool dummy = false;
float dummyfloat;
static float currentSpeed = 0.0f;

// Menu navigation

static int currentTab = 0;
static int currentSelection = 0;
static const int MainCheatSelections = 10;  //Main tab has 9 options
static const int VisualsSelections = 1;   //Visuals tab has 1 option (Apply FOV)
static const int ESPSelections = 2;      //ESP tab has 2 options
static const int AimbotSelections = 2;  //Aimbot tab has 2 options