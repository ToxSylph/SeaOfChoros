#include "engine.h"

using namespace engine;

void render(ImDrawList* drawList)
{
	static struct {
		ACharacter* target = nullptr;
		FVector location = FVector(0.f, 0.f, 0.f);
		FRotator delta = FRotator(0.f, 0.f, 0.f);
		float best = FLT_MAX;
		float smoothness = 1.f;
	} aimBest;
	aimBest.target = nullptr;
	aimBest.best = FLT_MAX;

	auto& io = ImGui::GetIO();
	auto const world = *UWorld::GWorld;
	const auto myLocation = localPlayerActor->K2_GetActorLocation();
	const auto camera = ((AController*)playerController)->PlayerCameraManager;
	const auto cameraLocation = camera->GetCameraLocation();
	const auto cameraRotation = camera->GetCameraRotation();
	auto item = localPlayerActor->GetWieldedItem();

	bool isWieldedWeapon = false;
	if (item) isWieldedWeapon = item->isWeapon();

	const auto localSword = *reinterpret_cast<AMeleeWeapon**>(&item);
	const auto localWeapon = *reinterpret_cast<AProjectileWeapon**>(&item);
	ACharacter* attachObject = localPlayerActor->GetAttachParentActor();


	int XMarksMapCount = 1;

	TArray<ULevel*> levels = AthenaGameViewportClient->World->Levels;

	if (cfg->client.enable)
	{
		static std::uintptr_t desiredTimeFOV = 0;
		if (milliseconds_now() >= desiredTimeFOV)
		{
			float fov = localPlayerActor->GetTargetFOV(localPlayerActor);
			playerController->FOV(cfg->client.fov);
			if (fov == 17.f)
			{
				playerController->FOV(cfg->client.fov * 0.2f);
			}
			desiredTimeFOV = milliseconds_now() + 100;
		}

		if (cfg->client.oxygen && localPlayerActor->IsInWater())
		{
			auto drownComp = localPlayerActor->DrowningComponent;
			if (drownComp)
			{
				float level = drownComp->GetOxygenLevel();
				auto posX = io.DisplaySize.x * 0.5f;
				auto posY = io.DisplaySize.y * 0.85f;
				auto barWidth = io.DisplaySize.x * 0.05f;
				auto barHeight = io.DisplaySize.y * 0.0030f;
				drawList->AddRectFilled({ posX - barWidth, posY - barHeight }, { posX + barWidth, posY + barHeight }, ImGui::GetColorU32(IM_COL32(0, 0, 0, 255)));
				drawList->AddRectFilled({ posX - barWidth, posY - barHeight }, { posX - barWidth + barWidth * level * 2.f, posY + barHeight }, ImGui::GetColorU32(IM_COL32(0, 200, 255, 255)));
				char buf[0x64];
				float pLevel = level * 100.f;
				sprintf_s(buf, sizeof(buf), "[ %.0f%% ]", pLevel);
				auto oxColor = level > 0.25f ? ImVec4(0.f, 1.f, 0.f, 1.f) : ImVec4(1.f, 0.f, 0.f, 1.f);
				RenderText(drawList, buf, FVector2D(posX + (barWidth * 0.5f), posY + barHeight + 10.f), oxColor, 20);
			}
		}
		if (cfg->client.crosshair)
		{
			switch (cfg->client.crosshairType)
			{
			case Config::Configuration::ECrosshairs::ENone:
				break;
			case Config::Configuration::ECrosshairs::ECircle:
				drawList->AddCircle({ io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f }, cfg->client.crosshairSize, ImGui::GetColorU32(cfg->client.crosshairColor), 0, cfg->client.crosshairThickness);
				break;
			case Config::Configuration::ECrosshairs::ECross:
				drawList->AddLine({ io.DisplaySize.x * 0.5f - cfg->client.crosshairSize, io.DisplaySize.y * 0.5f }, { io.DisplaySize.x * 0.5f + cfg->client.crosshairSize, io.DisplaySize.y * 0.5f }, ImGui::GetColorU32(cfg->client.crosshairColor), cfg->client.crosshairThickness);
				drawList->AddLine({ io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f - cfg->client.crosshairSize }, { io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f + cfg->client.crosshairSize }, ImGui::GetColorU32(cfg->client.crosshairColor), cfg->client.crosshairThickness);
				break;
			default:
				break;
			}
		}
	}
	if (cfg->esp.enable)
	{
		if (cfg->esp.islands.enable)
		{
			const auto entries = AthenaGameViewportClient->World->GameState->IslandService->IslandDataAsset->IslandDataEntries;
			for (auto i = 0u; i < entries.Count; i++)
			{
				auto const island = entries[i];
				auto const WorldMapData = island->WorldMapData;
				if (!WorldMapData) continue;
				const FVector islandLoc = WorldMapData->CaptureParams.WorldSpaceCameraPosition;
				const float dist = myLocation.DistTo(islandLoc) * 0.01f;
				if ((islandLoc.DistTo(FVector(0.f, 0.f, 0.f)) * 0.01) < 50.f) continue; // Conflictive Islands merging at 0.0.0 World Pos
				if (dist > cfg->esp.islands.renderDistance) continue;
				FVector2D screen;
				if (playerController->ProjectWorldLocationToScreen(islandLoc, screen))
				{
					char buf[0x64];
					auto len = island->LocalisedName->multi(buf, 0x50);
					sprintf_s(buf + len, sizeof(buf) - len, " [%.0fm]", dist);
					RenderText(drawList, buf, screen, cfg->esp.islands.color, (int)cfg->esp.islands.size + 10);
				}
			}
		}
	}

	for (UINT32 i = 0; i < levels.Count; i++)
	{
		if (!levels[i])
			continue;
		TArray<ACharacter*> actors = levels[i]->AActors;
		for (UINT32 j = 0; j < actors.Count; j++)
		{
			ACharacter* actor = actors[j];
			if (!actor)
				continue;


			if (cfg->esp.enable)
			{
				if (actor == localPlayerActor) continue;
				if (cfg->esp.players.enable)
				{
					if (actor->isPlayer())
					{
						const bool isTeamMate = UCrewFunctions::AreCharactersInSameCrew(actor, localPlayerActor);
						if (!cfg->esp.players.team && isTeamMate) continue;
						const FVector location = actor->K2_GetActorLocation();
						const float dist = myLocation.DistTo(location) * 0.01f;
						if (dist >= cfg->esp.players.renderDistance) continue;

						FVector origin, extent;
						actor->GetActorBounds(true, origin, extent);
						FVector2D headPos;
						playerController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z + extent.Z }, headPos);
						FVector2D footPos;
						playerController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z - extent.Z }, footPos);
						const float height = abs(footPos.Y - headPos.Y);
						const float width = height * 0.4f;

						ImVec4 color = (playerController->LineOfSightTo(actor, cameraLocation, false)) ? cfg->esp.players.colorVisible : cfg->esp.players.colorInvisible;
						if (!isTeamMate)
							Render2DBox(drawList, headPos, footPos, height, width, color);

						FVector2D screen;
						if (playerController->ProjectWorldLocationToScreen(location, screen))
						{
							auto const playerState = actor->PlayerState;
							if (!playerState) continue;
							const auto playerName = playerState->PlayerName;
							if (!playerName.Data) continue;

							char buf[0x64];
							ZeroMemory(buf, sizeof(buf));
							const int len = playerName.multi(buf, 0x20);
							sprintf_s(buf + len, sizeof(buf) - len, " [%.0fm]", dist);
							RenderText(drawList, buf, screen, color, dist);
						}

						if (cfg->esp.players.tracers && !isTeamMate)
						{
							Vector2 bScreen = Vector2();
							float fov = cfg->client.fov;
							if (WorldToScreen(Vector3(location.X, location.Y, location.Z), &bScreen, cameraLocation, cameraRotation, fov))
							{
								auto col = ImGui::GetColorU32(color);
								drawList->AddLine({ io.DisplaySize.x * 0.5f , io.DisplaySize.y * 0.5f }, { bScreen.x, bScreen.y }, col, cfg->esp.players.tracersThickness);
							}
						}
					}
				}
				if (cfg->esp.skeletons.enable)
				{
					if (actor->isSkeleton())
					{
						const FVector location = actor->K2_GetActorLocation();
						const float dist = myLocation.DistTo(location) * 0.01f;
						if (dist >= cfg->esp.skeletons.renderDistance) continue;
						FVector2D screen;
						if (playerController->ProjectWorldLocationToScreen(location, screen))
						{
							char buf[0x64];
							ZeroMemory(buf, sizeof(buf));
							sprintf_s(buf, sizeof(buf), "Skeleton [%.0fm]", dist);
							RenderText(drawList, buf, screen, cfg->esp.skeletons.color, dist);
						}
					}
				}
				if (cfg->esp.ships.enable)
				{
					FVector location = actor->K2_GetActorLocation();
					const float dist = myLocation.DistTo(location) * 0.01f;
					location.Z = 3000;

					if (actor->isShip() || actor->isFarShip())
					{
						if (dist >= cfg->esp.ships.renderDistance) continue;
						FVector2D screen;
						if (playerController->ProjectWorldLocationToScreen(location, screen))
						{
							char buf[0x64];
							ZeroMemory(buf, sizeof(buf));
							const float velocity = (actor->GetVelocity() * 0.01f).Size();
							float internalWater = 0.f;
							if (!actor->isFarShip()) internalWater = actor->GetInternalWater()->GetNormalizedWaterAmount() * 100.f;

							if (actor->isShip() && dist < 1726)
							{
								if (actor->compareName("BP_Small"))
									sprintf_s(buf, sizeof(buf), "Sloop [%.0f%%][%.0fm][%.0fm/s]", internalWater, dist, velocity);
								else if (actor->compareName("BP_Medium"))
									sprintf_s(buf, sizeof(buf), "Brigatine [%.0f%%][%.0fm][%.0fm/s]", internalWater, dist, velocity);
								else if (actor->compareName("BP_Large"))
									sprintf_s(buf, sizeof(buf), "Galleon [%.0f%%][%.0fm][%.0fm/s]", internalWater, dist, velocity);
								else if (actor->compareName("AISmall") && cfg->esp.ships.skeletons)
									sprintf_s(buf, sizeof(buf), "AI Sloop [%.0f%%][%.0fm][%.0fm/s]", internalWater, dist, velocity);
								else if (actor->compareName("AILarge") && cfg->esp.ships.skeletons)
									sprintf_s(buf, sizeof(buf), "AI Galleon [%.0f%%][%.0fm][%.0fm/s]", internalWater, dist, velocity);

								RenderText(drawList, buf, screen, cfg->esp.ships.color, 20);
							}
							else if (actor->isFarShip() && dist > 1725)
							{
								if (actor->compareName("BP_Small"))
									sprintf_s(buf, sizeof(buf), "Sloop [%.0fm][%.0fm/s]", dist, velocity);
								else if (actor->compareName("BP_Medium"))
									sprintf_s(buf, sizeof(buf), "Brigatine [%.0fm][%.0fm/s]", dist, velocity);
								else if (actor->compareName("BP_Large"))
									sprintf_s(buf, sizeof(buf), "Galleon [%.0fm][%.0fm/s]", dist, velocity);
								else if (actor->compareName("AISmall") && cfg->esp.ships.skeletons)
									sprintf_s(buf, sizeof(buf), "AI Sloop [%.0fm][%.0fm/s]", dist, velocity);
								else if (actor->compareName("AILarge") && cfg->esp.ships.skeletons)
									sprintf_s(buf, sizeof(buf), "AI Galleon [%.0fm][%.0fm/s]", dist, velocity);

								RenderText(drawList, buf, screen, cfg->esp.ships.color, 20);
							}
						}
					}
					if (cfg->esp.ships.ghosts && actor->isGhostShip())
					{
						FVector2D screen;
						if (playerController->ProjectWorldLocationToScreen(location, screen))
						{
							int lives = reinterpret_cast<AAggressiveGhostShip*>(actor)->NumShotsLeftToKill;
							char buf[0x64];
							ZeroMemory(buf, sizeof(buf));
							const int len = sprintf_s(buf, sizeof(buf), "Ghost Ship [%.0fm] | ", dist);
							switch (lives)
							{
							case 3: snprintf(buf + len, sizeof(buf) - len, "***"); break;
							case 2: snprintf(buf + len, sizeof(buf) - len, "**"); break;
							case 1: snprintf(buf + len, sizeof(buf) - len, "*"); break;
							default: snprintf(buf + len, sizeof(buf) - len, "DEAD"); break;
							}
							if (lives > 0)
								RenderText(drawList, buf, screen, cfg->esp.ships.color, 20);
						}
					}
					if (actor->isShip())
					{
						const FVector location = actor->K2_GetActorLocation();
						const int dist = myLocation.DistTo(location) * 0.01f;
						FVector2D screen;
						if (cfg->esp.ships.holes && dist <= 225)
						{
							auto const damage = actor->GetHullDamage();
							if (!damage) continue;
							const auto holes = damage->ActiveHullDamageZones;
							for (auto h = 0u; h < holes.Count; h++)
							{
								auto const hole = holes[h];
								const FVector location = hole->K2_GetActorLocation();
								if (playerController->ProjectWorldLocationToScreen(location, screen))
								{
									auto color = cfg->esp.ships.holesColor;
									drawList->AddLine({ screen.X - 6.f, screen.Y + 6.f }, { screen.X + 6.f, screen.Y - 6.f }, ImGui::GetColorU32(color));
									drawList->AddLine({ screen.X - 6.f, screen.Y - 6.f }, { screen.X + 6.f, screen.Y + 6.f }, ImGui::GetColorU32(color));
								}
							}
						}
					}
				}

				if (cfg->esp.islands.marks && actor->isXMarkMap()) // TODO: fix this shit
				{
					if (XMarksMapCount == 1 && item && item->isXMarkMap())
						RenderText(drawList, "Recognized Maps:", { io.DisplaySize.x * 0.85f, io.DisplaySize.y * 0.85f }, { 1.f, 1.f, 0.f, 1.f }, 30);
					auto map = reinterpret_cast<AXMarksTheSpotMap*>(actor);
					FString mapName = map->MapTexturePath;
					auto mapMarks = map->Marks;

					char cMapName[0x64];
					const int len = mapName.multi(cMapName, sizeof(cMapName));
					int mapCode = getMapNameCode(cMapName);

					try
					{
						const auto islandDataEntries = AthenaGameViewportClient->World->GameState->IslandService->IslandDataAsset->IslandDataEntries;

						float scaleFactor = 1.041669;

						for (auto i = 0u; i < islandDataEntries.Count; i++)
						{
							auto island = islandDataEntries[i];
							const char* sIslandName = island->IslandName.GetNameFast();
							char ccIsland[0x64];
							ZeroMemory(ccIsland, sizeof(ccIsland));
							sprintf_s(ccIsland, sizeof(ccIsland), sIslandName);

							int code = getMapNameCode(ccIsland);

							if (mapCode == code)
							{
								std::string buf = getIslandNameByCode(mapCode);
								if (item && item->isXMarkMap())
									RenderText(drawList, buf.c_str(), { io.DisplaySize.x * 0.85f, io.DisplaySize.y * 0.85f + 10 + 20 * XMarksMapCount }, { 0.f, 1.f, 0.f, 1.f }, 22);
								XMarksMapCount++;

								auto const WorldMapData = island->WorldMapData;
								if (!WorldMapData) continue;

								const FVector islandLoc = WorldMapData->CaptureParams.WorldSpaceCameraPosition;
								auto islandOrtho = WorldMapData->CaptureParams.CameraOrthoWidth;


								for (auto i = 0u; i < mapMarks.Count; i++)
								{
									auto mark = mapMarks[i];

									Vector2 v = Vector2(mark.Position);
									Vector2 vectorRotated = RotatePoint(v, Vector2(0.5f, 0.5f), 180 + map->Rotation, false);
									Vector2 vectorAlligned = Vector2(vectorRotated.x - 0.5f, vectorRotated.y - 0.5f);

									float islandScale = islandOrtho / scaleFactor;
									Vector2 offsetPos = vectorAlligned * islandScale;


									FVector digSpot = FVector(islandLoc.X - offsetPos.x, islandLoc.Y - offsetPos.y, 0.f);

									FHitResult hit_result;
									auto start = FVector(digSpot.X, digSpot.Y, 5000.f);

									bool res = raytrace(world, start, digSpot, &hit_result);
									digSpot.Z = hit_result.ImpactPoint.Z;

									FVector2D screen;
									if (playerController->ProjectWorldLocationToScreen(digSpot, screen))
									{
										char buf3[0x64];
										const int dist = myLocation.DistTo(digSpot) * 0.01f;
										if (dist > cfg->esp.islands.marksRenderDistance) continue;
										sprintf_s(buf3, sizeof(buf3), "X [%dm]", dist);
										RenderText(drawList, buf3, screen, cfg->esp.islands.marksColor, 18);
									}

								}
							}
						}
					}
					catch (...) {}

				}

				if (cfg->esp.islands.enable)
				{
					if (cfg->esp.islands.vaults && actor->isPuzzleVault())
					{
						auto vault = reinterpret_cast<APuzzleVault*>(actor);
						const FVector location = reinterpret_cast<ACharacter*>(vault->OuterDoor)->K2_GetActorLocation();
						FVector2D screen;
						if (playerController->ProjectWorldLocationToScreen(location, screen))
						{
							char buf[0x64];
							const float dist = myLocation.DistTo(location) * 0.01f;
							if (dist > cfg->esp.islands.vaultsRenderDistance) continue;
							sprintf_s(buf, "Vault Door [%.0fm]", dist);
							RenderText(drawList, buf, screen, cfg->esp.islands.vaultsColor, 51.f);
						}
					}
					if (cfg->esp.islands.barrels && actor->isBarrel())
					{
						const FVector location = actor->K2_GetActorLocation();
						FVector2D screen;
						const int dist = myLocation.DistTo(location) * 0.01f;
						if (dist > cfg->esp.islands.barrelsRenderDistance) continue;
						if (playerController->ProjectWorldLocationToScreen(location, screen))
						{
							char buf[0x64];
							sprintf_s(buf, "B");
							RenderText(drawList, buf, screen, cfg->esp.islands.barrelsColor, 16);
						}

					}
					if (cfg->esp.islands.ammoChest)
					{
						const FVector location = actor->K2_GetActorLocation();
						FVector2D screen;
						if (actor->isAmmoChest())
						{
							const float dist = myLocation.DistTo(location) * 0.01f;
							if (dist > cfg->esp.islands.ammoChestRenderDistance) continue;
							if (playerController->ProjectWorldLocationToScreen(location, screen))
							{
								char buf[0x64];
								sprintf_s(buf, "Ammo Chest [%.0fm]", dist);
								RenderText(drawList, buf, screen, cfg->esp.islands.ammoChestColor, 20);
							}
						}
					}
				}
				if (cfg->esp.items.enable)
				{
					auto location = actor->K2_GetActorLocation();
					FVector2D screen;
					if (actor->isItem())
					{
						const float dist = myLocation.DistTo(location) * 0.01f;
						if (dist > cfg->esp.items.renderDistance) continue;

						if (playerController->ProjectWorldLocationToScreen(location, screen))
						{
							char buf[0x64];
							ZeroMemory(buf, sizeof(buf));

							if (actor->compareName("BP_SunkenCurseArtefact_"))
							{
								if (actor->compareName("Ruby"))
									sprintf_s(buf, sizeof(buf), "Ruby UnderWater Statue");
								if (actor->compareName("Emerald"))
									sprintf_s(buf, sizeof(buf), "Emerald UnderWater Statue");
								if (actor->compareName("Sapphire"))
									sprintf_s(buf, sizeof(buf), "Sapphire UnderWater Statue");
								if (cfg->esp.items.nameToggle)
								{
									if (GetAsyncKeyState(0x52)) // R Key
										RenderText(drawList, buf, screen, cfg->esp.items.color, 51.f);
									else
										RenderText(drawList, buf, screen, cfg->esp.items.color, 35.f);
								}
								else
								{
									RenderText(drawList, buf, screen, cfg->esp.items.color, 51.f);
								}
								continue;
							}

							auto const desc = actor->GetItemInfo()->Desc;
							if (!desc) continue;
							const int len = desc->Title->multi(buf, 0x50);
							snprintf(buf + len, sizeof(buf) - len, " [%.0fm]", dist);
							if (cfg->esp.items.nameToggle)
							{
								if (GetAsyncKeyState(0x52)) // R Key
									RenderText(drawList, buf, screen, cfg->esp.items.color, 51.f);
								else
									RenderText(drawList, buf, screen, cfg->esp.items.color, 35.f);
							}
							else
							{
								RenderText(drawList, buf, screen, cfg->esp.items.color, 51.f);
							}
						}
					}
				}
				if (cfg->esp.items.animals && actor->isAnimal())
				{
					FVector origin, extent;
					actor->GetActorBounds(true, origin, extent);
					FVector2D headPos;
					if (!playerController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z + extent.Z }, headPos)) continue;
					FVector2D footPos;
					if (!playerController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z - extent.Z }, footPos)) continue;
					float height = abs(footPos.Y - headPos.Y);
					float width = height * 0.6f;

					FText displayNameText = reinterpret_cast<AFauna*>(actor)->DisplayName;
					FString displayNameString = FString(displayNameText.TextData->Text);
					if (displayNameString.IsValid()) {
						const float dist = myLocation.DistTo(origin) * 0.01f;
						if (dist > cfg->esp.items.animalsRenderDistance) continue;
						char buf[0x32];
						const int len = displayNameString.multi(buf, 0x50);
						snprintf(buf + len, sizeof(buf) - len, " [%.0fm]", dist);
						const float adjust = height * 0.05f;
						FVector2D pos = { headPos.X, headPos.Y - adjust };
						RenderText(drawList, buf, pos, cfg->esp.items.animalsColor, dist);
					}
				}
				if (cfg->esp.others.enable)
				{
					if (cfg->esp.others.shipwrecks && (actor->isShipwreck() || actor->compareName("BP_Shipwreck_01_a_NetProxy_C")))
					{
						auto location = actor->K2_GetActorLocation();
						FVector2D screen;
						if (playerController->ProjectWorldLocationToScreen(location, screen))
						{
							const float dist = myLocation.DistTo(location) * 0.01f;
							if (dist > cfg->esp.others.shipwrecksRenderDistance) continue;
							char buf[0x64];
							sprintf_s(buf, sizeof(buf), "Shipwreck [%.0fm]", dist);
							RenderText(drawList, buf, screen, cfg->esp.others.shipwrecksColor, dist);
						}
					}
					if (cfg->esp.others.mermaids)
					{
						if (actor->isMermaid())
						{
							FVector2D screen;
							const FVector location = actor->K2_GetActorLocation();
							const float dist = myLocation.DistTo(location) * 0.01f;
							if (dist > cfg->esp.others.mermaidsRenderDistance) continue;
							{
								if (playerController->ProjectWorldLocationToScreen(location, screen))
								{
									char buf[0x16];
									sprintf_s(buf, sizeof(buf), "Mermaid [%.0fm]", dist);
									RenderText(drawList, buf, screen, cfg->esp.others.mermaidsColor, 30);
								}

							}
						}
					}
					if (cfg->esp.others.rowboats)
					{
						if (actor->isRowboat())
						{
							const FVector location = actor->K2_GetActorLocation();
							const float dist = myLocation.DistTo(location) * 0.01f;
							if (dist > cfg->esp.others.rowboatsRenderDistance) continue;
							FVector2D screen;
							if (playerController->ProjectWorldLocationToScreen(location, screen))
							{
								char buf[0x64];
								if (actor->compareName("Cannon"))
									sprintf_s(buf, sizeof(buf), "Cannon Rowboat [%.0fm]", dist);
								else if (actor->compareName("Harpoon"))
									sprintf_s(buf, sizeof(buf), "Harpoon Rowboat [%.0fm]", dist);
								else if (actor->compareName("Rowboat"))
									sprintf_s(buf, sizeof(buf), "Rowboat [%.0fm]", dist);
								RenderText(drawList, buf, screen, cfg->esp.others.rowboatsColor, dist);
							}
						}
					}
					if (cfg->esp.others.sharks && actor->isShark())
					{
						FVector origin, extent;
						actor->GetActorBounds(true, origin, extent);
						FVector2D headPos;
						if (!playerController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z + extent.Z }, headPos)) continue;
						FVector2D footPos;
						if (!playerController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z - extent.Z }, footPos)) continue;
						const float height = abs(footPos.Y - headPos.Y);
						const float width = height * 0.6f;
						char buf[0x20];
						const float dist = myLocation.DistTo(origin) * 0.01f;
						if (dist > cfg->esp.others.sharksRenderDistance) continue;
						sprintf_s(buf, sizeof(buf), "Shark [%.0fm]", dist);
						const float adjust = height * 0.05f;
						FVector2D pos = { headPos.X, headPos.Y - adjust };
						RenderText(drawList, buf, pos, cfg->esp.others.sharksColor, 25);
					}
					if (cfg->esp.others.events)
					{
						if (actor->isEvent())
						{
							const FVector location = actor->K2_GetActorLocation();
							FVector2D screen;
							if (playerController->ProjectWorldLocationToScreen(location, screen))
							{
								auto type = actor->GetName();
								const float dist = myLocation.DistTo(location) * 0.01f;
								if (dist > cfg->esp.others.eventsRenderDistance) continue;
								char buf[0x64];
								ZeroMemory(buf, sizeof(buf));
								if (actor->compareName("ShipCloud"))
									sprintf_s(buf, "Fleet [%.0fm]", dist);
								else if (actor->compareName("AshenLord"))
									sprintf_s(buf, "Ashen Lord [%.0fm]", dist);
								else if (actor->compareName("Flameheart"))
									sprintf_s(buf, "Flame Heart [%.0fm]", dist);
								else if (actor->compareName("LegendSkellyFort"))
									sprintf_s(buf, "Fort of Fortune [%.0fm]", dist);
								else if (actor->compareName("SkellyFortOfTheDamned"))
									sprintf_s(buf, "FOTD [%.0fm]", dist);
								else if (actor->compareName("BP_SkellyFort"))
									sprintf_s(buf, "Skull Fort [%.0fm]", dist);
								RenderText(drawList, buf, screen, cfg->esp.others.eventsColor, 25);
							}
						}
					}
				}
			}

			if (cfg->aim.enable)
			{
				if (attachObject && attachObject->isCannon() && cfg->aim.cannon.enable)
				{
					if (cfg->aim.cannon.chains && actor->isShip())
					{
						do
						{
							if (actor == localPlayerActor->GetCurrentShip())
							{
								break;
							}
							FVector location = actor->K2_GetActorLocation();
							if (cfg->aim.cannon.visibleOnly && !playerController->LineOfSightTo(actor, cameraLocation, false))
							{
								break;
							}
							if (location.DistTo(cameraLocation) > 55000)
							{
								break;
							}
							int amount = 0;
							auto water = actor->GetInternalWater();
							amount = water->GetNormalizedWaterAmount() * 100.f;
							if (amount == 100)
								break;
							auto cannon = reinterpret_cast<ACannon*>(attachObject);
							float gravity_scale = cannon->ProjectileGravityScale;
							const FVector forward = actor->GetActorForwardVector();
							const FVector up = actor->GetActorUpVector();
							const FVector loc = actor->K2_GetActorLocation();
							FVector loc_mast = loc;
							loc_mast += forward * 80.f;
							loc_mast += up * 1300.f;
							location = loc_mast;
							gravity_scale = 1.f;
							FRotator low, high;
							//int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLocation, attachObject->GetVelocity(), low, high);
							int i_solutions = 0;
							if (i_solutions < 1)
								break;
							low.Clamp();
							low -= attachObject->K2_GetActorRotation();
							low.Clamp();
							float absPitch = abs(low.Pitch);
							float absYaw = abs(low.Yaw);
							if (absPitch > cfg->aim.cannon.fPitch || absYaw > cfg->aim.cannon.fYaw) { break; }
							float sum = absYaw + absPitch;
							if (sum < aimBest.best)
							{
								aimBest.target = actor;
								aimBest.location = location;
								aimBest.delta = low;
								aimBest.best = sum;
							}
						} while (false);
					}
					if (cfg->aim.cannon.players && cfg->aim.cannon.chains == false && actor->isPlayer() && actor != localPlayerActor && !actor->IsDead())
					{
						do
						{
							if (UCrewFunctions::AreCharactersInSameCrew(actor, localPlayerActor)) break;
							FVector location = actor->K2_GetActorLocation();
							if (location.DistTo(cameraLocation) > 55000)
							{
								break;
							}
							auto cannon = reinterpret_cast<ACannon*>(attachObject);
							float gravity_scale = cannon->ProjectileGravityScale;
							FRotator low, high;
							//int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLocation, attachObject->GetForwardVelocity(), low, high);
							int i_solutions = 0;
							if (i_solutions < 1)
								break;
							low.Clamp();
							low -= attachObject->K2_GetActorRotation();
							low.Clamp();
							float absPitch = abs(low.Pitch);
							float absYaw = abs(low.Yaw);
							if (absPitch > cfg->aim.cannon.fPitch || absYaw > cfg->aim.cannon.fYaw) { break; }
							float sum = absYaw + absPitch;
							if (sum < aimBest.best)
							{
								aimBest.target = actor;
								aimBest.location = location;
								aimBest.delta = low;
								aimBest.best = sum;
							}
						} while (false);
					}
					if (cfg->aim.cannon.skeletons && cfg->aim.cannon.chains == false && actor->isSkeleton() && actor != localPlayerActor && !actor->IsDead() && !playerController->LineOfSightTo(actor, cameraLocation, false))
					{
						do
						{
							FVector location = actor->K2_GetActorLocation();
							if (location.DistTo(cameraLocation) > 55000)
							{
								break;
							}
							auto cannon = reinterpret_cast<ACannon*>(attachObject);
							float gravity_scale = cannon->ProjectileGravityScale;
							FRotator low, high;
							//int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLocation, attachObject->GetForwardVelocity(), low, high);
							int i_solutions = 0;
							if (i_solutions < 1)
								break;
							low.Clamp();
							low -= attachObject->K2_GetActorRotation();
							low.Clamp();
							float absPitch = abs(low.Pitch);
							float absYaw = abs(low.Yaw);
							if (absPitch > cfg->aim.cannon.fPitch || absYaw > cfg->aim.cannon.fYaw) { break; }
							float sum = absYaw + absPitch;
							if (sum < aimBest.best)
							{
								aimBest.target = actor;
								aimBest.location = location;
								aimBest.delta = low;
								aimBest.best = sum;
							}
						} while (false);
					}
					if (cfg->aim.cannon.chains == false && actor->isShip())
					{
						do
						{
							if (actor == localPlayerActor->GetCurrentShip())
							{
								break;
							}
							FVector location = actor->K2_GetActorLocation();
							if (cfg->aim.cannon.visibleOnly && !playerController->LineOfSightTo(actor, cameraLocation, false))
							{
								break;
							}
							if (location.DistTo(cameraLocation) > 55000)
							{
								break;
							}
							auto cannon = reinterpret_cast<ACannon*>(attachObject);
							int amount = 0;
							auto water = actor->GetInternalWater();
							amount = water->GetNormalizedWaterAmount() * 100.f;
							if (amount == 100)
								break;
							float gravity_scale = cannon->ProjectileGravityScale;
							if (cfg->aim.cannon.deckshots)
							{
								const FVector forward = actor->GetActorForwardVector();
								const FVector up = actor->GetActorUpVector();
								const FVector loc = actor->K2_GetActorLocation();
								FVector loc_mast = loc;
								loc_mast += forward * 52.f;
								loc_mast += up * 275.f;
								location = loc_mast;
								gravity_scale = 1.30f;
							}
							else if (cfg->aim.cannon.lowAim)
							{
								auto const damage = actor->GetHullDamage();
								if (damage)
								{
									FVector loc = pickHoleToAim(damage, myLocation);
									if (loc.Sum() != 9999.f)
										location = loc;
								}
							}
							FRotator low, high;
							//int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLocation, attachObject->GetVelocity(), low, high);
							int i_solutions = 0;
							if (i_solutions < 1)
								break;
							low.Clamp();
							low -= attachObject->K2_GetActorRotation();
							low.Clamp();
							float absPitch = abs(low.Pitch);
							float absYaw = abs(low.Yaw);
							if (absPitch > cfg->aim.cannon.fPitch || absYaw > cfg->aim.cannon.fYaw) { break; }
							float sum = absYaw + absPitch;
							if (sum < aimBest.best)
							{
								aimBest.target = actor;
								aimBest.location = location;
								aimBest.delta = low;
								aimBest.best = sum;
							}
						} while (false);
					}
					if (cfg->aim.cannon.ghostShips && cfg->aim.cannon.chains == false && actor->isGhostShip())
					{
						do
						{
							FVector location = actor->K2_GetActorLocation();
							auto ship = reinterpret_cast<AAggressiveGhostShip*>(actor);
							if (cfg->aim.cannon.visibleOnly && !playerController->LineOfSightTo(actor, cameraLocation, false))
							{
								break;
							}
							if (myLocation.DistTo(location) > 55000)
							{
								break;
							}
							auto cannon = reinterpret_cast<ACannon*>(attachObject);
							float gravity_scale = cannon->ProjectileGravityScale;
							auto forward = actor->GetActorForwardVector();
							forward *= ship->ShipState.ShipSpeed;
							FRotator low, high;
							//int i_solutions = AimAtMovingTarget(location, forward, cannon->ProjectileSpeed, gravity_scale, cameraLocation, attachObject->GetVelocity(), low, high);
							int i_solutions = 0;
							if (i_solutions < 1)
								break;
							low.Clamp();
							low -= attachObject->K2_GetActorRotation();
							low.Clamp();
							float absPitch = abs(low.Pitch);
							float absYaw = abs(low.Yaw);
							if (absPitch > cfg->aim.cannon.fPitch || absYaw > cfg->aim.cannon.fYaw) { break; }
							float sum = absYaw + absPitch;
							if (sum < aimBest.best)
							{
								aimBest.target = actor;
								aimBest.location = location;
								aimBest.delta = low;
								aimBest.best = sum;
							}
						} while (false);
					}
				}
				else if (isWieldedWeapon && cfg->aim.weapon.enable)
				{
					if (cfg->aim.weapon.players && actor->isPlayer() && actor != localPlayerActor && !actor->IsDead())
					{
						do
						{
							FVector playerLoc = actor->K2_GetActorLocation();
							if (!actor->IsInWater() && localWeapon->WeaponParameters.NumberOfProjectiles == 1)
							{
								playerLoc.Z += cfg->aim.weapon.height;
							}
							float dist = myLocation.DistTo(playerLoc);
							if (dist > localWeapon->WeaponParameters.ProjectileMaximumRange * 2.f) { break; }
							if (cfg->aim.weapon.visibleOnly) if (!playerController->LineOfSightTo(actor, cameraLocation, false)) { break; }
							if (UCrewFunctions::AreCharactersInSameCrew(actor, localPlayerActor)) break;
							FRotator rotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, playerLoc), cameraRotation);
							float absYaw = abs(rotationDelta.Yaw);
							float absPitch = abs(rotationDelta.Pitch);
							if (absYaw > cfg->aim.weapon.fYaw || absPitch > cfg->aim.weapon.fPitch) { break; }
							float sum = absYaw + absPitch;
							if (sum < aimBest.best)
							{
								aimBest.target = actor;
								aimBest.location = playerLoc;
								aimBest.delta = rotationDelta;
								aimBest.best = sum;
								aimBest.smoothness = cfg->aim.weapon.smooth;
							}

						} while (false);
					}
					if (cfg->aim.weapon.kegs)
					{
						if (actor->compareName("BP_MerchantCrate_GunpowderBarrel_"))
						{
							do
							{
								const FVector playerLoc = actor->K2_GetActorLocation();
								const float dist = myLocation.DistTo(playerLoc);
								if (dist > localWeapon->WeaponParameters.ProjectileMaximumRange * 2.f) break;
								if (cfg->aim.weapon.visibleOnly) if (!playerController->LineOfSightTo(actor, cameraLocation, false)) break;
								const FRotator rotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, playerLoc), cameraRotation);
								const float absYaw = abs(rotationDelta.Yaw);
								const float absPitch = abs(rotationDelta.Pitch);
								if (absYaw > cfg->aim.weapon.fYaw || absPitch > cfg->aim.weapon.fPitch) break;
								const float sum = absYaw + absPitch;
								if (sum < aimBest.best)
								{
									aimBest.target = actor;
									aimBest.location = playerLoc;
									aimBest.delta = rotationDelta;
									aimBest.best = sum;
									aimBest.smoothness = cfg->aim.weapon.smooth;
								}

							} while (false);
						}
					}
					if (cfg->aim.weapon.skeletons && actor->isSkeleton() && !actor->IsDead())
					{
						do
						{
							FVector playerLoc = actor->K2_GetActorLocation();
							if (localWeapon->WeaponParameters.NumberOfProjectiles == 1)
								playerLoc.Z += cfg->aim.weapon.height;
							const float dist = myLocation.DistTo(playerLoc);
							if (dist > localWeapon->WeaponParameters.ProjectileMaximumRange * 2.f) break;
							if (cfg->aim.weapon.visibleOnly) if (!playerController->LineOfSightTo(actor, cameraLocation, false)) break;
							const FRotator rotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, playerLoc), cameraRotation);
							const float absYaw = abs(rotationDelta.Yaw);
							const float absPitch = abs(rotationDelta.Pitch);
							if (absYaw > cfg->aim.weapon.fYaw || absPitch > cfg->aim.weapon.fPitch) break;
							const float sum = absYaw + absPitch;
							if (sum < aimBest.best)
							{
								aimBest.target = actor;
								aimBest.location = playerLoc;
								aimBest.delta = rotationDelta;
								aimBest.best = sum;
								aimBest.smoothness = cfg->aim.weapon.smooth;
							}
						} while (false);
					}
				}
			}
		}
	}
	if (aimBest.target != nullptr)
	{
		FVector2D screen;
		if (playerController->ProjectWorldLocationToScreen(aimBest.location, screen))
		{
			auto col = ImGui::GetColorU32(IM_COL32(0, 200, 0, 255));
			drawList->AddCircle({ screen.X, screen.Y }, 5.f, col, 0, 3);
			drawList->AddLine({ screen.X, screen.Y - 20.f }, { screen.X, screen.Y + 20.f }, col, 2);
			drawList->AddLine({ screen.X - 20.f, screen.Y }, { screen.X + 20.f, screen.Y }, col, 2);
		}
		static std::uintptr_t shotDesiredTime = 0;
		if (GetAsyncKeyState(VK_RBUTTON))
		{
			if (true)
			{
				FVector LV;
				FVector TV;

				if (cfg->aim.weapon.calcShipVel)
				{
					try // Kegs exception
					{
						LV = localPlayerActor->GetVelocity();
						if (auto const localShip = localPlayerActor->GetCurrentShip()) LV += localShip->GetVelocity();
						TV = aimBest.target->GetVelocity();
						if (auto const targetShip = aimBest.target->GetCurrentShip()) TV += targetShip->GetVelocity();
					}
					catch (...)
					{
						LV = { 0.f,0.f,0.f };
						TV = { 0.f,0.f,0.f };
					}
				}
				else
				{
					LV = { 0.f,0.f,0.f };
					TV = { 0.f,0.f,0.f };
				}
				const FVector RV = TV - LV;
				const float BS = localWeapon->WeaponParameters.AmmoParams.Velocity;
				const FVector RL = myLocation - aimBest.location;
				const float a = RV.Size() - BS * BS;
				const float b = (RL * RV * 2.f).Sum();
				const float c = RL.SizeSquared();
				const float D = b * b - 4 * a * c;
				if (D > 0)
				{
					const float DRoot = sqrtf(D);
					const float x1 = (-b + DRoot) / (2 * a);
					const float x2 = (-b - DRoot) / (2 * a);
					if (x1 >= 0 && x1 >= x2) aimBest.location += RV * x1;
					else if (x2 >= 0) aimBest.location += RV * x2;
					aimBest.delta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, aimBest.location), cameraRotation);
					auto smoothness = 1.f / aimBest.smoothness;
					playerController->AddYawInput(aimBest.delta.Yaw * smoothness);
					playerController->AddPitchInput(aimBest.delta.Pitch * -smoothness);
				}
				static byte shotFixC = 0;
				static bool shotFixing = false;

				if (cfg->aim.weapon.trigger && isWieldedWeapon && localWeapon->CanFire())
				{
					do {
						if (shotFixC != 1 && !shotFixing)
						{
							shotFixC++;
							break;
						}
						if (shotDesiredTime == 0)
						{
							shotDesiredTime = milliseconds_now() + 210;
							shotFixing = true;
						}
						if (milliseconds_now() >= shotDesiredTime)
						{
							shotDesiredTime = 0;
							shotFixing = false;
						}
						else
						{
							break;
						}

						if (!playerController->LineOfSightTo(aimBest.target, cameraLocation, false)) break;
						INPUT inputs[6] = {};
						ZeroMemory(inputs, sizeof(inputs));

						inputs[0].type = INPUT_MOUSE;
						inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

						inputs[1].type = INPUT_MOUSE;
						inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

						inputs[2].type = INPUT_KEYBOARD;
						inputs[2].ki.wVk = 0x58; // X Key

						inputs[3].type = INPUT_KEYBOARD;
						inputs[3].ki.wVk = 0x58; // X Key
						inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

						inputs[4].type = INPUT_KEYBOARD;
						inputs[4].ki.wVk = 0x58; // X Key

						inputs[5].type = INPUT_KEYBOARD;
						inputs[5].ki.wVk = 0x58; // X Key
						inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;

						SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
						shotFixC = 0;
					} while (false);
				}
			}
		}
		else
		{
			shotDesiredTime = 0;
		}
	}
}
bool initUE4(uintptr_t world, uintptr_t objects, uintptr_t names)
{
	try
	{
		UWorld::GWorld = reinterpret_cast<decltype(UWorld::GWorld)>(world);
		UObject::GObjects = reinterpret_cast<decltype(UObject::GObjects)>(objects);
		FName::GNames = *reinterpret_cast<decltype(FName::GNames)*>(names);
		AthenaGameViewportClient = UObject::FindObject<UAthenaGameViewportClient>("AthenaGameViewportClient Transient.AthenaGameEngine_1.AthenaGameViewportClient_1");
		if (!checkSDKObjects())
			return false;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

bool setGameVars()
{
	localPlayer = AthenaGameViewportClient->GameInstance->LocalPlayers[0];
	playerController = localPlayer->PlayerController;
	localPlayerActor = (AAthenaPlayerCharacter*)playerController->K2_GetPawn();
	return true;
}

bool checkGameVars()
{
	if (!AthenaGameViewportClient) return false;
	if (!localPlayer) return false;
	if (!playerController) return false;
	if (!localPlayerActor) return false;

	auto world = *UWorld::GWorld;
	if (!world) return false;
	auto const game = world->GameInstance;
	if (!game) return false;
	auto const localPlayer = game->LocalPlayers[0];
	if (!localPlayer) return false;
	auto const localController = localPlayer->PlayerController;
	if (!localController) return false;
	auto const localCharacter = localController->Character;
	if (!localCharacter) return false;
	if (localCharacter->IsLoading()) return false;

	return true;
}

bool updateGameVars()
{
	if (!AthenaGameViewportClient) return false;
	localPlayer = AthenaGameViewportClient->GameInstance->LocalPlayers[0];
	if (!localPlayer) return false;
	playerController = localPlayer->PlayerController;
	if (!playerController) return false;
	localPlayerActor = (AAthenaPlayerCharacter*)playerController->K2_GetPawn();
	if (!localPlayerActor) return false;
	if (!checkGameVars()) return false;
	return true;
}

void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const float dist, const bool outlined, const bool centered)
{
	if (!text) return;
	auto ImScreen = *reinterpret_cast<const ImVec2*>(&pos);
	if (dist <= cfg->dev.pinRenderDistanceMax && dist >= cfg->dev.pinRenderDistanceMin)
	{
		float pinSize = dist * -0.2 + 10;
		pinSize = fClamp(pinSize, 0.5f, 5.f);
		renderPin(drawList, ImScreen, color, pinSize);
	}
	if (centered)
	{
		auto size = ImGui::CalcTextSize(text);
		ImScreen.x -= size.x * 0.5f;
	}
	if (dist <= cfg->dev.nameTextRenderDistanceMax && dist >= cfg->dev.nameTextRenderDistanceMin)
	{
		float fontSize = -(std::sqrt(dist + 750.f)) + 50;
		fontSize = fClamp(fontSize, 10.f, 25.f);
		RenderText(drawList, text, ImScreen, color, fontSize);
	}

}

void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const bool outlined, const bool centered)
{
	if (!text) return;
	auto ImScreen = *reinterpret_cast<const ImVec2*>(&pos);
	if (centered)
	{
		auto size = ImGui::CalcTextSize(text);
		ImScreen.x -= size.x * 0.5f;
		ImScreen.y += size.y;
	}
	drawList->AddText(nullptr, 20.f, ImScreen, ImGui::GetColorU32(color), text);
}

