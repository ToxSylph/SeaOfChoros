#pragma once
#include <Windows.h>
#include <UE4/UE4.h>
#include <string>

struct UFunction;
class UClass;

inline void ProcessEvent(void* obj, UFunction* function, void* parms)
{
	auto vtable = *reinterpret_cast<void***>(obj);
	reinterpret_cast<void(*)(void*, UFunction*, void*)>(vtable[55])(obj, function, parms);
}

template<typename Fn>
inline Fn GetVFunction(const void* instance, std::size_t index)
{
	auto vtable = *reinterpret_cast<const void***>(const_cast<void*>(instance));
	return reinterpret_cast<Fn>(vtable[index]);
}

struct FNameEntry
{
	uint32_t Index;
	uint32_t pad;
	FNameEntry* HashNext;
	char AnsiName[1024];

	const int GetIndex() const { return Index >> 1; }
	const char* GetAnsiName() const { return AnsiName; }
};

class TNameEntryArray
{
public:

	bool IsValidIndex(uint32_t index) const { return index < NumElements; }

	FNameEntry const* GetById(uint32_t index) const { return *GetItemPtr(index); }

	FNameEntry const* const* GetItemPtr(uint32_t Index) const {
		const auto ChunkIndex = Index / 16384;
		const auto WithinChunkIndex = Index % 16384;
		const auto Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	FNameEntry** Chunks[128];
	uint32_t NumElements = 0;
	uint32_t NumChunks = 0;
};

// @abrn
struct FTextData
{
public:
	wchar_t* Text; //0x0000 (0x08)
};

struct FText
{
public:
	struct FTextData* TextData; //0x0000 (0x08)
};

class FString : public TArray<wchar_t>
{
public:
	inline FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? static_cast<int32_t>(std::wcslen(other)) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	int multi(char* name, int size) const
	{
		return WideCharToMultiByte(CP_UTF8, 0, Data, Count, name, size, nullptr, nullptr) - 1;
	}

	inline const wchar_t* wide() const
	{
		return Data;
	}

	inline const wchar_t* c_str() const
	{
		if (Data)
			return Data;
		return L"";
	}
};

struct FName
{
	int ComparisonIndex = 0;
	int Number = 0;

	static inline TNameEntryArray* GNames = nullptr;

	static const char* GetNameByIdFast(int Id) {
		auto NameEntry = GNames->GetById(Id);
		if (!NameEntry) return nullptr;
		return NameEntry->GetAnsiName();
	}

	static std::string GetNameById(int Id) {
		auto NameEntry = GNames->GetById(Id);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	}

	const char* GetNameFast() const {
		auto NameEntry = GNames->GetById(ComparisonIndex);
		if (!NameEntry) return nullptr;
		return NameEntry->GetAnsiName();
	}

	const std::string GetName() const {
		auto NameEntry = GNames->GetById(ComparisonIndex);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	};

	inline bool operator==(const FName& other) const {
		return ComparisonIndex == other.ComparisonIndex;
	};

	FName() {}

	FName(const char* find) {
		for (auto i = 6000u; i < GNames->NumElements; i++)
		{
			auto name = GetNameByIdFast(i);
			if (!name) continue;
			if (strcmp(name, find) == 0) {
				ComparisonIndex = i;
				return;
			};
		}
	}
};

struct FUObjectItem
{
	class UObject* Object;
	int Flags;
	int ClusterIndex;
	int SerialNumber;
	int pad;
};

struct TUObjectArray
{
	FUObjectItem* Objects;
	int MaxElements;
	int NumElements;

	class UObject* GetByIndex(int index) { return Objects[index].Object; }
};

// Class CoreUObject.Object
// Size: 0x28 (Inherited: 0x00)
struct UObject {
	UObject(UObject* addr) { *this = addr; }
	static inline TUObjectArray* GObjects = nullptr;
	void* Vtable; // 0x0
	int ObjectFlags; // 0x8
	int InternalIndex; // 0xC
	UClass* Class; // 0x10
	FName Name; // 0x18
	UObject* Outer; // 0x20

	std::string GetName() const;
	const char* GetNameFast() const;
	std::string GetFullName() const;
	bool IsA(UClass* cmp) const;

	template<typename T>
	static T* FindObject(const std::string& name)
	{
		for (int i = 0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);

			if (object == nullptr)
			{
				continue;
			}

			if (object->GetFullName() == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	}

	static UClass* FindClass(const std::string& name)
	{
		return FindObject<UClass>(name);
	}
};

struct FVector_NetQuantize : public FVector
{

};
struct FHitResult
{
	unsigned char                                      bBlockingHit : 1;                                         // 0x0000(0x0001)
	unsigned char                                      bStartPenetrating : 1;                                    // 0x0000(0x0001)
	unsigned char                                      UnknownData00[0x3];                                       // 0x0001(0x0003) MISSED OFFSET
	float                                              Time;                                                     // 0x0004(0x0004) (ZeroConstructor, IsPlainOldData)
	float                                              Distance;                                                 // 0x0008(0x0004) (ZeroConstructor, IsPlainOldData)
	struct FVector_NetQuantize                         ImpactPoint;                                              // 0x0018(0x000C)
	char pad_5894[0x48];
	float                                              PenetrationDepth;                                         // 0x0054(0x0004) (ZeroConstructor, IsPlainOldData)
	int                                                Item;                                                     // 0x0058(0x0004) (ZeroConstructor, IsPlainOldData)
	char pad_3424[0x18];
	struct FName                                       BoneName;                                                 // 0x0074(0x0008) (ZeroConstructor, IsPlainOldData)
	int                                                FaceIndex;                                                // 0x007C(0x0004) (ZeroConstructor, IsPlainOldData)
};

enum class EPlayerActivityType : uint8_t
{
	None = 0,
	Bailing = 1,
	Cannon = 2,
	Cannon_END = 3,
	Capstan = 4,
	Capstan_END = 5,
	CarryingBooty = 6,
	CarryingBooty_END = 7,
	Dead = 8,
	Dead_END = 9,
	Digging = 10,
	Dousing = 11,
	EmptyingBucket = 12,
	Harpoon = 13,
	Harpoon_END = 14,
	LoseHealth = 15,
	Repairing = 16,
	Sails = 17,
	Sails_END = 18,
	UndoingRepair = 19,
	Wheel = 20,
	Wheel_END = 21,
};

struct APlayerState
{
	char pad[0x03D0];
	float                                              Score;                                                     // 0x03D0(0x0004) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float											   Ping;                                                      // 0x03D4(0x0001) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	FString							                   PlayerName;                                                // 0x03D8 (BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, RepNotify, HasGetValueTypeHash)


	EPlayerActivityType GetPlayerActivity()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaPlayerState.GetPlayerActivity");
		EPlayerActivityType activity;
		ProcessEvent(this, fn, &activity);
		return activity;
	}
};

struct ADamageZone {
	char pad[0x064C];
	int32_t DamageLevel; // 0x64c(0x04)
};

struct AHullDamage {
	char pad[0x0418];
	struct TArray<struct ADamageZone*> DamageZones; // 0x418(0x10)
	TArray<class ACharacter*> ActiveHullDamageZones; // 0x0428
};

struct AShipInternalWater {
	float GetNormalizedWaterAmount() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipInternalWater.GetNormalizedWaterAmount");
		float params = 0.f;
		ProcessEvent(this, fn, &params);
		return params;
	}
};

struct AFauna {
	char pad1[0x0808];
	FText DisplayName; // 0x0808
};

// ScriptStruct Engine.RepMovement
// Size: 0x38 (Inherited: 0x00)
struct FRepMovement {
	struct FVector LinearVelocity; // 0x00(0x0c)
	struct FVector AngularVelocity; // 0x0c(0x0c)
	struct FVector Location; // 0x18(0x0c)
	struct FRotator Rotation; // 0x24(0x0c)
	char bSimulatedPhysicSleep : 1; // 0x30(0x01)
	char bRepPhysics : 1; // 0x30(0x01)
	char UnknownData_30_2 : 6; // 0x30(0x01)
	char LocationQuantizationLevel; // 0x31(0x01)
	char VelocityQuantizationLevel; // 0x32(0x01)
	char RotationQuantizationLevel; // 0x33(0x01)
	char UnknownData_34[0x4]; // 0x34(0x04)
};

// Class Engine.Actor
// Size: 0x3d0 (Inherited: 0x28)
struct AActor : UObject {
	char PrimaryActorTick[0x50]; // 0x28(0x50)
	float CustomTimeDilation; // 0x78(0x04)
	char bAllowRemovalFromServerWhenCollisionMerged : 1; // 0x7c(0x01)
	char bAllowRemovalFromServerWhenAutomaticallyInstanced : 1; // 0x7c(0x01)
	char bHidden : 1; // 0x7c(0x01)
	char bNetTemporary : 1; // 0x7c(0x01)
	char bNetStartup : 1; // 0x7c(0x01)
	char bOnlyRelevantToOwner : 1; // 0x7c(0x01)
	char bAlwaysRelevant : 1; // 0x7c(0x01)
	char bReplicateMovement : 1; // 0x7c(0x01)
	char bTearOff : 1; // 0x7d(0x01)
	char bExchangedRoles : 1; // 0x7d(0x01)
	char bPendingNetUpdate : 1; // 0x7d(0x01)
	char bNetLoadOnClient : 1; // 0x7d(0x01)
	char bNetUseOwnerRelevancy : 1; // 0x7d(0x01)
	char bBlockInput : 1; // 0x7d(0x01)
	char UnknownData_7D_6 : 1; // 0x7d(0x01)
	char bCanBeInCluster : 1; // 0x7d(0x01)
	char UnknownData_7E_0 : 2; // 0x7e(0x01)
	char bActorEnableCollision : 1; // 0x7e(0x01)
	char UnknownData_7E_3 : 1; // 0x7e(0x01)
	char bReplicateAttachment : 1; // 0x7e(0x01)
	char UnknownData_7E_5 : 1; // 0x7e(0x01)
	char bReplicates : 1; // 0x7e(0x01)
	char UnknownData_7F[0x1]; // 0x7f(0x01)
	char OnPreNetOwnershipChange[0x1]; // 0x80(0x01)
	char UnknownData_81[0x1]; // 0x81(0x01)
	char RemoteRole; // 0x82(0x01)
	char UnknownData_83[0x5]; // 0x83(0x05)
	struct AActor* Owner; // 0x88(0x08)
	char SpawnRestrictions; // 0x90(0x01)
	char UnknownData_91[0x3]; // 0x91(0x03)
	struct FRepMovement ReplicatedMovement; // 0x94(0x38)
	char UnknownData_CC[0x4]; // 0xcc(0x04)
	char AttachmentReplication[0x48]; // 0xd0(0x48)
	char Role; // 0x118(0x01)
	char UnknownData_119[0x1]; // 0x119(0x01)
	char AutoReceiveInput; // 0x11a(0x01)
	char UnknownData_11B[0x1]; // 0x11b(0x01)
	int32_t InputPriority; // 0x11c(0x04)
	struct UInputComponent* InputComponent; // 0x120(0x08)
	float NetCullDistanceSquared; // 0x128(0x04)
	char UnknownData_12C[0x4]; // 0x12c(0x04)
	int32_t NetTag; // 0x130(0x04)
	float NetUpdateTime; // 0x134(0x04)
	float NetUpdateFrequency; // 0x138(0x04)
	float NetPriority; // 0x13c(0x04)
	float LastNetUpdateTime; // 0x140(0x04)
	struct FName NetDriverName; // 0x144(0x08)
	char bAutoDestroyWhenFinished : 1; // 0x14c(0x01)
	char bCanBeDamaged : 1; // 0x14c(0x01)
	char bActorIsBeingDestroyed : 1; // 0x14c(0x01)
	char bCollideWhenPlacing : 1; // 0x14c(0x01)
	char bFindCameraComponentWhenViewTarget : 1; // 0x14c(0x01)
	char bRelevantForNetworkReplays : 1; // 0x14c(0x01)
	char UnknownData_14C_6 : 2; // 0x14c(0x01)
	char UnknownData_14D[0x3]; // 0x14d(0x03)
	char SpawnCollisionHandlingMethod; // 0x150(0x01)
	char UnknownData_151[0x7]; // 0x151(0x07)
	struct APawn* Instigator; // 0x158(0x08)
	struct TArray<struct AActor*> Children; // 0x160(0x10)
	struct USceneComponent* RootComponent; // 0x170(0x08)
	struct TArray<struct AMatineeActor*> ControllingMatineeActors; // 0x178(0x10)
	float InitialLifeSpan; // 0x188(0x04)
	char UnknownData_18C[0x4]; // 0x18c(0x04)
	char bAllowReceiveTickEventOnDedicatedServer : 1; // 0x190(0x01)
	char UnknownData_190_1 : 7; // 0x190(0x01)
	char UnknownData_191[0x7]; // 0x191(0x07)
	struct TArray<struct FName> Layers; // 0x198(0x10)
	char ParentComponentActor[0x8]; // 0x1a8(0x08)
	struct TArray<struct AActor*> ChildComponentActors; // 0x1b0(0x10)
	char UnknownData_1C0[0x8]; // 0x1c0(0x08)
	char bActorSeamlessTraveled : 1; // 0x1c8(0x01)
	char bIgnoresOriginShifting : 1; // 0x1c8(0x01)
	char bEnableAutoLODGeneration : 1; // 0x1c8(0x01)
	char InvertFeatureCheck : 1; // 0x1c8(0x01)
	char UnknownData_1C8_4 : 4; // 0x1c8(0x01)
	char UnknownData_1C9[0x3]; // 0x1c9(0x03)
	struct FName Feature; // 0x1cc(0x08)
	char UnknownData_1D4[0x4]; // 0x1d4(0x04)
	struct TArray<struct FName> Tags; // 0x1d8(0x10)
	uint64_t HiddenEditorViews; // 0x1e8(0x08)
	char UnknownData_1F0[0x4]; // 0x1f0(0x04)
	char UnknownData_1F4[0x3c]; // 0x1f4(0x3c)
	char OnEndPlay[0x1]; // 0x230(0x01)
	bool bDoOverlapNotifiesOnLoad; // 0x231(0x01)
	char UnknownData_232[0xf6]; // 0x232(0xf6)
	struct TArray<struct UActorComponent*> BlueprintCreatedComponents; // 0x328(0x10)
	struct TArray<struct UActorComponent*> InstanceComponents; // 0x338(0x10)
	char UnknownData_348[0x8]; // 0x348(0x08)
	struct TArray<struct AActor*> ChildActorInterfaceProviders; // 0x350(0x10)
	char UnknownData_360[0x68]; // 0x360(0x68)
	double DormancyLingeringInSeconds; // 0x3c8(0x08)

