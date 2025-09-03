#include <windows.h>
#include "imgui/imgui.h"

// -------------------------------------
// Globals
// -------------------------------------
static HWND       g_GameWindow = nullptr;
static uintptr_t  moduleBase = 0;
static ent* localPlayer = nullptr;
static BYTE localPlayer_check = 0;
static bool       imguiInitialized = false;
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
static float bFov = 90;

//CubeScript globals
const char* pattern = "\x8B\x54\x24\x04\x81\xEC\x14\x02\x00\x00\x83\x3D\xD4\xF0\x57\x00"
"\x00\x56\x74\x11\x0F\xB6\xC2\x83\xF8\x01\x74\x09\x83\xF8\x03\x0F"
"\x85\x09\x02\x00\x00\x8B\x0D\xF8\xAB\x58\x00\xF7\xC2\x00\x01\x00"
"\x00\x74";

const char* mask = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";


//Dummy Globals
bool dummy = false;
float dummyfloat;
static float currentSpeed = 0.0f; // Added missing variable

// Menu navigation
static int currentSelection = 0;
static const int MainCheatSelections = 7;
static const int ESPSelections = 2;
static int currentTab = 0;

// OpenGL hooks
using wglSwapBuffersFn = BOOL(WINAPI*)(HDC);
static wglSwapBuffersFn owglSwapBuffers = nullptr;

// ImGui Win32 WndProc
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
static WNDPROC oWndProc = nullptr;
