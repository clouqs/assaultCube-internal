#include "ESP.h"
#include "../game/GameState.h"
#include "../features/DistanceCalc.h"
#include "../../external/imgui/imgui.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <gl/GL.h>
#include <iostream>

#pragma comment(lib, "opengl32.lib")

ESP& ESP::Get() {
    static ESP instance;
    return instance;
}

void ESP::Initialize() {
    glGetIntegerv(GL_VIEWPORT, viewport);
    screenWidth = viewport[2];
    screenHeight = viewport[3];
    std::cout << "[ESP] Initialized\n";
}

void ESP::Render() {
    if (!enabled) return;
    auto& gameState = GameState::Get();
    Entity* localPlayer = gameState.GetLocalPlayer();
    EntityList_t* entityList = gameState.GetEntityList();
    uintptr_t moduleBase = gameState.GetModuleBase();

    if (!localPlayer || !entityList || !moduleBase) return;

    glGetIntegerv(GL_VIEWPORT, viewport);
    screenWidth = viewport[2];
    screenHeight = viewport[3];

    // Read view matrix - NO TRANSPOSE
    __try {
        float* gameViewMatrix = (float*)(moduleBase + 0x17DFD0);  //check around here
        memcpy(viewMatrix, gameViewMatrix, sizeof(float) * 16);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        std::cout << "[ESP] FAILED to read matrix!\n";
        return;
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glViewport(0, 0, (GLsizei)viewport[2], (GLsizei)viewport[3]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    int drawn = 0;
    for (int i = 0; i < 32; i++) {
        Entity* entity = entityList->entities[i];
        if (!entity || entity == localPlayer) continue;
        if (IsBadReadPtr(entity, sizeof(Entity))) continue;
        if (entity->player_health <= 0) continue;

        float distance = CalculateDistance(localPlayer, entity);
        if (distance > maxDistance) continue;

        // Use BODY position (X, Y, Z) and calculate feet/head from that
        Vec3 headPos = entity->HeadPos;
        Vec3 feetPos = entity->FeetPos;  // Add to Y for head

        Vec2 screenHead, screenFeet;
        if (!WorldToScreen(headPos, screenHead)) continue;
        if (!WorldToScreen(feetPos, screenFeet)) continue;

        drawn++;

        /*if (drawn < 3) {
            std::cout << "[ESP] Entity " << i << ":\n";
            std::cout << "  Feet world: (" << feet.x << ", " << feet.y << ", " << feet.z << ")\n";
            std::cout << "  Head world: (" << head.x << ", " << head.y << ", " << head.z << ")\n";
            std::cout << "  Feet screen: (" << screenFeet.x << ", " << screenFeet.y << ")\n";
            std::cout << "  Head screen: (" << screenHead.x << ", " << screenHead.y << ")\n";
        }*/

        static bool printedMatrix = false;
        if (!printedMatrix) {
            std::cout << "[ESP] View Matrix:\n";
            for (int i = 0; i < 4; i++) {
                std::cout << "  [" << viewMatrix[i * 4] << ", " << viewMatrix[i * 4 + 1]
                    << ", " << viewMatrix[i * 4 + 2] << ", " << viewMatrix[i * 4 + 3] << "]\n";
            }
            printedMatrix = true;
        }

        float height = abs(screenHead.y - screenFeet.y);
        float width = height / 2.0f;

        if (height < 10.0f) continue;

        // Draw box
        if (drawBoxes) {
            float x = screenHead.x - width / 2.0f;
            float y = screenHead.y;

            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
            glLineWidth(2.0f);

            glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + width, y);
            glVertex2f(x + width, y + height);
            glVertex2f(x, y + height);
            glEnd();
        }

        // Draw line
        if (drawLines) {
            glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
            glLineWidth(2.0f);
            glBegin(GL_LINES);
            glVertex2f((float)screenWidth / 2.0f, (float)screenHeight);
            glVertex2f(screenFeet.x, screenFeet.y);
            glEnd();
        }

        // Name
        if (drawNames && entity->name[0] != '\0') {
            char nameCopy[20] = { 0 };
            memcpy(nameCopy, entity->name, 19);

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            if (drawList) {
                ImVec2 textSize = ImGui::CalcTextSize(nameCopy);
                ImVec2 textPos(screenHead.x - textSize.x / 2, screenHead.y - 20);
                drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 200), nameCopy);
                drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), nameCopy);
            }
        }
    }

    glPopMatrix();
    glPopAttrib();
}



void ESP::DrawEntity(Entity* entity, Entity* localPlayer) {}

// ONLY ONE WorldToScreen function
bool ESP::WorldToScreen(const Vec3& worldPos, Vec2& screenPos) {
    // Use column-major multiplication (like the working code)
    float clipX = worldPos.x * viewMatrix[0] + worldPos.y * viewMatrix[4] + worldPos.z * viewMatrix[8] + viewMatrix[12];
    float clipY = worldPos.x * viewMatrix[1] + worldPos.y * viewMatrix[5] + worldPos.z * viewMatrix[9] + viewMatrix[13];
    float clipZ = worldPos.x * viewMatrix[2] + worldPos.y * viewMatrix[6] + worldPos.z * viewMatrix[10] + viewMatrix[14];
    float clipW = worldPos.x * viewMatrix[3] + worldPos.y * viewMatrix[7] + worldPos.z * viewMatrix[11] + viewMatrix[15];

    if (clipW < 0.1f) {
        return false;
    }

    float ndcX = clipX / clipW;
    float ndcY = clipY / clipW;

    // Use the CORRECT formula from the working code
    screenPos.x = (ndcX + 1.0f) * 0.5f * (float)screenWidth;
    screenPos.y = (1.0f - ndcY) * 0.5f * (float)screenHeight;

    return true;
}
void ESP::DrawBox2D(const Vec2& top, const Vec2& bottom, float width, const Color& color) {
    float x = top.x - width / 2.0f;
    float y = top.y;
    float w = width;
    float h = bottom.y - top.y;

    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(2.0f);

    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void ESP::DrawLine(const Vec2& from, const Vec2& to, const Color& color, float thickness) {
    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(thickness);
    glBegin(GL_LINES);
    glVertex2f(from.x, from.y);
    glVertex2f(to.x, to.y);
    glEnd();
}

void ESP::DrawFilledRect(float x, float y, float w, float h, const Color& color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void ESP::DrawText(const char* text, float x, float y, const Color& color) {
    if (!text || !text[0]) return;
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    if (!drawList) return;

    ImU32 imguiColor = IM_COL32((int)(color.r * 255), (int)(color.g * 255),
        (int)(color.b * 255), (int)(color.a * 255));
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImVec2 textPos(x - textSize.x / 2, y);

    drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 200), text);
    drawList->AddText(textPos, imguiColor, text);
}

void ESP::DrawHealthBar(float x, float y, float w, float h, int health, int maxHealth) {
    if (health < 0) health = 0;
    if (health > maxHealth) health = maxHealth;

    float healthPercent = (float)health / (float)maxHealth;

    DrawFilledRect(x, y, w, h, colors.healthBarBG);

    float healthHeight = h * healthPercent;
    float healthY = y + (h - healthHeight);

    Color healthColor;
    healthColor.r = 1.0f - healthPercent;
    healthColor.g = healthPercent;
    healthColor.b = 0.0f;
    healthColor.a = 1.0f;

    DrawFilledRect(x, healthY, w, healthHeight, healthColor);

    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void ESP::Shutdown() {}