	struct FVector GetActorRightVector()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorRightVector");
		FVector ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}
};

// Class Engine.Pawn
// Size: 0x448 (Inherited: 0x3d0)
struct APawn : AActor {
	char pad[0x20];
	struct APlayerState* PlayerState; // 0x3f0(0x08)
	char pad2[0x50];
};

struct FWorldMapIslandDataCaptureParams
{
	char pad1[0x0018];
	struct FVector                                     WorldSpaceCameraPosition;                                  // 0x0018(0x000C) (Edit, ZeroConstructor, Transient, EditConst, IsPlainOldData, NoDestructor)
	char pad2[0x8];
	float                                              CameraOrthoWidth;                                          // 0x002C(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

struct UWorldMapIslandDataAsset {
	char pad[0x0030];
	struct FWorldMapIslandDataCaptureParams            CaptureParams;                                             // 0x0030(0x0040) (Edit, BlueprintVisible, BlueprintReadOnly)
	FVector WorldSpaceCameraPosition;
	// ADD THE OFFSET OF CAPTUREPARAMS TO THIS OFFSET
};

struct UIslandDataAssetEntry {
	char pad1[0x0028];
	struct FName IslandName; // 0x0028(0x0008) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	char pad2[0x10];
	UWorldMapIslandDataAsset* WorldMapData; // 0x0040
	char pad3[0x68];
	FString* LocalisedName; // 0x00B0
};

struct UIslandDataAsset {
	char pad[0x0048];
	TArray<UIslandDataAssetEntry*> IslandDataEntries; // 0x0048
};

struct AIslandService {
	char pad[0x0460];
	UIslandDataAsset* IslandDataAsset; // 0x460
};

struct FGuid
{
	int                                                A;                                                         // 0x0000(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                B;                                                         // 0x0004(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                C;                                                         // 0x0008(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                D;                                                         // 0x000C(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

struct FSessionTemplate
{
	struct FString                                     TemplateName;                                              // 0x0000(0x0010) (ZeroConstructor, Protected, HasGetValueTypeHash)
	unsigned char             SessionType;                                               // 0x0010(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	unsigned char                                      UnknownData_2Q1C[0x3];                                     // 0x0011(0x0003) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	int                                                MaxPlayers;                                                // 0x0014(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)

};

// ScriptStruct Sessions.CrewSessionTemplate
// 0x0020 (0x0038 - 0x0018)
struct FCrewSessionTemplate : public FSessionTemplate
{
	struct FString                                     MatchmakingHopper;                                         // 0x0018(0x0010) (ZeroConstructor, HasGetValueTypeHash)
	class UClass* ShipSize;                                                  // 0x0028(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash)
	int                                                MaxMatchmakingPlayers;                                     // 0x0030(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      UnknownData_JPXK[0x4];                                     // 0x0034(0x0004) MISSED OFFSET (PADDING)

};

struct FCrew
{
	struct FGuid                                       CrewId;                                                   // 0x0000(0x0010) (ZeroConstructor, IsPlainOldData)
	struct FGuid                                       SessionId;                                                // 0x0010(0x0010) (ZeroConstructor, IsPlainOldData)
	TArray<class APlayerState*>                        Players;                                                  // 0x0020(0x0010) (ZeroConstructor)
	struct FCrewSessionTemplate                        CrewSessionTemplate;                                      // 0x0030(0x0038)
	struct FGuid                                       LiveryID;                                                 // 0x0068(0x0010) (ZeroConstructor, IsPlainOldData)
	char pad[0x18];
};

struct ACrewService {
	char pad[0x04A8];
	TArray<FCrew> Crews; // 0x04A8
};

struct AShipService
{
	int GetNumShips()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipService.GetNumShips");
		int num;
		ProcessEvent(this, fn, &num);
		return num;
	}
};

struct AAthenaGameState {
	char pad[0x05B8];
	class AWindService* WindService;                                             // 0x05B8(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class APlayerManagerService* PlayerManagerService;                                    // 0x05C0(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AShipService* ShipService;                                             // 0x05C8(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AWatercraftService* WatercraftService;                                       // 0x05D0(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class ATimeService* TimeService;                                             // 0x05D8(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class UHealthCustomizationService* HealthService;                                           // 0x05E0(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UCustomWeatherService* CustomWeatherService;                                    // 0x05E8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UCustomStatusesService* CustomStatusesService;                                   // 0x05F0(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AFFTWaterService* WaterService;                                            // 0x05F8(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AStormService* StormService;                                            // 0x0600(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class ACrewService* CrewService;                                             // 0x0608(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AContestZoneService* ContestZoneService;                                      // 0x0610(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AContestRowboatsService* ContestRowboatsService;                                  // 0x0618(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AIslandService* IslandService;                                           // 0x0620(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class ANPCService* NPCService;                                              // 0x0628(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class ASkellyFortService* SkellyFortService;                                       // 0x0630(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class ADeepSeaRegionService* DeepSeaRegionService;                                    // 0x0638(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AAIDioramaService* AIDioramaService;                                        // 0x0640(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AAshenLordEncounterService* AshenLordEncounterService;                               // 0x0648(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AAggressiveGhostShipsEncounterService* AggressiveGhostShipsEncounterService;                    // 0x0650(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class ATallTaleService* TallTaleService;                                         // 0x0658(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AAIShipObstacleService* AIShipObstacleService;                                   // 0x0660(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AAIShipService* AIShipService;                                           // 0x0668(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AAITargetService* AITargetService;                                         // 0x0670(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UShipLiveryCatalogueService* ShipLiveryCatalogueService;                              // 0x0678(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AContestManagerService* ContestManagerService;                                   // 0x0680(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class ADrawDebugService* DrawDebugService;                                        // 0x0688(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AWorldEventZoneService* WorldEventZoneService;                                   // 0x0690(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UWorldResourceRegistry* WorldResourceRegistry;                                   // 0x0698(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AKrakenService* KrakenService;                                           // 0x06A0(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class UPlayerNameService* PlayerNameService;                                       // 0x06A8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class ATinySharkService* TinySharkService;                                        // 0x06B0(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AProjectileService* ProjectileService;                                       // 0x06B8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class ULaunchableProjectileService* LaunchableProjectileService;                             // 0x06C0(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UServerNotificationsService* ServerNotificationsService;                              // 0x06C8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AAIManagerService* AIManagerService;                                        // 0x06D0(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AAIEncounterService* AIEncounterService;                                      // 0x06D8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AAIEncounterGenerationService* AIEncounterGenerationService;                            // 0x06E0(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UEncounterService* EncounterService;                                        // 0x06E8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UGameEventSchedulerService* GameEventSchedulerService;                               // 0x06F0(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UHideoutService* HideoutService;                                          // 0x06F8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UAthenaStreamedLevelService* StreamedLevelService;                                    // 0x0700(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class ULocationProviderService* LocationProviderService;                                 // 0x0708(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AHoleService* HoleService;                                             // 0x0710(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class APlayerBuriedItemService* PlayerBuriedItemService;                                 // 0x0718(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class ULoadoutService* LoadoutService;                                          // 0x0720(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UOcclusionService* OcclusionService;                                        // 0x0728(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UPetsService* PetsService;                                             // 0x0730(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UAthenaAITeamsService* AthenaAITeamsService;                                    // 0x0738(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AAllianceService* AllianceService;                                         // 0x0740(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class UMaterialAccessibilityService* MaterialAccessibilityService;                            // 0x0748(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AReapersMarkService* ReapersMarkService;                                      // 0x0750(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AEmissaryLevelService* EmissaryLevelService;                                    // 0x0758(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class ACampaignService* CampaignService;                                         // 0x0760(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AStoryService* StoryService;                                            // 0x0768(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AStorySpawnedActorsService* StorySpawnedActorsService;                               // 0x0770(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AFlamesOfFateSettingsService* FlamesOfFateSettingsService;                             // 0x0778(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AServiceStatusNotificationsService* ServiceStatusNotificationsService;                       // 0x0780(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class UMigrationService* MigrationService;                                        // 0x0788(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AShroudBreakerService* ShroudBreakerService;                                    // 0x0790(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UServerUpdateReportingService* ServerUpdateReportingService;                            // 0x0798(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AGenericMarkerService* GenericMarkerService;                                    // 0x07A0(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AMechanismsService* MechanismsService;                                       // 0x07A8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UMerchantContractsService* MerchantContractsService;                                // 0x07B0(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UShipFactory* ShipFactory;                                             // 0x07B8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class URewindPhysicsService* RewindPhysicsService;                                    // 0x07C0(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UNotificationMessagesDataAsset* NotificationMessagesDataAsset;                           // 0x07C8(0x0008) Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class AProjectileCooldownService* ProjectileCooldownService;                               // 0x07D0(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UIslandReservationService* IslandReservationService;                                // 0x07D8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class APortalService* PortalService;                                           // 0x07E0(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UMeshMemoryConstraintService* MeshMemoryConstraintService;                             // 0x07E8(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class ABootyStorageService* BootyStorageService;                                     // 0x07F0(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class ASpireService* SpireService;                                            // 0x07F8(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class AFireworkService* FireworkService;                                         // 0x0800(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class UAirGivingService* AirGivingService;                                        // 0x0808(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UCutsceneService* CutsceneService;                                         // 0x0810(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class ACargoRunService* CargoRunService;                                         // 0x0818(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class ACommodityDemandService* CommodityDemandService;                                  // 0x0820(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class ADebugTeleportationDestinationService* DebugTeleportationDestinationService;                    // 0x0828(0x0008) Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash
	class ASeasonProgressionUIService* SeasonProgressionUIService;                              // 0x0830(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UTransientActorService* TransientActorService;                                   // 0x0838(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UTunnelsOfTheDamnedService* TunnelsOfTheDamnedService;                               // 0x0840(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UWorldSequenceService* WorldSequenceService;                                    // 0x0848(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UItemLifetimeManagerService* ItemLifetimeManagerService;                              // 0x0850(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class USeaFortsService* SeaFortsService;                                         // 0x0858(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class ABeckonService* BeckonService;                                           // 0x0860(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UVolcanoService* VolcanoService;                                          // 0x0868(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash
	class UShipAnnouncementService* ShipAnnouncementService;                                 // 0x0870(0x0008) ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash

};

struct UItemDesc {
	char pad[0x28];
	FString* Title; // 0x28(0x38)
};

struct AItemInfo {
	char pad[0x430];
	UItemDesc* Desc; // 0x430(0x08)
};

// Class Engine.Character
// Size: 0x5e0 (Inherited: 0x448)
struct ACharacter : APawn {
	char pad[0x198];

