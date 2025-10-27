#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include "../game/Entity.h"

// Forward declarations - these will be defined in dllmain.cpp
extern uintptr_t moduleBase;
extern Entity* localPlayer;

// CubeScript function signatures
typedef void(__cdecl* CubeScriptMain)(int type, char* name, char* action);
typedef void(__cdecl* CubeScriptSimple)(char* command);

// CubeScript globals - these will be defined in cubescript.cpp
extern CubeScriptMain g_CubeExecMain;
extern CubeScriptSimple g_CubeExecSimple;

// CubeScript function declarations
bool InitializeCubeScript();
bool ExecuteCubeScriptSafe(const char* command, const char* args = "");
//void TestCubeScriptCommands();    //test array
bool ExecuteSuicide();
//bool SetPlayerNameDirect(const char* newName);
//bool SimulateConsoleCommand(const char* command);
bool ChangeName(); //specificare argomento come input da imgui inputtext field.
//bool ChangeFov(const char* newFov);
//bool ChangeSensitivity(const char* newSens);
//bool ChangeGamma(const char* newGamma);
