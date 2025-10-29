#pragma once
#include <Windows.h>

class Hooks {
public:
    static bool Initialize();
    static void Shutdown();

private:
    static BOOL WINAPI wglSwapBuffersHook(HDC hdc);
    static LRESULT WINAPI WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};