	bool IsLoading() {
		static auto fn = UObject::FindObject<UFunction>("Function AthenaLoadingScreen.AthenaLoadingScreenBlueprintFunctionLibrary.IsLoadingScreenVisible");
		bool isLoading = true;
		ProcessEvent(this, fn, &isLoading);
		return isLoading;
	}

	inline bool isItem() {
		static auto obj = UObject::FindClass("Class Athena.ItemProxy");
		return IsA(obj);
	}

	inline bool isFarShip() {
		static auto obj = UObject::FindClass("Class Athena.ShipNetProxy");
		return IsA(obj);
	}

	inline bool isShip() {
		static auto obj = UObject::FindClass("Class Athena.Ship");
		return IsA(obj);
	}

	inline bool isGhostShip() {
		static auto obj = UObject::FindClass("Class Athena.AggressiveGhostShip");
		return IsA(obj);
	}

	inline bool isSkeleton() {
		static auto obj = FindClass("Class Athena.AthenaAICharacter");
		return IsA(obj);
	}

	inline bool isPlayer() {
		static auto obj = UObject::FindClass("Class Athena.AthenaPlayerCharacter");
		return IsA(obj);
	}

	inline bool isAmmoChest() {
		static auto obj = FindClass("Class Athena.AmmoChest");
		return IsA(obj);
	}

	inline bool isXMarkMap() {
		static auto obj = UObject::FindClass("Class Athena.XMarksTheSpotMap");
		return IsA(obj);
	}

	inline bool isShipwreck() {
		static auto obj = UObject::FindClass("Class Athena.Shipwreck");
		return IsA(obj);
	}

	inline bool isMermaid() {
		static auto obj = UObject::FindClass("Class Athena.Mermaid");
		return IsA(obj);
	}

	inline bool isRowboat() {
		static auto obj = UObject::FindClass("Class Watercrafts.Rowboat");
		return IsA(obj);
	}

	inline bool isShark() {
		static auto obj = UObject::FindClass("Class Athena.SharkPawn");
		return IsA(obj);
	}

	inline bool isEvent() {
		static auto obj = UObject::FindClass("Class Athena.GameplayEventSignal");
		return IsA(obj);
	}

	inline bool isPuzzleVault() {
		static auto obj = UObject::FindClass("Class Athena.PuzzleVault");
		return IsA(obj);
	}

	inline bool isBarrel() {
		static auto obj = UObject::FindClass("Class Athena.StorageContainer");
		return IsA(obj);
	}

	inline bool isAnimal() {
		static auto obj = UObject::FindClass("Class AthenaAI.Fauna");
		return IsA(obj);
	}

	bool IsDead() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.IsDead");
		bool isDead = true;
		ProcessEvent(this, fn, &isDead);
		return isDead;
	}

	bool IsInWater() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.IsInWater");
		bool isInWater = false;
		ProcessEvent(this, fn, &isInWater);
		return isInWater;
	}

	bool isWeapon() {
		static auto obj = UObject::FindClass("Class Athena.ProjectileWeapon");
		return IsA(obj);
	}

	bool isSpyglass() {
		static auto obj = UObject::FindClass("Class Athena.Spyglass");
		return IsA(obj);
	}

	inline bool isCannon() {
		static auto obj = UObject::FindClass("Class Athena.Cannon");
		return IsA(obj);
	}

	inline bool isMapTable() {
		static auto obj = UObject::FindClass("Class Athena.MapTable");
		return IsA(obj);
	}

	AHullDamage* GetHullDamage() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetHullDamage");
		AHullDamage* params = nullptr;
		ProcessEvent(this, fn, &params);
		return params;
	}

	AShipInternalWater* GetInternalWater() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetInternalWater");
		AShipInternalWater* params = nullptr;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector GetActorForwardVector() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorForwardVector");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector GetActorUpVector() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorUpVector");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	ACharacter* GetParentActor() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetParentActor");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	};

	ACharacter* GetAttachParentActor() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetAttachParentActor");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}

	ACharacter* GetCurrentShip() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.GetCurrentShip");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}

	FVector GetVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetVelocity");
		FVector velocity;
		ProcessEvent(this, fn, &velocity);
		return velocity;
	}

	FVector GetForwardVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorForwardVector");
		FVector ForwardVelocity;
		ProcessEvent(this, fn, &ForwardVelocity);
		return ForwardVelocity;
	}

	FVector K2_GetActorLocation() {
		static auto fn = FindObject<UFunction>("Function Engine.Actor.K2_GetActorLocation");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FRotator K2_GetActorRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_GetActorRotation");
		FRotator params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	AItemInfo* GetItemInfo() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ItemProxy.GetItemInfo");
		AItemInfo* info = nullptr;
		ProcessEvent(this, fn, &info);
		return info;
	}

	void GetActorBounds(bool bOnlyCollidingComponents, FVector& Origin, FVector& BoxExtent) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorBounds");
		struct
		{
			bool bOnlyCollidingComponents = false;
			FVector Origin;
			FVector BoxExtent;
		} params;

		params.bOnlyCollidingComponents = bOnlyCollidingComponents;

		ProcessEvent(this, fn, &params);

		Origin = params.Origin;
		BoxExtent = params.BoxExtent;
	}

	float GetTargetFOV(class AAthenaPlayerCharacter* Character) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.FOVHandlerFunctions.GetTargetFOV");
		struct
		{
			AAthenaPlayerCharacter* Character;
			float ReturnValue = 0.f;
		}params;
		params.Character = Character;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	inline bool compareName(const char* name)
	{
		auto type = GetName();
		if (type.find(name) != std::string::npos)
			return true;
		return false;
	}
};

struct FFloatRange {
	float pad1;
	float min;
	float pad2;
	float max;
};

struct ACannonLoadedItemInfo
{
	char pad[0x758];
	struct AItemInfo* LoadedItemInfo; // 0x758(0x08)
};

struct ACannon {
	char pad_4324[0x0528];
	struct USkeletalMeshComponent* BaseMeshComponent; // 0x528(0x08)
	struct UStaticMeshComponent* BarrelMeshComponent; // 0x530(0x08)
	struct UStaticMeshComponent* FuseMeshComponent; // 0x538(0x08)
	struct UReplicatedShipPartCustomizationComponent* CustomizationComponent; // 0x540(0x08)
	struct ULoadableComponent* LoadableComponent; // 0x548(0x08)
	struct ULoadingPointComponent* LoadingPointComponent; // 0x550(0x08)
	struct UChildActorComponent* CannonBarrelInteractionComponent; // 0x558(0x08)
	struct UFuseComponent* FuseComponent; // 0x560(0x08)
	struct FName CameraSocket; // 0x568(0x08)
	struct FName CameraInsideCannonSocket; // 0x570(0x08)
	struct FName LaunchSocket; // 0x578(0x08)
	struct FName TooltipSocket; // 0x580(0x08)
	struct FName AudioAimRTPCName; // 0x588(0x08)
	struct FName InsideCannonRTPCName; // 0x590(0x08)
	struct UClass* ProjectileClass; // 0x598(0x08)
	float TimePerFire; // 0x5a0(0x04)
	float ProjectileSpeed; // 0x5a4(0x04)
	float ProjectileGravityScale; // 0x5a8(0x04)
	struct FFloatRange PitchRange; // 0x5ac(0x10)
	struct FFloatRange YawRange; // 0x5bc(0x10)
	float PitchSpeed; // 0x5cc(0x04)
	float YawSpeed; // 0x5d0(0x04)
	char UnknownData_5D4[0x4]; // 0x5d4(0x04)
	struct UClass* CameraShake; // 0x5d8(0x08)
	float ShakeInnerRadius; // 0x5e0(0x04)
	float ShakeOuterRadius; // 0x5e4(0x04)
	float CannonFiredAINoiseRange; // 0x5e8(0x04)
	struct FName AINoiseTag; // 0x5ec(0x08)
	unsigned char                                      UnknownData_WIJI[0x4];                                     // 0x05F4(0x0004) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	char pad_956335424[0x18];
	unsigned char                                      UnknownData_YCZ3[0x20];                                    // 0x05F4(0x0020) FIX WRONG TYPE SIZE OF PREVIOUS PROPERTY
	char pad_9335443224[0x18];
	unsigned char                                      UnknownData_BEDV[0x20];                                    // 0x0630(0x0020) FIX WRONG TYPE SIZE OF PREVIOUS PROPERTY
	float DefaultFOV; // 0x668(0x04)
	float AimFOV; // 0x66c(0x04)
	float IntoAimBlendSpeed; // 0x670(0x04)
	float OutOfAimBlendSpeed; // 0x674(0x04)
	struct UWwiseEvent* FireSfx; // 0x678(0x08)
	struct UWwiseEvent* DryFireSfx; // 0x680(0x08)
	struct UWwiseEvent* LoadingSfx_Play; // 0x688(0x08)
	struct UWwiseEvent* LoadingSfx_Stop; // 0x690(0x08)
	struct UWwiseEvent* UnloadingSfx_Play; // 0x698(0x08)
	struct UWwiseEvent* UnloadingSfx_Stop; // 0x6a0(0x08)
	struct UWwiseEvent* LoadedPlayerSfx; // 0x6a8(0x08)
	struct UWwiseEvent* UnloadedPlayerSfx; // 0x6b0(0x08)
	struct UWwiseEvent* FiredPlayerSfx; // 0x6b8(0x08)
	struct UWwiseObjectPoolWrapper* SfxPool; // 0x6c0(0x08)
	struct UWwiseEvent* StartPitchMovement; // 0x6c8(0x08)
	struct UWwiseEvent* StopPitchMovement; // 0x6d0(0x08)
	struct UWwiseEvent* StartYawMovement; // 0x6d8(0x08)
	struct UWwiseEvent* StopYawMovement; // 0x6e0(0x08)
	struct UWwiseEvent* StopMovementAtEnd; // 0x6e8(0x08)
	struct UWwiseObjectPoolWrapper* SfxMovementPool; // 0x6f0(0x08)
	struct UObject* FuseVfxFirstPerson; // 0x6f8(0x08)
	struct UObject* FuseVfxThirdPerson; // 0x700(0x08)
	struct UObject* MuzzleFlashVfxFirstPerson; // 0x708(0x08)
	struct UObject* MuzzleFlashVfxThirdPerson; // 0x710(0x08)
	struct FName FuseSocketName; // 0x718(0x08)
	struct FName BarrelSocketName; // 0x720(0x08)
	struct UClass* RadialCategoryFilter; // 0x728(0x08)
	struct UClass* DefaultLoadedItemDesc; // 0x730(0x08)
	float ClientRotationBlendTime; // 0x738(0x04)
	char UnknownData_73C[0x4]; // 0x73c(0x04)
	char UnknownData_740[0x18]; // 0x73c(0x18)
	struct AItemInfo* LoadedItemInfo; // 0x740(0x08) // fix: 0x758
	unsigned char UnknownData_LECI[0xc]; // 0x748(0x0c)
	float ServerPitch; // 0x754(0x04)
	float ServerYaw; // 0x758(0x04)

