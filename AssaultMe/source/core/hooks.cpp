#include "Hooks.h"
#include "../game/GameState.h"
#include "../features/logic.h"
#include "../menu/menu.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/imgui_impl_win32.h"
#include "../../external/imgui/imgui_impl_opengl2.h"
#include "../../external/minhook/MinHook.h"
#include <gl/GL.h>
#include <iostream>

#pragma comment(lib, "opengl32.lib")

// ============================================
// Static variables
// ============================================
namespace {
    using wglSwapBuffersFn = BOOL(WINAPI*)(HDC);

    wglSwapBuffersFn originalWglSwapBuffers = nullptr;
    WNDPROC originalWndProc = nullptr;
    HWND gameWindow = nullptr;
    bool imguiInitialized = false;
}

// Forward declaration for ImGui
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

// ============================================
// WndProc Hook
// ============================================
LRESULT WINAPI Hooks::WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Toggle menu with INSERT key
    if (msg == WM_KEYDOWN && wParam == VK_INSERT) {
        Menu::Get().Toggle();
        std::cout << "[WndProc] Menu toggled: " << Menu::Get().IsVisible() << "\n";
        return 0;
    }

    // Pass keyboard input to menu if it's visible
    if (Menu::Get().IsVisible() && msg == WM_KEYDOWN) {
        Menu::Get().HandleKeyInput(wParam);
        return 0;
    }

    // Pass input to ImGui if menu is visible
    if (Menu::Get().IsVisible() && imguiInitialized) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
            return 1;
        }
    }

    // Call original window procedure
    return CallWindowProcW(originalWndProc, hWnd, msg, wParam, lParam);
}

// ============================================
// wglSwapBuffers Hook
// ============================================
BOOL WINAPI Hooks::wglSwapBuffersHook(HDC hdc) {
    // Initialize ImGui on first call
    if (!imguiInitialized) {
        gameWindow = WindowFromDC(hdc);
        if (!gameWindow) {
            return originalWglSwapBuffers(hdc);
        }

        // Create ImGui context
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        io.IniFilename = nullptr; // Don't save .ini file

        // Setup style
        ImGui::StyleColorsDark();

        // Initialize ImGui backends
        ImGui_ImplWin32_Init(gameWindow);
        ImGui_ImplOpenGL2_Init();

        // Hook window procedure
        originalWndProc = (WNDPROC)SetWindowLongPtrW(
            gameWindow,
            GWLP_WNDPROC,
            (LONG_PTR)WndProcHook
        );

        // Initialize menu
        Menu::Get().Initialize(gameWindow);

        imguiInitialized = true;
        std::cout << "[Hooks] ImGui initialized successfully\n";
    }

    // Update game state (reads pointers from memory)
    GameState::Get().Update();

    // Update all cheats
    CheatManager::Get().Update();

    // Save OpenGL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    // Start ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Render menu
    if (Menu::Get().IsVisible()) {
        Menu::Get().Render();
    }

    // End ImGui frame and render
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Restore OpenGL state
    glPopClientAttrib();
    glPopAttrib();

    // Call original function
    return originalWglSwapBuffers(hdc);
}

// ============================================
// Initialize hooks
// ============================================
bool Hooks::Initialize() {
    // Get OpenGL32 module
    HMODULE hOpenGL32 = GetModuleHandleA("opengl32.dll");
    if (!hOpenGL32) {
        std::cout << "[Hooks] Failed to get opengl32.dll handle\n";
        return false;
    }

    // Get wglSwapBuffers address
    void* wglSwapBuffersAddr = GetProcAddress(hOpenGL32, "wglSwapBuffers");
    if (!wglSwapBuffersAddr) {
        std::cout << "[Hooks] Failed to get wglSwapBuffers address\n";
        return false;
    }

    // Initialize MinHook
    if (MH_Initialize() != MH_OK) {
        std::cout << "[Hooks] Failed to initialize MinHook\n";
        return false;
    }

    // Create hook for wglSwapBuffers
    if (MH_CreateHook(
        wglSwapBuffersAddr,
        &wglSwapBuffersHook,
        reinterpret_cast<LPVOID*>(&originalWglSwapBuffers)
    ) != MH_OK) {
        std::cout << "[Hooks] Failed to create wglSwapBuffers hook\n";
        return false;
    }

    // Enable hook
    if (MH_EnableHook(wglSwapBuffersAddr) != MH_OK) {
        std::cout << "[Hooks] Failed to enable wglSwapBuffers hook\n";
        return false;
    }

    std::cout << "[Hooks] wglSwapBuffers hooked at " << wglSwapBuffersAddr << "\n";
    return true;
}

// ============================================
// Cleanup hooks
// ============================================
void Hooks::Shutdown() {
    // Restore original WndProc
    if (gameWindow && originalWndProc) {
        SetWindowLongPtrW(gameWindow, GWLP_WNDPROC, (LONG_PTR)originalWndProc);
    }

    // Shutdown ImGui
    if (imguiInitialized) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        imguiInitialized = false;
    }

    // Disable and remove all MinHook hooks
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    std::cout << "[Hooks] Shutdown complete\n";
}