void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const int fontSize = 10)
{
	if (!text) return;
	auto ImScreen = *reinterpret_cast<const ImVec2*>(&pos);
	auto size = ImGui::CalcTextSize(text);
	ImScreen.x -= size.x * 0.5f;

	drawList->AddText(nullptr, (float)fontSize, ImVec2(ImScreen.x - 1.f, ImScreen.y - 1.f), ImGui::GetColorU32(IM_COL32_BLACK), text);
	drawList->AddText(nullptr, (float)fontSize, ImVec2(ImScreen.x + 1.f, ImScreen.y + 1.f), ImGui::GetColorU32(IM_COL32_BLACK), text);
	drawList->AddText(nullptr, (float)fontSize, ImScreen, ImGui::GetColorU32(color), text);
}

void RenderText(ImDrawList* drawList, const char* text, const ImVec2& screen, const ImVec4& color, const float size, const bool outlined, const bool centered)
{
	auto window = ImGui::GetCurrentWindow();

	window->DrawList->AddText(nullptr, size, ImVec2(screen.x - 1.f, screen.y - 1.f), ImGui::GetColorU32(IM_COL32_BLACK), text);
	window->DrawList->AddText(nullptr, size, ImVec2(screen.x + 1.f, screen.y + 1.f), ImGui::GetColorU32(IM_COL32_BLACK), text);
	window->DrawList->AddText(nullptr, size, screen, ImGui::GetColorU32(color), text);

}
void renderPin(ImDrawList* drawList, const ImVec2& ImScreen, const ImVec4& color, const float radius)
{
	drawList->AddCircle(ImVec2(ImScreen.x, ImScreen.y), radius, ImGui::GetColorU32(color), 32, 7.f);
}