	void HandlePitchInput(float Pitch)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.HandlePitchInput");
		struct {
			float Pitch;
		} params;
		params.Pitch = Pitch;

		ProcessEvent(this, fn, &params);
	}

	void HandleYawInput(float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.HandleYawInput");
		struct {
			float Yaw;
		} params;
		params.Yaw = Yaw;

		ProcessEvent(this, fn, &params);
	}

	void ForceAimCannon(float Pitch, float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.ForceAimCannon");
		struct {
			float Pitch;
			float Yaw;
		} params;
		params.Pitch = Pitch;
		params.Yaw = Yaw;
		ProcessEvent(this, fn, &params);
	}

	bool IsReadyToFire() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.IsReadyToFire");
		bool is_ready = true;
		ProcessEvent(this, fn, &is_ready);
		return is_ready;
	}

	void Fire() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Fire");
		ProcessEvent(this, fn, nullptr);
	}

	bool IsReadyToReload() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.IsReadyToReload");
		bool is_ready = true;
		ProcessEvent(this, fn, &is_ready);
		return is_ready;
	}

	void Reload() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Reload");
		ProcessEvent(this, fn, nullptr);
	}

	void Server_Fire(float Pitch, float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Server_Fire");
		struct {
			float Pitch;
			float Yaw;
		} params;
		params.Pitch = Pitch;
		params.Yaw = Yaw;
		ProcessEvent(this, fn, &params);
	}

};

struct ASlidingDoor {
	char pad_0x0[0x52C];
	FVector InitialDoorMeshLocation; // 0x052C

	void OpenDoor() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.SkeletonFortDoor.OpenDoor");
		ProcessEvent(this, fn, nullptr);
	}
};

struct APuzzleVault {
	char pad[0x1000];
	ASlidingDoor* OuterDoor; // 0x1000
};

struct FXMarksTheSpotMapMark
{
	struct FVector2D                                   Position;                                                  // 0x0000(0x0008) (BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
	float                                              Rotation;                                                  // 0x0008(0x0004) (BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	bool                                               IsUnderground;                                             // 0x000C(0x0001) (BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
	unsigned char                                      UnknownData00[0x3];
};

struct AXMarksTheSpotMap
{
	char pad1[0x0818];
	struct FString                                     MapTexturePath;                                            // 0x0818(0x0010) (Net, ZeroConstructor, RepNotify, HasGetValueTypeHash)
	char pad2[0x80];
	TArray<struct FXMarksTheSpotMapMark>               Marks;                                                     // 0x08A8(0x0010) (Net, ZeroConstructor, RepNotify)
	char pad3[0x18];
	float                                              Rotation;                                                  // 0x08D0(0x0004) (Net, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

struct UWieldedItemComponent {
	char pad[0x02F0];
	ACharacter* CurrentlyWieldedItem; // 0x02F0
};
// Class Athena.AthenaCharacter
// Size: 0xba0 (Inherited: 0x5e0)
struct AAthenaCharacter : ACharacter {
	char pad[0x250];
	UWieldedItemComponent* WieldedItemComponent; // 0x0830
	char pad2[0x368];
};

// Class Athena.AthenaPlayerCharacter
// Size: 0x1a60 (Inherited: 0xba0)
struct AAthenaPlayerCharacter : AAthenaCharacter {
	char pad[0x168];
	struct UDrowningComponent* DrowningComponent; // 0xd08(0x08)
	char pad2[0xD50];

	ACharacter* GetWieldedItem() {
		if (!WieldedItemComponent) return nullptr;
		return WieldedItemComponent->CurrentlyWieldedItem;
	}

};

// Class Athena.DrowningComponent
// Size: 0x1c0 (Inherited: 0xc8)
struct UDrowningComponent {
	char pad[0x108];
	float OxygenLevel; // 0x108(0x04)
	char pad2[0xB4];

	float GetOxygenLevel() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.DrowningComponent.GetOxygenLevel");
		float oxygen;
		ProcessEvent(this, fn, &oxygen);
		return oxygen;
	} // Function Athena.DrowningComponent.GetOxygenLevel // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x39cb270
};

struct AGameCustomizationService_SetTime_Params
{
	int Hours;

	void SetTime(int Hours)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.GameCustomizationService.SetTime");
		AGameCustomizationService_SetTime_Params params;
		params.Hours = Hours;
		ProcessEvent(this, fn, &params);
	}
};

struct FMinimalViewInfo {
	FVector Location;
	FRotator Rotation;
	char UnknownData_18[0x10];
	//float FOV;
};

struct FCameraCacheEntry {
	float TimeStamp;
	char pad[0xc];
	FMinimalViewInfo POV;
};

struct FTViewTarget
{
	class AActor* Target;                                                    // 0x0000(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      UnknownData_KHBN[0x8];                                     // 0x0008(0x0008) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	struct FMinimalViewInfo                            POV;                                                       // 0x0010(0x05A0) (Edit, BlueprintVisible)
	class APlayerState* PlayerState;                                               // 0x05B0(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	unsigned char                                      UnknownData_QIMB[0x8];                                     // 0x05B8(0x0008) MISSED OFFSET (PADDING)

};

struct APlayerCameraManager {
	char pad[0x0440];
	FCameraCacheEntry CameraCache; // 0x0440
	FCameraCacheEntry LastFrameCameraCache;
	FTViewTarget ViewTarget;
	FTViewTarget PendingViewTarget;


	FVector GetCameraLocation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.GetCameraLocation");
		FVector location;
		ProcessEvent(this, fn, &location);
		return location;
	};
	FRotator GetCameraRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.GetCameraRotation");
		FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}
};

struct FKey
{
	FName KeyName;
	unsigned char UnknownData00[0x18] = {};

	FKey() {};
	FKey(const char* InName) : KeyName(FName(InName)) {}
};

struct AController_K2_GetPawn_Params
{
	class APawn* ReturnValue;
};

struct AController {

	char pad_0000[0x03E8];
	class ACharacter* Character; //0x03E8
	char pad_0480[0x70];
	APlayerCameraManager* PlayerCameraManager; //0x0460
	char pad_04f8[0x1031];
	bool IdleDisconnectEnabled; // 0x14D1

	void UnPossess()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.UnPossess");
		ProcessEvent(this, fn, nullptr);
	}

	void SetTime(int Hours)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.GameCustomizationService.SetTime");
		AGameCustomizationService_SetTime_Params params;
		params.Hours = Hours;
		ProcessEvent(this, fn, &params);
	}

	void SendToConsole(FString& cmd) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.SendToConsole");
		ProcessEvent(this, fn, &cmd);
	}

	bool WasInputKeyJustPressed(const FKey& Key) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.WasInputKeyJustPressed");
		struct
		{
			FKey Key;

			bool ReturnValue = false;
		} params;

		params.Key = Key;
		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	APawn* K2_GetPawn() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.K2_GetPawn");
		AController_K2_GetPawn_Params params;
		class APawn* ReturnValue;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	bool ProjectWorldLocationToScreen(const FVector& WorldLocation, FVector2D& ScreenLocation) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.ProjectWorldLocationToScreen");
		struct
		{
			FVector WorldLocation;
			FVector2D ScreenLocation;
			bool ReturnValue = false;
		} params;

		params.WorldLocation = WorldLocation;
		ProcessEvent(this, fn, &params);
		ScreenLocation = params.ScreenLocation;
		return params.ReturnValue;
	}

	FRotator GetControlRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Pawn.GetControlRotation");
		struct FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}

	FRotator GetDesiredRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Pawn.GetDesiredRotation");
		struct FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}

	void AddYawInput(float Val) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.AddYawInput");
		ProcessEvent(this, fn, &Val);
	}

	void AddPitchInput(float Val) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.AddPitchInput");
		ProcessEvent(this, fn, &Val);
	}

	void FOV(float NewFOV) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.FOV");
		ProcessEvent(this, fn, &NewFOV);
	}

	bool LineOfSightTo(ACharacter* Other, const FVector& ViewPoint, const bool bAlternateChecks) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.LineOfSightTo");
		struct {
			ACharacter* Other = nullptr;
			FVector ViewPoint;
			bool bAlternateChecks = false;
			bool ReturnValue = false;
		} params;
		params.Other = Other;
		params.ViewPoint = ViewPoint;
		params.bAlternateChecks = bAlternateChecks;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}
};

// Class Engine.PlayerController
// Size: 0x728 (Inherited: 0x430)
struct APlayerController : AController {
	struct UPlayer* Player; // 0x430(0x08)
	char UnknownData_438[0x8]; // 0x438(0x08)
	struct APawn* AcknowledgedPawn; // 0x440(0x08)
	struct UInterpTrackInstDirector* ControllingDirTrackInst; // 0x448(0x08)
	char UnknownData_450[0x8]; // 0x450(0x08)
	struct AHUD* MyHUD; // 0x458(0x08)
	struct APlayerCameraManager* PlayerCameraManager; // 0x460(0x08)
	struct UClass* PlayerCameraManagerClass; // 0x468(0x08)
	bool bAutoManageActiveCameraTarget; // 0x470(0x01)
	char UnknownData_471[0x3]; // 0x471(0x03)
	struct FRotator TargetViewRotation; // 0x474(0x0c)
	char UnknownData_480[0x10]; // 0x480(0x10)
	struct TArray<struct AActor*> HiddenActors; // 0x490(0x10)
	float LastSpectatorStateSynchTime; // 0x4a0(0x04)
	struct FVector LastSpectatorSyncLocation; // 0x4a4(0x0c)
	struct FRotator LastSpectatorSyncRotation; // 0x4b0(0x0c)
	int32_t ClientCap; // 0x4bc(0x04)
	struct UCheatManager* CheatManager; // 0x4c0(0x08)
	struct UClass* CheatClass; // 0x4c8(0x08)
	struct UPlayerInput* PlayerInput; // 0x4d0(0x08)
	struct TArray<struct FActiveForceFeedbackEffect> ActiveForceFeedbackEffects; // 0x4d8(0x10)
	char UnknownData_4E8[0x90]; // 0x4e8(0x90)
	char UnknownData_578_0 : 2; // 0x578(0x01)
	char bPlayerIsWaiting : 1; // 0x578(0x01)
	char UnknownData_578_3 : 5; // 0x578(0x01)
	char UnknownData_579[0x3]; // 0x579(0x03)
	char NetPlayerIndex; // 0x57c(0x01)
	bool bHasVoiceHandshakeCompleted; // 0x57d(0x01)
	char UnknownData_57E[0x2]; // 0x57e(0x02)
	struct UNetConnection* PendingSwapConnection; // 0x580(0x08)
	struct UNetConnection* NetConnection; // 0x588(0x08)
	char UnknownData_590[0xc]; // 0x590(0x0c)
	float InputYawScale; // 0x59c(0x04)
	float InputPitchScale; // 0x5a0(0x04)
	float InputRollScale; // 0x5a4(0x04)
	char bShowMouseCursor : 1; // 0x5a8(0x01)
	char bEnableClickEvents : 1; // 0x5a8(0x01)
	char bEnableTouchEvents : 1; // 0x5a8(0x01)
	char bEnableMouseOverEvents : 1; // 0x5a8(0x01)
	char bEnableTouchOverEvents : 1; // 0x5a8(0x01)
	char bForceFeedbackEnabled : 1; // 0x5a8(0x01)
	char UnknownData_5A8_6 : 2; // 0x5a8(0x01)
	char UnknownData_5A9[0x3]; // 0x5a9(0x03)
	char DefaultMouseCursor; // 0x5ac(0x01)
	char CurrentMouseCursor; // 0x5ad(0x01)
	char DefaultClickTraceChannel; // 0x5ae(0x01)
	char CurrentClickTraceChannel; // 0x5af(0x01)
	float HitResultTraceDistance; // 0x5b0(0x04)
	float ForceFeedbackIntensity; // 0x5b4(0x04)
	char UnknownData_5B8[0x88]; // 0x5b8(0x88)
	struct UInputComponent* InactiveStateInputComponent; // 0x640(0x08)
	char UnknownData_648[0x20]; // 0x648(0x20)
	struct UTouchInterface* CurrentTouchInterface; // 0x668(0x08)
	char UnknownData_670[0x30]; // 0x670(0x30)
	struct ASpectatorPawn* SpectatorPawn; // 0x6a0(0x08)
	struct FVector SpawnLocation; // 0x6a8(0x0c)
	char UnknownData_6B4[0x4]; // 0x6b4(0x04)
	char UnknownData_6B8[0x50]; // 0x6b8(0x50)
	char UnknownData_708[0x14]; // 0x708(0x14)
	bool bIsLocalPlayerController; // 0x71c(0x01)
	char UnknownData_71D[0x1]; // 0x71d(0x01)
	uint16_t SeamlessTravelCount; // 0x71e(0x02)
	uint16_t LastCompletedSeamlessTravelCount; // 0x720(0x02)
	bool bInCinematicMode; // 0x722(0x01)
	char UnknownData_723[0x5]; // 0x723(0x05)

