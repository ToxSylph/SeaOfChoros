#pragma once
#include "UE4/SDK.h"
#include "logger.h"
#include "config.h"
#include "cIcons.h"

#include <d3d11.h>
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_internal.h"

namespace engine
{

	static UAthenaGameViewportClient* AthenaGameViewportClient = nullptr;
	static ULocalPlayer* localPlayer = nullptr;
	static APlayerController* playerController = nullptr;
	static AAthenaPlayerCharacter* localPlayerActor = nullptr;
	static ULevel* persistentLevel = nullptr;
	static Config::Configuration* cfg = &Config::cfg;
}

void update();
void render(ImDrawList* drawList);
bool initUE4(uintptr_t world, uintptr_t objects, uintptr_t names);
bool setGameVars();
bool checkGameVars();
bool updateGameVars();
void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const float dist, const bool outlined = false, const bool centered = true);
void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const bool outlined = false, const bool centered = true);
void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const int fontSize);
void RenderText(ImDrawList* drawList, const char* text, const ImVec2& screen, const ImVec4& color, const float size, const bool outlined = false, const bool centered = true);
void renderPin(ImDrawList* drawList, const ImVec2& pos, const ImVec4& color, const float radius);
void Render2DBox(ImDrawList* drawList, const FVector2D& top, const FVector2D& bottom, const float height, const float width, const ImVec4& color);
float fClamp(float v, const float min, const float max);
bool checkSDKObjects();
bool WorldToScreen(Vector3 origin, Vector2* out, const FVector& cameraLocation, const FRotator& cameraRotation, const float fov);
uintptr_t milliseconds_now();
Vector2 RotatePoint(Vector2 pointToRotate, Vector2 centerPoint, float angle, bool angleInRadians = false);
bool raytrace(UWorld* world, const struct FVector& start, const struct FVector& end, struct FHitResult* hit);

int getMapNameCode(char* name);
std::string getIslandNameByCode(int code);