void Render2DBox(ImDrawList* drawList, const FVector2D& top, const FVector2D& bottom, const float height, const float width, const ImVec4& color)
{
	drawList->AddRect({ top.X - width * 0.5f, top.Y }, { top.X + width * 0.5f, bottom.Y }, ImGui::GetColorU32(color), 0.f, 15, 1.5f);
}

float fClamp(float v, const float min, const float max)
{
	if (v < min) v = min;
	if (v > max) v = max;
	return v;
}


#define Assert( _exp ) ((void)0)

struct vMatrix
{
	vMatrix() {}
	vMatrix(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
	{
		m_flMatVal[0][0] = m00;	m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
		m_flMatVal[1][0] = m10;	m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
		m_flMatVal[2][0] = m20;	m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
	}

	float* operator[](int i) { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
	const float* operator[](int i) const { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
	float* Base() { return &m_flMatVal[0][0]; }
	const float* Base() const { return &m_flMatVal[0][0]; }

	float m_flMatVal[3][4];
};

vMatrix Matrix(Vector3 rot, Vector3 origin)
{
	origin = Vector3(0, 0, 0);
	float radPitch = (rot.x * float(PI) / 180.f);
	float radYaw = (rot.y * float(PI) / 180.f);
	float radRoll = (rot.z * float(PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	vMatrix matrix;
	matrix[0][0] = CP * CY;
	matrix[0][1] = CP * SY;
	matrix[0][2] = SP;
	matrix[0][3] = 0.f;

	matrix[1][0] = SR * SP * CY - CR * SY;
	matrix[1][1] = SR * SP * SY + CR * CY;
	matrix[1][2] = -SR * CP;
	matrix[1][3] = 0.f;

	matrix[2][0] = -(CR * SP * CY + SR * SY);
	matrix[2][1] = CY * SR - CR * SP * SY;
	matrix[2][2] = CR * CP;
	matrix[2][3] = 0.f;

	//matrix[3][0] = origin.x;
	//matrix[3][1] = origin.y;
	//matrix[3][2] = origin.z;
	//matrix[3][3] = 1.f;

	return matrix;
}

// @vianove13
bool WorldToScreen(Vector3 origin, Vector2* out, const FVector& cameraLocation, const FRotator& cameraRotation, const float fov) {
	Vector3 Screenlocation = Vector3(0, 0, 0);
	Vector3 Rotation = Vector3(cameraRotation.Pitch, cameraRotation.Yaw, cameraRotation.Roll);	// FRotator
	Vector3 Location = Vector3(cameraLocation.X, cameraLocation.Y, cameraLocation.Z);

	vMatrix tempMatrix = Matrix(Rotation, Vector3(0, 0, 0)); // Matrix

	Vector3 vAxisX, vAxisY, vAxisZ;

	vAxisX = Vector3(tempMatrix[0][0], tempMatrix[0][1], tempMatrix[0][2]);
	vAxisY = Vector3(tempMatrix[1][0], tempMatrix[1][1], tempMatrix[1][2]);
	vAxisZ = Vector3(tempMatrix[2][0], tempMatrix[2][1], tempMatrix[2][2]);

	Vector3 vDelta = origin - Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	auto& io = ImGui::GetIO();
	float FovAngle = fov; // + 22; for widescreen?
	float ScreenCenterX = io.DisplaySize.x * 0.5f;
	float ScreenCenterY = io.DisplaySize.y * 0.5f;

	out->x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * static_cast<float>(PI) / 360.f)) / vTransformed.z;
	out->y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * static_cast<float>(PI) / 360.f)) / vTransformed.z;


	return true;
}


// @vianove13 checks this for some reason that idk, and idc
bool checkSDKObjects()
{
	if (!UCrewFunctions::Init()) return false;
	tslog::verbose("UCrewFunctions - Ready.");
	if (!UKismetMathLibrary::Init()) return false;
	tslog::verbose("UKismetMathLibrary - Ready.");
	return true;
}

uintptr_t milliseconds_now() {
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
	if (s_use_qpc) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (1000LL * now.QuadPart) / s_frequency.QuadPart;
	}
	else {
		return GetTickCount64();
	}
}

