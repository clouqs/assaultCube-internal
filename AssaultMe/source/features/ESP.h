#pragma once
#include <Windows.h>
#include <gl/GL.h>
#include "../game/Entity.h"
#include "../features/Vec3.h"

struct Color {
    float r, g, b, a;
    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
};

class ESP {
public:
    static ESP& Get();

    void Initialize();
    void Render();
    void Shutdown();

    // ESP Settings
    bool enabled = false;
    bool drawBoxes = true;
    bool drawLines = true;
    bool drawHealth = true;
    bool drawNames = true;
    bool drawDistance = true;
    bool drawSkeletons = false;
    bool showTeam = true;

    // Color settings
    struct Colors {
        Color enemy = Color(1.0f, 0.0f, 0.0f, 1.0f);      // Red
        Color team = Color(0.0f, 1.0f, 0.0f, 1.0f);       // Green
        Color healthBar = Color(0.0f, 1.0f, 0.0f, 1.0f);  // Green
        Color healthBarBG = Color(0.2f, 0.2f, 0.2f, 1.0f); // Dark gray
        Color text = Color(1.0f, 1.0f, 1.0f, 1.0f);       // White
        Color snapline = Color(1.0f, 1.0f, 0.0f, 1.0f);   // Yellow
    } colors;

    // Distance settings
    float maxDistance = 1000.0f;
	float minDistance = 200.0f;
    int fontSize = 14;

private:
    ESP() = default;
    ~ESP() = default;
    ESP(const ESP&) = delete;
    ESP& operator=(const ESP&) = delete;

    // Helper functions
    bool WorldToScreen(const Vec3& worldPos, Vec2& screenPos);
    void DrawBox2D(const Vec2& top, const Vec2& bottom, float width, const Color& color);
    void DrawLine(const Vec2& from, const Vec2& to, const Color& color, float thickness = 1.0f);
    void DrawFilledRect(float x, float y, float w, float h, const Color& color);
    void DrawText(const char* text, float x, float y, const Color& color);
    void DrawHealthBar(float x, float y, float w, float h, int health, int maxHealth);
    void DrawEntity(Entity* entity, Entity* localPlayer);

    // View matrix and screen info
    float viewMatrix[16];
    float projectionMatrix[16];
    int viewport[4];
    int screenWidth = 0;
    int screenHeight = 0;

};