	bool WasInputKeyJustReleased(struct FKey Key); // Function Engine.PlayerController.WasInputKeyJustReleased // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b33810
	bool WasInputKeyJustPressed(struct FKey Key); // Function Engine.PlayerController.WasInputKeyJustPressed // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b33710
	void ToggleSpeaking(bool bInSpeaking); // Function Engine.PlayerController.ToggleSpeaking // Exec|Native|Public // @ game+0x2b33340
	void SwitchLevel(struct FString URL); // Function Engine.PlayerController.SwitchLevel // Exec|Native|Public // @ game+0x2b33250
	void StopHapticEffect(char Hand); // Function Engine.PlayerController.StopHapticEffect // Final|Native|Public|BlueprintCallable // @ game+0x2b331b0
	void StartFire(char FireModeNum); // Function Engine.PlayerController.StartFire // Exec|Native|Public // @ game+0x2b32d80
	void SetVirtualJoystickVisibility(bool bVisible); // Function Engine.PlayerController.SetVirtualJoystickVisibility // Native|Public|BlueprintCallable // @ game+0x2b326a0
	void SetViewTargetWithBlend(struct AActor* NewViewTarget, float BlendTime, char BlendFunc, float BlendExp, bool bLockOutgoing); // Function Engine.PlayerController.SetViewTargetWithBlend // Native|Public|BlueprintCallable // @ game+0x2b32520
	void SetName(struct FString S); // Function Engine.PlayerController.SetName // Exec|Native|Public // @ game+0x2b31ea0
	void SetIgnoreMoveInput(bool bNewMoveInput); // Function Engine.PlayerController.SetIgnoreMoveInput // Native|Public|BlueprintCallable // @ game+0x2b319e0
	void SetIgnoreLookInput(bool bNewLookInput); // Function Engine.PlayerController.SetIgnoreLookInput // Native|Public|BlueprintCallable // @ game+0x2b31950
	void SetHapticsByValue(float Frequency, float Amplitude, char Hand); // Function Engine.PlayerController.SetHapticsByValue // Final|Native|Public|BlueprintCallable // @ game+0x2b31770
	void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning); // Function Engine.PlayerController.SetCinematicMode // Native|Public|BlueprintCallable // @ game+0x2b31290
	void SetAudioListenerOverride(struct USceneComponent* AttachToComponent, struct FVector Location, struct FRotator Rotation); // Function Engine.PlayerController.SetAudioListenerOverride // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2b30ff0
	void ServerViewSelf(struct FViewTargetTransitionParams TransitionParams); // Function Engine.PlayerController.ServerViewSelf // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b30860
	void ServerViewPrevPlayer(); // Function Engine.PlayerController.ServerViewPrevPlayer // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b30810
	void ServerViewNextPlayer(); // Function Engine.PlayerController.ServerViewNextPlayer // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b307c0
	void ServerVerifyViewTarget(); // Function Engine.PlayerController.ServerVerifyViewTarget // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30770
	void ServerUpdateCamera(struct FVector_NetQuantize CamLoc, int32_t CamPitchAndYaw); // Function Engine.PlayerController.ServerUpdateCamera // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b30650
	void ServerUnmutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ServerUnmutePlayer // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30500
	void ServerToggleAILogging(); // Function Engine.PlayerController.ServerToggleAILogging // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b304b0
	void ServerShortTimeout(); // Function Engine.PlayerController.ServerShortTimeout // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30460
	void ServerSetSpectatorWaiting(bool bWaiting); // Function Engine.PlayerController.ServerSetSpectatorWaiting // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b303a0
	void ServerSetSpectatorLocation(struct FVector NewLoc, struct FRotator NewRot); // Function Engine.PlayerController.ServerSetSpectatorLocation // Net|Native|Event|Public|NetServer|HasDefaults|NetValidate // @ game+0x2b30270
	void ServerRestartPlayer(); // Function Engine.PlayerController.ServerRestartPlayer // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30220
	void ServerPause(); // Function Engine.PlayerController.ServerPause // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b301d0
	void ServerNotifyLoadedWorld(struct FName WorldPackageName); // Function Engine.PlayerController.ServerNotifyLoadedWorld // Final|Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30120
	void ServerMutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ServerMutePlayer // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2ffd0
	void ServerCheckClientPossessionReliable(); // Function Engine.PlayerController.ServerCheckClientPossessionReliable // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2ff80
	void ServerCheckClientPossession(); // Function Engine.PlayerController.ServerCheckClientPossession // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b2ff30
	void ServerChangeName(struct FString S); // Function Engine.PlayerController.ServerChangeName // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2fe60
	void ServerCamera(struct FName NewMode); // Function Engine.PlayerController.ServerCamera // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2fda0
	void ServerAcknowledgePossession(struct APawn* P); // Function Engine.PlayerController.ServerAcknowledgePossession // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2fce0
	void SendToConsole(struct FString Command); // Function Engine.PlayerController.SendToConsole // Exec|Native|Public // @ game+0x2b2fc30
	void RestartLevel(); // Function Engine.PlayerController.RestartLevel // Exec|Native|Public // @ game+0x2b2fc10
	void ResetIgnoreMoveInput(); // Function Engine.PlayerController.ResetIgnoreMoveInput // Native|Public|BlueprintCallable // @ game+0x2b2fbd0
	void ResetIgnoreLookInput(); // Function Engine.PlayerController.ResetIgnoreLookInput // Native|Public|BlueprintCallable // @ game+0x2b2fbb0
	void ResetIgnoreInputFlags(); // Function Engine.PlayerController.ResetIgnoreInputFlags // Native|Public|BlueprintCallable // @ game+0x2b2fb90
	bool ProjectWorldLocationToScreen(const FVector& WorldLocation, FVector2D& ScreenLocation) // Function Engine.PlayerController.ProjectWorldLocationToScreen // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2f450
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.ProjectWorldLocationToScreen");
		struct
		{
			FVector WorldLocation;
			FVector2D ScreenLocation;
			bool ReturnValue = false;
		} params;

		params.WorldLocation = WorldLocation;
		ProcessEvent(this, fn, &params);
		ScreenLocation = params.ScreenLocation;
		return params.ReturnValue;
	}
	void PlayHapticEffect(struct UHapticFeedbackEffect* HapticEffect, char Hand, float Scale); // Function Engine.PlayerController.PlayHapticEffect // Final|Native|Public|BlueprintCallable // @ game+0x2b2f2b0
	void PlayDynamicForceFeedback(float Intensity, float Duration, bool bAffectsLeftLarge, bool bAffectsLeftSmall, bool bAffectsRightLarge, bool bAffectsRightSmall, char Action, struct FLatentActionInfo LatentInfo); // Function Engine.PlayerController.PlayDynamicForceFeedback // Final|Native|Public|BlueprintCallable // @ game+0x2b2f020
	void Pause(); // Function Engine.PlayerController.Pause // Exec|Native|Public // @ game+0x2b2eaf0
	void OnServerStartedVisualLogger(bool bIsLogging); // Function Engine.PlayerController.OnServerStartedVisualLogger // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b2ea60
	void LocalTravel(struct FString URL); // Function Engine.PlayerController.LocalTravel // Exec|Native|Public // @ game+0x2b2e5e0
	bool IsSeamlessTravelInProgress(); // Function Engine.PlayerController.IsSeamlessTravelInProgress // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a930
	bool IsPossessingServerAcknowledgedPawn(); // Function Engine.PlayerController.IsPossessingServerAcknowledgedPawn // Final|Native|Public|BlueprintCallable|BlueprintPure // @ game+0x2b2a8d0
	bool IsMoveInputIgnored(); // Function Engine.PlayerController.IsMoveInputIgnored // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a810
	bool IsLookInputIgnored(); // Function Engine.PlayerController.IsLookInputIgnored // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a7e0
	bool IsInputKeyDown(struct FKey Key); // Function Engine.PlayerController.IsInputKeyDown // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a680
	void GetViewportSize(int32_t SizeX, int32_t SizeY); // Function Engine.PlayerController.GetViewportSize // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a290
	struct ASpectatorPawn* GetSpectatorPawn(); // Function Engine.PlayerController.GetSpectatorPawn // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b29d20
	bool GetMousePosition(float LocationX, float LocationY); // Function Engine.PlayerController.GetMousePosition // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b29500
	struct FVector GetInputVectorKeyState(struct FKey Key); // Function Engine.PlayerController.GetInputVectorKeyState // Final|Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b292b0
	void GetInputTouchState(char FingerIndex, float LocationX, float LocationY, bool bIsCurrentlyPressed); // Function Engine.PlayerController.GetInputTouchState // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b29010
	void GetInputMouseDelta(float DeltaX, float DeltaY); // Function Engine.PlayerController.GetInputMouseDelta // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28f30
	void GetInputMotionState(struct FVector Tilt, struct FVector RotationRate, struct FVector Gravity, struct FVector Acceleration); // Function Engine.PlayerController.GetInputMotionState // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28da0
	float GetInputKeyTimeDown(struct FKey Key); // Function Engine.PlayerController.GetInputKeyTimeDown // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28ca0
	void GetInputAnalogStickState(char WhichStick, float StickX, float StickY); // Function Engine.PlayerController.GetInputAnalogStickState // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b289e0
	float GetInputAnalogKeyState(struct FKey Key); // Function Engine.PlayerController.GetInputAnalogKeyState // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b288e0
	struct AHUD* GetHUD(); // Function Engine.PlayerController.GetHUD // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27e10
	bool GetHitResultUnderFingerForObjects(char FingerIndex, struct TArray<char> ObjectTypes, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderFingerForObjects // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b285d0
	bool GetHitResultUnderFingerByChannel(char FingerIndex, char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderFingerByChannel // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28430
	bool GetHitResultUnderFinger(char FingerIndex, char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderFinger // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28290
	bool GetHitResultUnderCursorForObjects(struct TArray<char> ObjectTypes, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderCursorForObjects // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28100
	bool GetHitResultUnderCursorByChannel(char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderCursorByChannel // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27fa0
	bool GetHitResultUnderCursor(char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderCursor // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27e40
	struct FVector GetFocalLocation(); // Function Engine.PlayerController.GetFocalLocation // Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27d90
	void EnableCheats(); // Function Engine.PlayerController.EnableCheats // Exec|Native|Public // @ game+0x2b265a0
	bool DeprojectScreenPositionToWorld(float ScreenX, float ScreenY, struct FVector WorldLocation, struct FVector WorldDirection); // Function Engine.PlayerController.DeprojectScreenPositionToWorld // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b26190
	bool DeprojectMousePositionToWorld(struct FVector WorldLocation, struct FVector WorldDirection); // Function Engine.PlayerController.DeprojectMousePositionToWorld // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b26090
	void ConsoleKey(struct FKey Key); // Function Engine.PlayerController.ConsoleKey // Exec|Native|Public // @ game+0x2b25e50
	void ClientWasKicked(struct FText KickReason); // Function Engine.PlayerController.ClientWasKicked // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25d20
	void ClientVoiceHandshakeComplete(); // Function Engine.PlayerController.ClientVoiceHandshakeComplete // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25d00
	void ClientUpdateLevelStreamingStatusBatched(struct FString PackageBasePath, struct TArray<struct FLevelStreamingStatusUpdateInfo> LevelStreamingStatusUpdateInfo); // Function Engine.PlayerController.ClientUpdateLevelStreamingStatusBatched // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25bd0
	void ClientUpdateLevelStreamingStatus(struct FName PackageName, bool bNewShouldBeLoaded, bool bNewShouldBeVisible, bool bNewShouldBlockOnLoad, int32_t LODIndex); // Function Engine.PlayerController.ClientUpdateLevelStreamingStatus // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25a20
	void ClientUnmutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ClientUnmutePlayer // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25930
	void ClientTravelInternal(struct FString URL, char TravelType, bool bSeamless, struct FGuid MapPackageGuid); // Function Engine.PlayerController.ClientTravelInternal // Net|NetReliableNative|Event|Public|HasDefaults|NetClient // @ game+0x2b257b0
	void ClientTravel(struct FString URL, char TravelType, bool bSeamless, struct FGuid MapPackageGuid); // Function Engine.PlayerController.ClientTravel // Final|Native|Public|HasDefaults // @ game+0x2b25630
	void ClientTeamMessage(struct APlayerState* SenderPlayerState, struct FString S, struct FName Type, float MsgLifeTime); // Function Engine.PlayerController.ClientTeamMessage // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b254c0
	void ClientStopForceFeedback(struct UForceFeedbackEffect* ForceFeedbackEffect, struct FName Tag); // Function Engine.PlayerController.ClientStopForceFeedback // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b253f0
	void ClientStopCameraShake(struct UClass* Shake); // Function Engine.PlayerController.ClientStopCameraShake // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b25360
	void ClientStopCameraAnim(struct UCameraAnim* AnimToStop); // Function Engine.PlayerController.ClientStopCameraAnim // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b252d0
	void ClientStartOnlineSession(); // Function Engine.PlayerController.ClientStartOnlineSession // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b252b0
	void ClientSpawnCameraLensEffect(struct UClass* LensEffectEmitterClass); // Function Engine.PlayerController.ClientSpawnCameraLensEffect // Net|Native|Event|Public|NetClient|BlueprintCallable // @ game+0x2b25220
	void ClientSetViewTarget(struct AActor* A, struct FViewTargetTransitionParams TransitionParams); // Function Engine.PlayerController.ClientSetViewTarget // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25100
	void ClientSetSpectatorWaiting(bool bWaiting); // Function Engine.PlayerController.ClientSetSpectatorWaiting // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25070
	void ClientSetHUD(struct UClass* NewHUDClass); // Function Engine.PlayerController.ClientSetHUD // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b24e20
	void ClientSetForceMipLevelsToBeResident(struct UMaterialInterface* Material, float ForceDuration, int32_t CinematicTextureGroups); // Function Engine.PlayerController.ClientSetForceMipLevelsToBeResident // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24d20
	void ClientSetCinematicMode(bool bInCinematicMode, bool bAffectsMovement, bool bAffectsTurning, bool bAffectsHUD); // Function Engine.PlayerController.ClientSetCinematicMode // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24bb0
	void ClientSetCameraMode(struct FName NewCamMode); // Function Engine.PlayerController.ClientSetCameraMode // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24b20
	void ClientSetCameraFade(bool bEnableFading, struct FColor FadeColor, struct FVector2D FadeAlpha, float FadeTime, bool bFadeAudio); // Function Engine.PlayerController.ClientSetCameraFade // Net|NetReliableNative|Event|Public|HasDefaults|NetClient // @ game+0x2b24990
	void ClientSetBlockOnAsyncLoading(); // Function Engine.PlayerController.ClientSetBlockOnAsyncLoading // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24970
	void ClientSeamlessTravelComplete(); // Function Engine.PlayerController.ClientSeamlessTravelComplete // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24950
	void ClientReturnToMainMenu(struct FString ReturnReason); // Function Engine.PlayerController.ClientReturnToMainMenu // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b248a0
	void ClientRetryClientRestart(struct APawn* NewPawn); // Function Engine.PlayerController.ClientRetryClientRestart // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24810
	void ClientRestart(struct APawn* NewPawn); // Function Engine.PlayerController.ClientRestart // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24780
	void ClientReset(); // Function Engine.PlayerController.ClientReset // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24760
	void ClientRepObjRef(struct UObject* Object); // Function Engine.PlayerController.ClientRepObjRef // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b246d0
	void ClientReceiveLocalizedMessage(struct UClass* Message, int32_t Switch, struct APlayerState* RelatedPlayerState_2, struct APlayerState* RelatedPlayerState_3, struct UObject* OptionalObject); // Function Engine.PlayerController.ClientReceiveLocalizedMessage // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24550
	void ClientPrestreamTextures(struct AActor* ForcedActor, float ForceDuration, bool bEnableStreaming, int32_t CinematicTextureGroups); // Function Engine.PlayerController.ClientPrestreamTextures // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24400
	void ClientPrepareMapChange(struct FName LevelName, bool bFirst, bool bLast); // Function Engine.PlayerController.ClientPrepareMapChange // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b242f0
	void ClientPlaySoundAtLocation(struct USoundBase* Sound, struct FVector Location, float VolumeMultiplier, float PitchMultiplier); // Function Engine.PlayerController.ClientPlaySoundAtLocation // Net|Native|Event|Public|HasDefaults|NetClient // @ game+0x2b241a0
	void ClientPlaySound(struct USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier); // Function Engine.PlayerController.ClientPlaySound // Net|Native|Event|Public|NetClient // @ game+0x2b240a0
	void ClientPlayForceFeedback(struct UForceFeedbackEffect* ForceFeedbackEffect, bool bLooping, struct FName Tag); // Function Engine.PlayerController.ClientPlayForceFeedback // Net|Native|Event|Public|NetClient|BlueprintCallable // @ game+0x2b23f90
	void ClientPlayCameraShake(struct UClass* Shake, float Scale, char PlaySpace, struct FRotator UserPlaySpaceRot); // Function Engine.PlayerController.ClientPlayCameraShake // Net|Native|Event|Public|HasDefaults|NetClient|BlueprintCallable // @ game+0x2b23e30
	void ClientPlayCameraAnim(struct UCameraAnim* AnimToPlay, float Scale, float Rate, float BlendInTime, float BlendOutTime, bool bLoop, bool bRandomStartTime, char Space, struct FRotator CustomPlaySpace); // Function Engine.PlayerController.ClientPlayCameraAnim // Net|Native|Event|Public|HasDefaults|NetClient|BlueprintCallable // @ game+0x2b23b90
	void ClientMutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ClientMutePlayer // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23aa0
	void ClientMessage(struct FString S, struct FName Type, float MsgLifeTime); // Function Engine.PlayerController.ClientMessage // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23970
	void ClientIgnoreMoveInput(bool bIgnore); // Function Engine.PlayerController.ClientIgnoreMoveInput // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b238e0
	void ClientIgnoreLookInput(bool bIgnore); // Function Engine.PlayerController.ClientIgnoreLookInput // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23850
	void ClientGotoState(struct FName NewState); // Function Engine.PlayerController.ClientGotoState // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b237c0
	void ClientGameEnded(struct AActor* EndGameFocus, bool bIsWinner); // Function Engine.PlayerController.ClientGameEnded // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b236f0
	void ClientForceGarbageCollection(); // Function Engine.PlayerController.ClientForceGarbageCollection // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b236d0
	void ClientFlushLevelStreaming(); // Function Engine.PlayerController.ClientFlushLevelStreaming // Final|Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b236b0
	void ClientEndOnlineSession(); // Function Engine.PlayerController.ClientEndOnlineSession // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23690
	void ClientEnableNetworkVoice(bool bEnable); // Function Engine.PlayerController.ClientEnableNetworkVoice // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23600
	void ClientCommitMapChange(); // Function Engine.PlayerController.ClientCommitMapChange // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b235e0
	void ClientClearCameraLensEffects(); // Function Engine.PlayerController.ClientClearCameraLensEffects // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b235c0
	void ClientCapBandwidth(int32_t Cap); // Function Engine.PlayerController.ClientCapBandwidth // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23530
	void ClientCancelPendingMapChange(); // Function Engine.PlayerController.ClientCancelPendingMapChange // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23510
	void ClientBlendOutCameraShake(struct UClass* Shake); // Function Engine.PlayerController.ClientBlendOutCameraShake // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b23480
	void ClientAddTextureStreamingLoc(struct FVector InLoc, float Duration, bool bOverrideLocation); // Function Engine.PlayerController.ClientAddTextureStreamingLoc // Final|Net|NetReliableNative|Event|Public|HasDefaults|NetClient // @ game+0x2b23360
	void ClearAudioListenerOverride(); // Function Engine.PlayerController.ClearAudioListenerOverride // Final|Native|Public|BlueprintCallable // @ game+0x2b23320
	void Camera(struct FName NewMode); // Function Engine.PlayerController.Camera // Exec|Native|Public // @ game+0x2b23260
	void ActivateTouchInterface(struct UTouchInterface* NewTouchInterface); // Function Engine.PlayerController.ActivateTouchInterface // Native|Public|BlueprintCallable // @ game+0x2b22540
};

struct ULevel
{
	char UnkownData[0xa0];
	TArray<class ACharacter*> AActors; // 0x00A0(0x0010)
};
/*
// Class Engine.Level
// Size: 0x300 (Inherited: 0x28)
struct ULevel : UObject {
	unsigned char UnknownData00[0x78]; // 0x0028(0x0078) MISSED OFFSET
	TArray<class AActor*> AActors; // 0x00A0(0x0010)
	unsigned char UnknownData10[0x18]; // 0x00B0(0x0018) MISSED OFFSET
	struct ULevelActorContainer* ActorCluster; // 0xc8(0x08)
	struct UWorld* OwningWorld; // 0xd0(0x08)
	struct UModel* Model; // 0xd8(0x08)
	struct TArray<struct UModelComponent*> ModelComponents; // 0xe0(0x10)
	struct ALevelScriptActor* LevelScriptActor; // 0xf0(0x08)
	struct ANavigationObjectBase* NavListStart; // 0xf8(0x08)
	struct ANavigationObjectBase* NavListEnd; // 0x100(0x08)
	struct TArray<struct UNavigationDataChunk*> NavDataChunks; // 0x108(0x10)
	float LightmapTotalSize; // 0x118(0x04)
	float ShadowmapTotalSize; // 0x11c(0x04)
	struct TArray<struct FVector> StaticNavigableGeometry; // 0x120(0x10)
	char UnknownData_130[0x130]; // 0x130(0x130)
	bool LevelVisibility; // 0x260(0x01)
	char UnknownData_261[0xf]; // 0x261(0x0f)
	char UnknownData_270_0 : 3; // 0x270(0x01)
	char Blocked : 1; // 0x270(0x01)
	char UnknownData_270_4 : 4; // 0x270(0x01)
	char UnknownData_271[0x67]; // 0x271(0x67)
	struct TArray<struct UAssetUserData*> AssetUserData; // 0x2d8(0x10)
	char UnknownData_2E8[0x18]; // 0x2e8(0x18)
};*/

struct UPlayer {
	char UnknownData00[0x30];
	APlayerController* PlayerController;
};

// Class Engine.LocalPlayer
// Size: 0x210 (Inherited: 0x48)
struct ULocalPlayer : UPlayer {
	char UnknownData_48[0x18]; // 0x48(0x18)
	struct UGameViewportClient* ViewportClient; // 0x60(0x08)
	char UnknownData_68[0x40]; // 0x68(0x40)
	char AspectRatioAxisConstraint; // 0xa8(0x01)
	char UnknownData_A9[0x7]; // 0xa9(0x07)
	struct UClass* PendingLevelPlayerControllerClass; // 0xb0(0x08)
	char bSentSplitJoin : 1; // 0xb8(0x01)
	char UnknownData_B8_1 : 7; // 0xb8(0x01)
	char UnknownData_B9[0x3]; // 0xb9(0x03)
	float MinimumAspectRatio; // 0xbc(0x04)
	char UnknownData_C0[0x150]; // 0xc0(0x150)
};

struct UGameInstance {
	char UnknownData00[0x38];
	TArray<ULocalPlayer*> LocalPlayers; // 0x38
};

// Class Engine.World
// Size: 0x7c8 (Inherited: 0x28)
struct UWorld : UObject {
	static inline UWorld** GWorld = nullptr;
	char UnknownData_28[0x8]; // 0x28(0x08)
	struct ULevel* PersistentLevel; // 0x30(0x08)
	char pad3[0x20];
	AAthenaGameState* GameState; //0x0058
	char pad4[0xF0];
	TArray<ULevel*> Levels; //0x0150
	char pad5[0x50];
	ULevel* CurrentLevel; //0x01B0	
	char pad6[0x8];
	UGameInstance* GameInstance; //0x01C0
};


// Class CoreUObject.Interface
// Size: 0x28 (Inherited: 0x28)
struct UInterface : UObject {
};

// Class CoreUObject.GCObjectReferencer
// Size: 0x60 (Inherited: 0x28)
struct UGCObjectReferencer : UObject {
	char UnknownData_28[0x38]; // 0x28(0x38)
};

// Class CoreUObject.TextBuffer
// Size: 0x50 (Inherited: 0x28)
struct UTextBuffer : UObject {
	char UnknownData_28[0x28]; // 0x28(0x28)
};

// Class CoreUObject.Field
// Size: 0x30 (Inherited: 0x28)
struct UField : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class CoreUObject.Struct
// Size: 0x88 (Inherited: 0x30)
struct UStruct : UField {
	UStruct* SuperField;
	char UnknownData_30[0x50]; // 0x28(0x50)
};

// Class CoreUObject.ScriptStruct
// Size: 0x98 (Inherited: 0x88)
struct UScriptStruct : UStruct {
	char UnknownData_88[0x10]; // 0x88(0x10)
};

// Class CoreUObject.Package
// Size: 0x80 (Inherited: 0x28)
struct UPackage : UObject {
	char UnknownData_28[0x58]; // 0x28(0x58)
};

// Class CoreUObject.Class
// Size: 0x1c0 (Inherited: 0x88)
class UClass : public UStruct
{
public:
	using UStruct::UStruct;
	unsigned char                                      UnknownData00[0x138];                                     // 0x0088(0x0138) MISSED OFFSET

	template<typename T>
	inline T* CreateDefaultObject()
	{
		return static_cast<T*>(CreateDefaultObject());
	}

	inline UObject* CreateDefaultObject()
	{
		return GetVFunction<UObject* (*)(UClass*)>(this, 88)(this);
	}
};

// Class Engine.ScriptViewportClient
// Size: 0x30 (Inherited: 0x28)
struct UScriptViewportClient : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class Engine.GameViewportClient
// Size: 0x250 (Inherited: 0x30)
struct UGameViewportClient : UScriptViewportClient {
	char UnknownData_30[0x8]; // 0x30(0x08)
	struct UConsole* ViewportConsole; // 0x38(0x08)
	char DebugProperties[0x10]; // 0x40(0x10)
	char UnknownData_50[0x30]; // 0x50(0x30)
	struct UWorld* World; // 0x80(0x08)
	struct UGameInstance* GameInstance; // 0x88(0x08)
	char UnknownData_90[0x1c0]; // 0x90(0x1c0)

	void SSSwapControllers(); // Function Engine.GameViewportClient.SSSwapControllers // Exec|Native|Public // @ game+0x2fe5ca0
	void ShowTitleSafeArea(); // Function Engine.GameViewportClient.ShowTitleSafeArea // Exec|Native|Public // @ game+0x2fe5d50
	void SetConsoleTarget(int32_t PlayerIndex); // Function Engine.GameViewportClient.SetConsoleTarget // Exec|Native|Public // @ game+0x2fe5cc0
};

// Class Athena.AthenaGameViewportClient
// Size: 0x260 (Inherited: 0x250)
struct UAthenaGameViewportClient : UGameViewportClient {
	char UnknownData_250[0x10]; // 0x250(0x10)
};

// Class CoreUObject.Function
// Size: 0xb8 (Inherited: 0x88)
struct UFunction : UStruct {
	char UnknownData_88[0x30]; // 0x88(0x30)
};

// Class CoreUObject.DelegateFunction
// Size: 0xb8 (Inherited: 0xb8)
struct UDelegateFunction : UFunction {
};

// Class CoreUObject.SparseDelegateFunction
// Size: 0xc8 (Inherited: 0xb8)
struct USparseDelegateFunction : UDelegateFunction {
	char UnknownData_B8[0x10]; // 0xb8(0x10)
};

// Class CoreUObject.PackageMap
// Size: 0x50 (Inherited: 0x28)
struct UPackageMap : UObject {
	char UnknownData_28[0x28]; // 0x28(0x28)
};

// Class CoreUObject.Enum
// Size: 0x58 (Inherited: 0x30)
struct UEnum : UField {
	char UnknownData_30[0x28]; // 0x30(0x28)
};

// Class CoreUObject.LinkerPlaceholderClass
// Size: 0x310 (Inherited: 0x1c0)
struct ULinkerPlaceholderClass : UClass {
	char UnknownData_1C0[0x150]; // 0x1c0(0x150)
};

// Class CoreUObject.LinkerPlaceholderExportObject
// Size: 0x88 (Inherited: 0x28)
struct ULinkerPlaceholderExportObject : UObject {
	char UnknownData_28[0x60]; // 0x28(0x60)
};

// Class CoreUObject.LinkerPlaceholderFunction
// Size: 0x208 (Inherited: 0xb8)
struct ULinkerPlaceholderFunction : UFunction {
	char UnknownData_B8[0x150]; // 0xb8(0x150)
};

// Class CoreUObject.MetaData
// Size: 0x78 (Inherited: 0x28)
struct UMetaData : UObject {
	char UnknownData_28[0x50]; // 0x28(0x50)
};

// Class CoreUObject.ObjectRedirector
// Size: 0x30 (Inherited: 0x28)
struct UObjectRedirector : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class CoreUObject.Property
// Size: 0x70 (Inherited: 0x30)
struct UProperty : UField {
	char UnknownData_30[0x40]; // 0x30(0x40)
};

// Class CoreUObject.NumericProperty
// Size: 0x70 (Inherited: 0x70)
struct UNumericProperty : UProperty {
};

// Class CoreUObject.ArrayProperty
// Size: 0x78 (Inherited: 0x70)
struct UArrayProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.ObjectPropertyBase
// Size: 0x78 (Inherited: 0x70)
struct UObjectPropertyBase : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.AssetObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct UAssetObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.AssetClassProperty
// Size: 0x80 (Inherited: 0x78)
struct UAssetClassProperty : UAssetObjectProperty {
	char UnknownData_78[0x8]; // 0x78(0x08)
};

// Class CoreUObject.BoolProperty
// Size: 0x78 (Inherited: 0x70)
struct UBoolProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.ByteProperty
// Size: 0x78 (Inherited: 0x70)
struct UByteProperty : UNumericProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.ObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct UObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.ClassProperty
// Size: 0x80 (Inherited: 0x78)
struct UClassProperty : UObjectProperty {
	char UnknownData_78[0x8]; // 0x78(0x08)
};

// Class CoreUObject.DelegateProperty
// Size: 0x78 (Inherited: 0x70)
struct UDelegateProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.DoubleProperty
// Size: 0x70 (Inherited: 0x70)
struct UDoubleProperty : UNumericProperty {
};

// Class CoreUObject.FloatProperty
// Size: 0x70 (Inherited: 0x70)
struct UFloatProperty : UNumericProperty {
};

// Class CoreUObject.IntProperty
// Size: 0x70 (Inherited: 0x70)
struct UIntProperty : UNumericProperty {
};

// Class CoreUObject.Int16Property
// Size: 0x70 (Inherited: 0x70)
struct UInt16Property : UNumericProperty {
};

// Class CoreUObject.Int64Property
// Size: 0x70 (Inherited: 0x70)
struct UInt64Property : UNumericProperty {
};

// Class CoreUObject.Int8Property
// Size: 0x70 (Inherited: 0x70)
struct UInt8Property : UNumericProperty {
};

// Class CoreUObject.InterfaceProperty
// Size: 0x78 (Inherited: 0x70)
struct UInterfaceProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.LazyObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct ULazyObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.MapProperty
// Size: 0xa8 (Inherited: 0x70)
struct UMapProperty : UProperty {
	char UnknownData_70[0x38]; // 0x70(0x38)
};

// Class CoreUObject.MulticastDelegateProperty
// Size: 0x78 (Inherited: 0x70)
struct UMulticastDelegateProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.MulticastInlineDelegateProperty
// Size: 0x78 (Inherited: 0x78)
struct UMulticastInlineDelegateProperty : UMulticastDelegateProperty {
};

// Class CoreUObject.NameProperty
// Size: 0x70 (Inherited: 0x70)
struct UNameProperty : UProperty {
};

// Class CoreUObject.StrProperty
// Size: 0x70 (Inherited: 0x70)
struct UStrProperty : UProperty {
};

// Class CoreUObject.StructProperty
// Size: 0x78 (Inherited: 0x70)
struct UStructProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.UInt16Property
// Size: 0x70 (Inherited: 0x70)
struct UUInt16Property : UNumericProperty {
};

// Class CoreUObject.UInt32Property
// Size: 0x70 (Inherited: 0x70)
struct UUInt32Property : UNumericProperty {
};

// Class CoreUObject.UInt64Property
// Size: 0x70 (Inherited: 0x70)
struct UUInt64Property : UNumericProperty {
};

// Class CoreUObject.WeakObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct UWeakObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.TextProperty
// Size: 0x70 (Inherited: 0x70)
struct UTextProperty : UProperty {
};

struct UCrewFunctions {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Athena.CrewFunctions");
	}
	static bool AreCharactersInSameCrew(ACharacter* Player1, ACharacter* Player2) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.CrewFunctions.AreCharactersInSameCrew");
		struct
		{
			ACharacter* Player1;
			ACharacter* Player2;
			bool ReturnValue;
		} params;
		params.Player1 = Player1;
		params.Player2 = Player2;
		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}
};

struct FAggressiveGhostShipState {
	bool IsShipVisible; // 0x00(0x01)
	bool IsShipDead; // 0x01(0x01)
	char UnknownData_2[0x2]; // 0x02(0x02)
	float ShipSpeed; // 0x04(0x04)
};

// Class Athena.AggressiveGhostShip
// Size: 0x880 (Inherited: 0x3d0)
struct AAggressiveGhostShip : AActor {
	char UnknownData_3D0[0x158]; // 0x3d0(0x40)
	struct FAggressiveGhostShipState ShipState; // 0x528(0x08)
	char UnknownData_530[0x14]; // 0x530(0x14)
	int32_t NumShotsLeftToKill; // 0x544(0x04)
	char UnknownData_548[0x338]; // 0x548(0x338)
};

enum class EDrawDebugTrace : uint8_t
{
	EDrawDebugTrace__None = 0,
	EDrawDebugTrace__ForOneFrame = 1,
	EDrawDebugTrace__ForDuration = 2,
	EDrawDebugTrace__Persistent = 3,
	EDrawDebugTrace__EDrawDebugTrace_MAX = 4
};


enum class ETraceTypeQuery : uint8_t
{
	TraceTypeQuery1 = 0,
	TraceTypeQuery2 = 1,
	TraceTypeQuery3 = 2,
	TraceTypeQuery4 = 3,
	TraceTypeQuery5 = 4,
	TraceTypeQuery6 = 5,
	TraceTypeQuery7 = 6,
	TraceTypeQuery8 = 7,
	TraceTypeQuery9 = 8,
	TraceTypeQuery10 = 9,
	TraceTypeQuery11 = 10,
	TraceTypeQuery12 = 11,
	TraceTypeQuery13 = 12,
	TraceTypeQuery14 = 13,
	TraceTypeQuery15 = 14,
	TraceTypeQuery16 = 15,
	TraceTypeQuery17 = 16,
	TraceTypeQuery18 = 17,
	TraceTypeQuery19 = 18,
	TraceTypeQuery20 = 19,
	TraceTypeQuery21 = 20,
	TraceTypeQuery22 = 21,
	TraceTypeQuery23 = 22,
	TraceTypeQuery24 = 23,
	TraceTypeQuery25 = 24,
	TraceTypeQuery26 = 25,
	TraceTypeQuery27 = 26,
	TraceTypeQuery28 = 27,
	TraceTypeQuery29 = 28,
	TraceTypeQuery30 = 29,
	TraceTypeQuery31 = 30,
	TraceTypeQuery32 = 31,
	TraceTypeQuery_MAX = 32,
	ETraceTypeQuery_MAX = 33
};


class UKismetSystemLibrary {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetSystemLibrary");
	}

	static FRotator DrawDebugCircle(struct UObject* WorldContextObject, struct FVector Center, float Radius, int32_t NumSegments, struct FLinearColor LineColor, float Duration, float Thickness, struct FVector YAxis, struct FVector ZAxis, bool bDrawAxis) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugCircle");

		struct {
			struct UObject* WorldContextObject;
			struct FVector Center;
			float Radius;
			int32_t NumSegments;
			struct FLinearColor LineColor;
			float Duration;
			float Thickness;
			struct FVector YAxis;
			struct FVector ZAxis;
			bool bDrawAxis;
		} params;
		params.WorldContextObject = WorldContextObject;
		params.Center = Center;
		params.Radius = Radius;
		params.NumSegments = NumSegments;
		params.LineColor = LineColor;
		params.Duration = Duration;
		params.Thickness = Thickness;
		params.YAxis = YAxis;
		params.ZAxis = ZAxis;
		params.bDrawAxis = bDrawAxis;

		ProcessEvent(defaultObj, fn, &params);
	}
};

class UKismetMathLibrary {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetMathLibrary");
	}
	static FRotator NormalizedDeltaRotator(const struct FRotator& A, const struct FRotator& B) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.NormalizedDeltaRotator");

		struct
		{
			struct FRotator                A;
			struct FRotator                B;
			struct FRotator                ReturnValue;
		} params;

		params.A = A;
		params.B = B;

		ProcessEvent(defaultObj, fn, &params);

		return params.ReturnValue;

	};
	static FRotator FindLookAtRotation(const FVector& Start, const FVector& Target) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.FindLookAtRotation");

		struct {
			FVector Start;
			FVector Target;
			FRotator ReturnValue;
		} params;
		params.Start = Start;
		params.Target = Target;

		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}

	static FVector Conv_RotatorToVector(const struct FRotator& InRot) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.Conv_RotatorToVector");

		struct
		{
			struct FRotator                InRot;
			struct FVector                 ReturnValue;
		} params;
		params.InRot = InRot;

		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}

	static bool LineTraceSingle_NEW(class UObject* WorldContextObject, const struct FVector& Start, const struct FVector& End, ETraceTypeQuery TraceChannel, bool bTraceComplex, TArray<class AActor*> ActorsToIgnore, EDrawDebugTrace DrawDebugType, bool bIgnoreSelf, struct FHitResult* OutHit)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.LineTraceSingle_NEW");

		struct
		{
			class UObject*                 WorldContextObject;
			struct FVector                 Start;
			struct FVector                 End;
			ETraceTypeQuery				   TraceChannel;
			bool                           bTraceComplex;
			TArray<class AActor*>          ActorsToIgnore;
			EDrawDebugTrace				   DrawDebugType;
			struct FHitResult              OutHit;
			bool                           bIgnoreSelf;
			bool                           ReturnValue;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.Start = Start;
		params.End = End;
		params.TraceChannel = TraceChannel;
		params.bTraceComplex = bTraceComplex;
		params.ActorsToIgnore = ActorsToIgnore;
		params.DrawDebugType = DrawDebugType;
		params.bIgnoreSelf = bIgnoreSelf;

		ProcessEvent(defaultObj, fn, &params);

		if (OutHit != nullptr)
			*OutHit = params.OutHit;

		return params.ReturnValue;
	}

	static void DrawDebugBox(UObject* WorldContextObject, const FVector& Center, const FVector& Extent, const FLinearColor& LineColor, const FRotator& Rotation, float Duration) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct
		{
			UObject* WorldContextObject = nullptr;
			FVector Center;
			FVector Extent;
			FLinearColor LineColor;
			FRotator Rotation;
			float Duration = INFINITY;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.Center = Center;
		params.Extent = Extent;
		params.LineColor = LineColor;
		params.Rotation = Rotation;
		params.Duration = Duration;
		ProcessEvent(defaultObj, fn, &params);
	}
	static void DrawDebugArrow(UObject* WorldContextObject, const FVector& LineStart, const FVector& LineEnd, float ArrowSize, const FLinearColor& LineColor, float Duration) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct
		{
			class UObject* WorldContextObject = nullptr;
			struct FVector LineStart;
			struct FVector LineEnd;
			float ArrowSize = 1.f;
			struct FLinearColor LineColor;
			float Duration = 1.f;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.LineStart = LineStart;
		params.LineEnd = LineEnd;
		params.ArrowSize = ArrowSize;
		params.LineColor = LineColor;
		params.Duration = Duration;

		ProcessEvent(defaultObj, fn, &params);
	}
};