// @gummy
Vector2 RotatePoint(Vector2 pointToRotate, Vector2 centerPoint, float angle, bool angleInRadians)
{
	if (!angleInRadians)
		angle = static_cast<float>(angle * (PI / 180.f));
	float cosTheta = static_cast<float>(cos(angle));
	float sinTheta = static_cast<float>(sin(angle));
	Vector2 returnVec = Vector2(cosTheta * (pointToRotate.x - centerPoint.x) - sinTheta * (pointToRotate.y - centerPoint.y), sinTheta * (pointToRotate.x - centerPoint.x) + cosTheta * (pointToRotate.y - centerPoint.y)
	);
	returnVec += centerPoint;
	return returnVec;
}

bool raytrace(UWorld* world, const struct FVector& start, const struct FVector& end, struct FHitResult* hit)
{
	if (world == nullptr || world->PersistentLevel == nullptr)
		return false;
	return UKismetMathLibrary::LineTraceSingle_NEW((UObject*)world, start, end, ETraceTypeQuery::TraceTypeQuery3, true, TArray<AActor*>(), EDrawDebugTrace::EDrawDebugTrace__None, true, hit);
}

// Picks the closest undamaged HullDamageZone
FVector pickHoleToAim(AHullDamage* damage, const FVector& localLoc)
{
	FVector fLocation = { 0.f, 0.f, 9999.f };
	FVector location = FVector();
	float currentDist = 55000.f;
	const auto holes = damage->DamageZones;
	for (auto h = 0u; h < holes.Count; h++)
	{
		auto const hole = holes[h];
		if (hole->DamageLevel < 3)
		{
			location = FVector(reinterpret_cast<ACharacter*>(hole)->K2_GetActorLocation());
			float dist = localLoc.DistTo(location);
			if (dist <= currentDist)
			{
				currentDist = dist;
				fLocation = location;
			}
		}
	}
	return fLocation;
}

