#pragma once

#include <imgui/imgui.h>

class Config
{
public:
	static inline struct Configuration {
		enum class ECrosshairs : int {
			ENone,
			ECircle,
			ECross
		};
		struct
		{
			bool enable = false;
			float fov = 90.f;
			bool oxygen = false;
			bool crosshair = false;
			float crosshairSize = 1.f;
			float crosshairThickness = 1.f;
			ImVec4 crosshairColor = { 1.f, 1.f, 1.f, 1.f };
			ECrosshairs crosshairType = ECrosshairs::ENone;
		}client;
		struct
		{
			bool enable = false;
			struct
			{
				bool enable = false;
				float renderDistance = 100.f;
				ImVec4 colorVisible = { 1.f, 1.f, 1.f, 1.f };
				ImVec4 colorInvisible = { 1.f, 1.f, 1.f, 1.f };
				bool team = false;
				bool tracers = false;
				float tracersThickness = 1.f;

			}players;
			struct
			{
				bool enable = false;
				float renderDistance = 100.f;
				ImVec4 color = { 1.f, 1.f, 1.f, 1.f };
			}skeletons;
			struct
			{
				bool enable = false;
				float renderDistance = 100.f;
				ImVec4 color = { 1.f, 1.f, 1.f, 1.f };
				bool holes = false;
				ImVec4 holesColor = { 1.f, 1.f, 1.f, 1.f };
				bool skeletons = false;
				bool ghosts = false;
			}ships;
			struct
			{
				bool enable = false;
				float renderDistance = 1000.f;
				float size = 10.f;
				ImVec4 color = { 1.f, 1.f, 1.f, 1.f };
				bool marks = false;
				float marksRenderDistance = 100.f;
				ImVec4 marksColor = { 1.f, 1.f, 1.f, 1.f };
				bool vaults = false;
				float vaultsRenderDistance = 100.f;
				ImVec4 vaultsColor = { 1.f, 1.f, 1.f, 1.f };
				bool barrels = false;
				float barrelsRenderDistance = 100.f;
				ImVec4 barrelsColor = { 1.f, 1.f, 1.f, 1.f };
				bool ammoChest = false;
				float ammoChestRenderDistance = 100.f;
				ImVec4 ammoChestColor = { 1.f, 1.f, 1.f, 1.f };
			}islands;
			struct
			{
				bool enable = false;
				float renderDistance = 100.f;
				ImVec4 color = { 1.f, 1.f, 1.f, 1.f };
				bool nameToggle = false;
				bool mermaids = false;
				bool animals = false;
				float animalsRenderDistance = 100.f;
				ImVec4 animalsColor = { 1.f, 1.f, 1.f, 1.f };

			}items;
			struct 
			{
				bool enable = false;
				bool mermaids = false;
				float mermaidsRenderDistance = 100.f;
				ImVec4 mermaidsColor = { 1.f, 1.f, 1.f, 1.f };
				bool shipwrecks = false;
				float shipwrecksRenderDistance = 100.f;
				ImVec4 shipwrecksColor = { 1.f, 1.f, 1.f, 1.f };
				bool rowboats = false;
				float rowboatsRenderDistance = 100.f;
				ImVec4 rowboatsColor = { 1.f, 1.f, 1.f, 1.f };
				bool sharks = false;
				float sharksRenderDistance = 100.f;
				ImVec4 sharksColor = { 1.f, 1.f, 1.f, 1.f };
				bool events = false;
				float eventsRenderDistance = 100.f;
				ImVec4 eventsColor = { 1.f, 1.f, 1.f, 1.f };
			}others;
		}esp;
		struct
		{
			bool enable = false;
			struct
			{
				bool enable = false;
				float fPitch = 1.f;
				float fYaw = 1.f;
				float smooth = 1.f;
				float height = 1.f;
				bool players = false;
				bool skeletons = false;
				bool kegs = false;
				bool trigger = false;
				bool visibleOnly = false;
				bool calcShipVel = false;
			}weapon;
			struct
			{
				bool enable = false;
				float fPitch = 1.f;
				float fYaw = 1.f;
				float smooth = 1.f;
				bool instant = false;
				bool chains = false;
				bool players = false;
				bool skeletons = false;
				bool deckshots = false;
				bool ghostShips = false;
				bool lowAim = false;
				bool visibleOnly = false;
				bool drawPred = false;
			}cannon;
		}aim;
		struct
		{
			bool enable = false;
			bool shipInfo = false;
			bool mapPins = false;
			bool showSunk = false;
			ImVec4 sunkColor = { 1.f, 1.f, 1.f, 1.f };
			bool shipTray = false;
			float shipTrayThickness = 0.f;
			float shipTrayHeight = 0.f;
			ImVec4 shipTrayCol = { 1.f, 1.f, 1.f, 1.f };
		}game;
		struct
		{

			int cFont = 0;
			int lFont = 0;
			float nameTextRenderDistanceMax = 5000.f;
			float nameTextRenderDistanceMin = 45.f;
			float pinRenderDistanceMax = 50.f;
			float pinRenderDistanceMin = 0.f;
		}dev;
	}cfg;
};