enum class EMeleeWeaponMovementSpeed : uint8_t
{
	EMeleeWeaponMovementSpeed__Default = 0,
	EMeleeWeaponMovementSpeed__SlightlySlowed = 1,
	EMeleeWeaponMovementSpeed__Slowed = 2,
	EMeleeWeaponMovementSpeed__EMeleeWeaponMovementSpeed_MAX = 3
};

struct UMeleeAttackDataAsset
{
	char pad[0x0238];
	float                                              ClampYawRange;                                             // 0x0238(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              ClampYawRate;                                              // 0x023C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

struct UMeleeWeaponDataAsset
{
	char pad[0x0048];
	class UMeleeAttackDataAsset* HeavyAttack; //0x0048
	char pad2[0x0028];
	EMeleeWeaponMovementSpeed BlockingMovementSpeed; //0x0078
};

struct AMeleeWeapon
{
	char pad[0x07C0];
	struct UMeleeWeaponDataAsset* DataAsset; //0x07C0
};

// ScriptStruct Athena.ProjectileShotParams
// Size: 0x1c (Inherited: 0x00)
struct FProjectileShotParams {
	int Seed; // 0x00(0x04)
	float ProjectileDistributionMaxAngle; // 0x04(0x04)
	int NumberOfProjectiles; // 0x08(0x04)
	float ProjectileMaximumRange; // 0x0c(0x04)
	float ProjectileHitScanMaximumRange; // 0x10(0x04)
	float ProjectileDamage; // 0x14(0x04)
	float ProjectileDamageMultiplierAtMaximumRange; // 0x18(0x04)
};

// ScriptStruct Athena.WeaponProjectileParams
// Size: 0xc0 (Inherited: 0x00)
struct FWeaponProjectileParams {
	float Damage; // 0x00(0x04)
	float DamageMultiplierAtMaximumRange; // 0x04(0x04)
	float LifeTime; // 0x08(0x04)
	float TrailFadeOutTime; // 0x0c(0x04)
	float Velocity; // 0x10(0x04)
	float TimeBeforeApplyingGravity; // 0x14(0x04)
	float DownForceVelocityFractionPerSecond; // 0x18(0x04)
	float VelocityDampeningPerSecond; // 0x1c(0x04)
	struct FLinearColor Color; // 0x20(0x10)
	struct UClass* ProjectileId; // 0x30(0x08)
	char HealthChangeReason; // 0x38(0x01)UWorld
	char UnknownData_39[0x3]; // 0x39(0x03)
	char UnknownData_3C[0x68]; // 0x3C(0x68)
	int SuggestedMaxSimulationIterations; // 0xa4(0x04)
	float SuggestedMinTickTimeSecs; // 0xa8(0x04)
	float SuggestedMaxSimulationTimeStep; // 0xac(0x04)
	float HitScanTrailUpdateModifier; // 0xb0(0x04)
	float HitScanTrailFadeOutTime; // 0xb4(0x04)
	float HitScanTrailGrowthSpeed; // 0xb8(0x04)
	char UnknownData_BC[0x4]; // 0xbc(0x04)
};

// ScriptStruct Athena.ProjectileWeaponParameters
// Size: 0x1f8 (Inherited: 0x00)
struct FProjectileWeaponParameters {
	int AmmoClipSize; // 0x00(0x04)
	int AmmoCostPerShot; // 0x04(0x04)
	float EquipDuration; // 0x08(0x04)
	float IntoAimingDuration; // 0x0c(0x04)
	float RecoilDuration; // 0x10(0x04)
	float ReloadDuration; // 0x14(0x04)
	struct FProjectileShotParams HipFireProjectileShotParams; // 0x18(0x1c)
	struct FProjectileShotParams AimDownSightsProjectileShotParams; // 0x34(0x1c)
	int Seed; // 0x50(0x04)
	float ProjectileDistributionMaxAngle; // 0x54(0x04)
	int NumberOfProjectiles; // 0x58(0x04)
	float ProjectileMaximumRange; // 0x5c(0x04)
	float ProjectileHitScanMaximumRange; // 0x60(0x04)
	float ProjectileDamage; // 0x64(0x04)
	float ProjectileDamageMultiplierAtMaximumRange; // 0x68(0x04)
	char UnknownData_6C[0x4]; // 0x6c(0x04)
	struct UClass* DamagerType; // 0x70(0x08)
	struct UClass* ProjectileId; // 0x78(0x08)
	struct FWeaponProjectileParams AmmoParams; // 0x80(0xc0)
	bool UsesScope; // 0x140(0x01)
	char UnknownData_141[0x3]; // 0x141(0x03)
	float ZoomedRecoilDurationIncrease; // 0x144(0x04)
	float SecondsUntilZoomStarts; // 0x148(0x04)
	float SecondsUntilPostStarts; // 0x14c(0x04)
	float WeaponFiredAINoiseRange; // 0x150(0x04)
	float MaximumRequestPositionDelta; // 0x154(0x04)
	float MaximumRequestAngleDelta; // 0x158(0x04)
	float TimeoutTolerance; // 0x15c(0x04)
	float AimingMoveSpeedScalar; // 0x160(0x04)
	char AimSensitivitySettingCategory; // 0x164(0x01)
	char UnknownData_165[0x3]; // 0x165(0x03)
	float InAimFOV; // 0x168(0x04)
	float BlendSpeed; // 0x16c(0x04)
	struct UWwiseEvent* DryFireSfx; // 0x170(0x08)
	char UnknownData_178[0x10]; // 0x178(0x10)
	struct FName RumbleTag; // 0x188(0x08)
	bool KnockbackEnabled; // 0x190(0x01)
	char UnknownData_191[0x3]; // 0x191(0x03)
	char UnknownData_194[0x50]; // 0x194(0x50)
	bool StunEnabled; // 0x1e4(0x01)
	char UnknownData_1E5[0x3]; // 0x1e5(0x03)
	float StunDuration; // 0x1e8(0x04)
	struct FVector TargetingOffset; // 0x1ec(0x0c)
};

struct AProjectileWeapon {
	char pad[0x7d0]; // 0
	FProjectileWeaponParameters WeaponParameters; // 0x7d0

	bool CanFire()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ProjectileWeapon.CanFire");
		bool canfire;
		ProcessEvent(this, fn, &canfire);
		return canfire;
	}


};

struct AMapTable
{
	char pad[0x04F0];
	TArray<struct FVector2D> MapPins; // 0x04F0
};