int getMapNameCode(char* name)
{
	if (strstr(name, "wsp_resource_island_02_e") != NULL) // Barnacle Cay
	{
		return 1;
	}
	if (strstr(name, "wld_resource_island_01_b") != NULL) // Black Sand Atoll
	{
		return 2;
	}
	if (strstr(name, "wld_resource_island_02_b") != NULL) // Black Water Enclave
	{
		return 3;
	}
	if (strstr(name, "wld_resource_island_01_d") != NULL) // Blind Man's Lagoon
	{
		return 4;
	}
	if (strstr(name, "wsp_resource_island_01_f") != NULL) // Booty Isle
	{
		return 5;
	}
	if (strstr(name, "bsp_resource_island_01_a") != NULL) // Boulder Cay
	{
		return 6;
	}
	if (strstr(name, "dvr_resource_island_01_h") != NULL) // Brimstone Rock
	{
		return 7;
	}
	if (strstr(name, "wsp_resource_island_01_c") != NULL) // Castaway Isle
	{
		return 8;
	}
	if (strstr(name, "wsp_resource_island_01_a") != NULL) // Chicken Isle
	{
		return 9;
	}
	if (strstr(name, "dvr_resource_island_01_c") != NULL) // Cinder Islet
	{
		return 10;
	}
	if (strstr(name, "dvr_resource_island_01_b") != NULL) // Cursewater Shores
	{
		return 11;
	}
	if (strstr(name, "wsp_resource_island_01_e") != NULL) // Cutlass Cay
	{
		return 12;
	}
	if (strstr(name, "dvr_resource_island_01_g") != NULL) // Flame's End
	{
		return 13;
	}
	if (strstr(name, "wsp_resource_island_01_b") != NULL) // Fools Lagoon
	{
		return 14;
	}
	if (strstr(name, "dvr_resource_island_01_e") != NULL) // Glowstone Cay
	{
		return 15;
	}
	if (strstr(name, "wld_resource_island_02_c") != NULL) // Isle of Last Words
	{
		return 16;
	}
	if (strstr(name, "bsp_resource_island_02_f") != NULL) // Lagoon of Whispers
	{
		return 17;
	}
	if (strstr(name, "wld_resource_island_01_k") != NULL) // Liar's Backbone
	{
		return 18;
	}
	if (strstr(name, "bsp_resource_island_02_c") != NULL) // Lonely Isle
	{
		return 19;
	}
	if (strstr(name, "wsp_resource_island_02_a") != NULL) // Lookout Point
	{
		return 20;
	}
	if (strstr(name, "dvr_resource_island_01_i") != NULL) // Magma's Tide
	{
		return 21;
	}
	if (strstr(name, "wsp_resource_island_02_f") != NULL) // Mutineer Rock
	{
		return 22;
	}
	if (strstr(name, "wsp_resource_island_01_g") != NULL) // Old Salts Atoll
	{
		return 23;
	}
	if (strstr(name, "wsp_resource_island_01_d") != NULL) // Paradise Spring
	{
		return 24;
	}
	if (strstr(name, "bsp_resource_island_01_i") != NULL) // Picaroon Palms
	{
		return 25;
	}
	if (strstr(name, "wld_resource_island_01_c") != NULL) // Plunderer's Plight
	{
		return 26;
	}
	if (strstr(name, "bsp_resource_island_02_d") != NULL) // Rapier Cay
	{
		return 27;
	}
	if (strstr(name, "dvr_resource_island_01_f") != NULL) // Roaring Sands
	{
		return 28;
	}
	if (strstr(name, "bsp_resource_island_02_a") != NULL) // Rum Runner Isle
	{
		return 29;
	}
	if (strstr(name, "bsp_resource_island_01_b") != NULL) // Salty Sands
	{
		return 30;
	}
	if (strstr(name, "bsp_resource_island_02_b") != NULL) // Sandy Shallows
	{
		return 31;
	}
	if (strstr(name, "dvr_resource_island_01_a") != NULL) // Schored Pass
	{
		return 32;
	}
	if (strstr(name, "wld_resource_island_01_j") != NULL) // Scurvy Isley
	{
		return 33;
	}
	if (strstr(name, "bsp_resource_island_02_e") != NULL) // Sea Dog's Rest
	{
		return 34;
	}
	if (strstr(name, "wld_resource_island_01_h") != NULL) // Shark Tooth Key
	{
		return 35;
	}
	if (strstr(name, "wld_resource_island_02_e") != NULL) // Shiver Retreat
	{
		return 36;
	}
	if (strstr(name, "dvr_resource_island_01_d") != NULL) // The Forsaken Brink
	{
		return 37;
	}
	if (strstr(name, "wld_feature_tribute_peak") != NULL) // Tribute Peak
	{
		return 38;
	}
	if (strstr(name, "wld_resource_island_02_d") != NULL) // Tri-Rock Isle
	{
		return 39;
	}
	if (strstr(name, "bsp_resource_island_01_e") != NULL) // Twin Groves
	{
		return 40;
	}
	if (strstr(name, "dvr_feature_island_01_e") != NULL) // Ashen Reaches
	{
		return 41;
	}
	if (strstr(name, "bsp_feature_crescent_cove") != NULL) // Cannon Cove
	{
		return 42;
	}
	if (strstr(name, "bsp_feature_crescent_island") != NULL) // Crescent Isle
	{
		return 43;
	}
	if (strstr(name, "wsp_feature_crooks_hollow") != NULL) // Crook's Hollow
	{
		return 44;
	}
	if (strstr(name, "wsp_feature_devils_ridge") != NULL) // Devil's Ridge
	{
		return 45;
	}
	if (strstr(name, "wsp_feature_discovery_ridge") != NULL) // Discovery Ridge
	{
		return 46;
	}
	if (strstr(name, "dvr_feature_island_01_a") != NULL) // Fetcher's Rest
	{
		return 47;
	}
	if (strstr(name, "dvr_feature_island_01_b") != NULL) // Flintlock Peninsula
	{
		return 48;
	}
	if (strstr(name, "wld_feature_dragons_breath") != NULL) // Kraken's Fall
	{
		return 49;
	}
	if (strstr(name, "bsp_feature_lone_cove") != NULL) // Lone Cove
	{
		return 50;
	}
	if (strstr(name, "wld_feature_arches") != NULL) // Marauder's Arch
	{
		return 51;
	}
	if (strstr(name, "bsp_feature_mermaids_hideaway") != NULL) // Mermaid's Hideaway
	{
		return 52;
	}
	if (strstr(name, "wld_feature_old_faithful") != NULL) // Old Faithful Isle
	{
		return 53;
	}
	if (strstr(name, "wsp_feature_plunder_valley") != NULL) // Plunder Valley
	{
		return 54;
	}
	if (strstr(name, "dvr_feature_island_01_c") != NULL) // Ruby's Fall
	{
		return 55;
	}
	if (strstr(name, "bsp_feature_sailors_bounty") != NULL) // Sailor's Bounty
	{
		return 56;
	}
	if (strstr(name, "wsp_feature_sharkbait_cove") != NULL) // Shark Bait Cove
	{
		return 57;
	}
	if (strstr(name, "wld_feature_shipwreck_bay") != NULL) // Shipwreck Bay
	{
		return 58;
	}
	if (strstr(name, "bsp_feature_smugglers_bay") != NULL) // Smugglers' Bay
	{
		return 59;
	}
	if (strstr(name, "wsp_feature_snake_island") != NULL) // Snake Island
	{
		return 60;
	}
	if (strstr(name, "wld_feature_three_peaks") != NULL) // The Crooked Masts
	{
		return 61;
	}
	if (strstr(name, "dvr_feature_island_01_d") != NULL) // The Devil's Thirst
	{
		return 62;
	}
	if (strstr(name, "wld_feature_cavern_isle") != NULL) // The Sunken Grove
	{
		return 63;
	}
	if (strstr(name, "wsp_feature_thieves_haven") != NULL) // Thieves' Haven
	{
		return 64;
	}
	if (strstr(name, "bsp_feature_wanderers_archipelago") != NULL) // Wanderers Refuge
	{
		return 65;
	}
	return 0;
}


std::string getIslandNameByCode(int code)
{
	switch (code)
	{
	case 1: return "BarnaCle Cay";
	case 2: return "Black Sand Atoll";
	case 3: return "Black Water Enclave";
	case 4: return "Blind Man's Lagoon";
	case 5: return "Booty Isle";
	case 6: return "Boulder Cay";
	case 7: return "Brimstone Rock";
	case 8: return "Castaway Isle";
	case 9: return "Chicken Isle";
	case 10:return "Cinder Islet";
	case 11:return "Cursewater Shores";
	case 12:return "Cutlass Cay";
	case 13:return "Flame's End";
	case 14:return "Fools Lagoon";
	case 15:return "Glowstone Cay";
	case 16:return "Isle of Last Words";
	case 17:return "Lagoon of Whispers";
	case 18:return "Liar's Backbone";
	case 19:return "Lonely Isle";
	case 20:return "Lookout Point";
	case 21:return "Magma's Tide";
	case 22:return "Mutineer Rock";
	case 23:return "Old Salts Atoll";
	case 24:return "Paradise Spring";
	case 25:return "Picaroon Palms";
	case 26:return "Plunderer's Plight";
	case 27:return "Rapier Cay";
	case 28:return "Roaring Sands";
	case 29:return "Rum Runner Isle";
	case 30:return "Salty Sands";
	case 31:return "Sandy Shallows";
	case 32:return "Schored Pass";
	case 33:return "Scurvy Isley";
	case 34:return "Sea Dog's Rest";
	case 35:return "Shark Tooth Key";
	case 36:return "Shiver Retreat";
	case 37:return "The Forsaken Brink";
	case 38:return "Tribute Peak";
	case 39:return "Tri-Rock Isle";
	case 40:return "Twin Groves";
	case 41:return "Ashen Reaches";
	case 42:return "Cannon Cove";
	case 43:return "Crescent Isle";
	case 44:return "Crook's Hollow";
	case 45:return "Devil's Ridge";
	case 46:return "Discovery Ridge";
	case 47:return "Fetcher's Rest";
	case 48:return "Flintlock Peninsula";
	case 49:return "Kraken's Fall";
	case 50:return "Lone Cove";
	case 51:return "Marauder's Arch";
	case 52:return "Mermaid's Hideaway";
	case 53:return "Old Faithful Isle";
	case 54:return "Plunder Valley";
	case 55:return "Ruby's Fall";
	case 56:return "Sailor's Bounty";
	case 57:return "Shark Bait Cove";
	case 58:return "Shipwreck Bay";
	case 59:return "Smugglers' Bay";
	case 60:return "Snake Island";
	case 61:return "The Crooked Masts";
	case 62:return "The Devil's Thirst";
	case 63:return "The Sunken Grove";
	case 64:return "Thieves' Haven";
	case 65:return "Wanderers Refuge";
	default:return "No Island Data";
	}
}
