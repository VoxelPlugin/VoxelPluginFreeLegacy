// Copyright 2018 Phyronnaz

#include "VoxelWorld.h"
#include "VoxelLogStatDefinitions.h"
#include "VoxelData/VoxelData.h"
#include "VoxelRender/IVoxelRender.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelUtilities.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "VoxelDebug/VoxelCrashReporter.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "EngineUtils.h"
#include "Logging/MessageLog.h"
#include "FastNoise.h"
#include "Engine/Engine.h"
#include "VoxelMaterialCollection.h"
#include "UObject/UObjectIterator.h"
#include "VoxelThreadPool.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "VoxelCacheManager.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "VoxelWorld"

DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::Tick"), STAT_VoxelWorld_Tick, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::ReceiveData"), STAT_VoxelWorld_ReceiveData, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::SendData"), STAT_VoxelWorld_SendData, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::GetIntersection"), STAT_VoxelWorld_GetIntersection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::CreateWorldInternal"), STAT_VoxelWorld_CreateWorldInternal, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::DestroyWorldInternal"), STAT_VoxelWorld_DestroyWorldInternal, STATGROUP_Voxel);

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Commands ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CmdHelperBool(Name) \
static void	Name(const TArray<FString>& Args, UWorld* World) \
{ \
	bool bShow = Args.Num() == 0 || (Args[0] == "1" || Args[0] == "true"); \
	 \
	for (TActorIterator<AVoxelWorld> It(World); It; ++It) \
	{ \
		(*It)->b##Name = bShow; \
	} \
}

CmdHelperBool(ShowInvokersPositions);
CmdHelperBool(ShowWorldBounds);
CmdHelperBool(ShowDataOctreeLeavesStatus);
CmdHelperBool(ShowUpdatedChunks);

FAutoConsoleCommandWithWorldAndArgs ShowInvokersPositionsCmd(
	TEXT("voxel.ShowInvokersPositions"),
	TEXT("Show Voxel Invokers Positions"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(ShowInvokersPositions));

FAutoConsoleCommandWithWorldAndArgs ShowWorldBoundsCmd(
	TEXT("voxel.ShowWorldBounds"),
	TEXT("Show Voxel Worlds Bounds"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(ShowWorldBounds));

FAutoConsoleCommandWithWorldAndArgs ShowDataOctreeLeavesStatusCmd(
	TEXT("voxel.ShowDataOctreeLeavesStatus"),
	TEXT("Show Data Octree Leaves Status. Useful to debug the cache status & to see which chunks are stored in memory"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(ShowDataOctreeLeavesStatus));

FAutoConsoleCommandWithWorldAndArgs ShowUpdatedChunksCmd(
	TEXT("voxel.ShowUpdatedChunks"),
	TEXT("Show Updated Chunks"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(ShowUpdatedChunks));

///////////////////////////////////////////////////////////////////////////////
///////////////////// Macros to generate getters/setters //////////////////////
///////////////////////////////////////////////////////////////////////////////

static TArray<FString> __NoCheckVariables;
struct FLocalAddToCheckVariables
{
	FLocalAddToCheckVariables(FString String)
	{
		__NoCheckVariables.Add(String);
	}
};
#define COMBINEIMPL(A, B) A##B
#define COMBINE(A, B) COMBINEIMPL(A, B)
#define ADD_TO_NOCHECKVARIABLES(Text)\
static FLocalAddToCheckVariables COMBINE(__NoCheckVar, __LINE__) = FLocalAddToCheckVariables(Text);

inline void __VoxelLogError(const FText& Error)
{
	FMessageLog("PIE").Error(Error);
}

#define CHECK_VOXELWORLD_IS_CREATED(Name, ...) if(!IsCreated()) { __VoxelLogError(FText::FromString(FString(#Name) + TEXT(": World isn't created!"))); return __VA_ARGS__; }
#define CHECK_VOXELWORLD_ISNT_CREATED(Name, ...) if(IsCreated()) { __VoxelLogError(FText::FromString(FString(#Name) + TEXT(": World is created!"))); return __VA_ARGS__; }

#define GETTER_SETTER_IMPL(Name, Prefix, CheckFunc, Accessor) \
void AVoxelWorld::Set##Name(decltype(AVoxelWorld::Prefix##Name) New) \
{ \
	CheckFunc(Set##Name); \
	Prefix##Name = Accessor; \
} \
decltype(AVoxelWorld::Prefix##Name) AVoxelWorld::Get##Name() const \
{ \
	return Prefix##Name; \
}

#define IGNOREMACRO(...)
#define GETTER_SETTER_NOCHECK(Name)              GETTER_SETTER_IMPL(Name,,  IGNOREMACRO, New); ADD_TO_NOCHECKVARIABLES(#Name);
#define GETTER_SETTER_NOCHECK_BOOL(Name)         GETTER_SETTER_IMPL(Name,b, IGNOREMACRO, New); ADD_TO_NOCHECKVARIABLES("b"#Name);
#define GETTER_SETTER_CHECKNOTCREATED(Name)      GETTER_SETTER_IMPL(Name,,  CHECK_VOXELWORLD_ISNT_CREATED, New)
#define GETTER_SETTER_CHECKNOTCREATED_BOOL(Name) GETTER_SETTER_IMPL(Name,b, CHECK_VOXELWORLD_ISNT_CREATED, New)

#define GETTER_SETTER_NOCHECK_ACCESSOR(Name, Accessor)         GETTER_SETTER_IMPL(Name,, IGNOREMACRO, Accessor); ADD_TO_NOCHECKVARIABLES(#Name);
#define GETTER_SETTER_CHECKNOTCREATED_ACCESSOR(Name, Accessor) GETTER_SETTER_IMPL(Name,, CHECK_VOXELWORLD_ISNT_CREATED, Accessor)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline bool IsDedicatedServerInternal(UWorld* World)
{
	auto* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}
	auto* WorldContext = GameInstance->GetWorldContext();
	if (!WorldContext)
	{
		return false;
	}
	return WorldContext->RunAsDedicated;
}

inline void FixWorldBoundsMinMax(int& Min, int& Max)
{
	if (Min >= Max)
	{
		if (Max > 0)
		{
			Min = Max - CHUNK_SIZE;
		}
		else
		{
			Max = Min + CHUNK_SIZE;
		}
	}
}

inline void FixWorldBounds(FIntBox& WorldBounds, int Depth)
{
	FixWorldBoundsMinMax(WorldBounds.Min.X, WorldBounds.Max.X);
	FixWorldBoundsMinMax(WorldBounds.Min.Y, WorldBounds.Max.Y);
	FixWorldBoundsMinMax(WorldBounds.Min.Z, WorldBounds.Max.Z);
	if (!WorldBounds.IsMultipleOf(CHUNK_SIZE))
	{
		WorldBounds.MakeMultipleOfInclusive(CHUNK_SIZE);
	}
	WorldBounds = FVoxelUtilities::GetBoundsFromDepth<CHUNK_SIZE>(Depth).Overlap(WorldBounds);
	check(WorldBounds.IsMultipleOf(CHUNK_SIZE));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelWorld::AVoxelWorld()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bHighPriority = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

#if WITH_EDITORONLY_DATA
	{
		auto SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));

		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTextureObject;
			FName ID_Voxel;
			FText NAME_Voxel;
			FConstructorStatics()
				: SpriteTextureObject(TEXT("/Engine/EditorResources/S_Terrain"))
				, ID_Voxel(TEXT("VoxelWorld"))
				, NAME_Voxel(NSLOCTEXT("VoxelCategory", "Voxel World", "Voxel World"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		if (SpriteComponent)
		{
			SpriteComponent->Sprite = ConstructorStatics.SpriteTextureObject.Get();
			SpriteComponent->RelativeScale3D = FVector(0.5f, 0.5f, 0.5f);
			SpriteComponent->bHiddenInGame = true;
			SpriteComponent->bIsScreenSizeScaled = true;
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Voxel;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Voxel;
			SpriteComponent->SetupAttachment(RootComponent);
			SpriteComponent->bReceivesDecals = false;
		}
	}
#endif // WITH_EDITORONLY_DATA
	{
		ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> MaterialFinder(TEXT("/Voxel/M_MissingMaterial"));
		MissingMaterial = MaterialFinder.Get();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::CreateWorld()
{
	if (!IsCreated())
	{
		CreateWorldInternal(
			EPlayType::Normal,
			false,
			bEnableUndoRedo,
			bDontUseGlobalPool);
	}
	else
	{
		FMessageLog("PIE").Error(LOCTEXT("CreateWorld", "Can't create world: already created"));
	}
}

void AVoxelWorld::DestroyWorld(bool bClearMeshes)
{
	if (IsCreated())
	{
		DestroyWorldInternal();
		auto Components = GetComponents(); // need a copy as we are modifying it when destroying comps
		for (auto& Component : Components)
		{
			auto Mesh = Cast<UVoxelProceduralMeshComponent>(Component);
			if (bClearMeshes && Mesh)
			{
				Mesh->ClearSections();
			}
			
			if (!Mesh)
			{
				auto HISM = Cast<UHierarchicalInstancedStaticMeshComponent>(Component);
				if (HISM)
				{
					HISM->DestroyComponent();
				}
			}
		}
	}
	else
	{
		FMessageLog("PIE").Error(LOCTEXT("DestroyWorld", "Can't destroy world: not created"));
	}
}

void AVoxelWorld::CreateInEditor(UClass* VoxelWorldEditorClass)
{
	check(VoxelWorldEditorClass);

	if (IsCreated())
	{
		DestroyWorldInternal();
	}

	if (!VoxelWorldEditor)
	{
		// Create/Find VoxelWorldEditor
		for (TActorIterator<AVoxelWorldEditorInterface> It(GetWorld()); It; ++It)
		{
			VoxelWorldEditor = *It;
			break;
		}
		if (!VoxelWorldEditor)
		{
			FActorSpawnParameters Transient;
			Transient.ObjectFlags = RF_Transient;
			VoxelWorldEditor = GetWorld()->SpawnActor<AVoxelWorldEditorInterface>(VoxelWorldEditorClass, Transient);
		}
	}
	VoxelWorldEditor->Init(this);

	CreateWorldInternal(
		EPlayType::Editor,
		false,
		true,
		true);
}

void AVoxelWorld::DestroyInEditor()
{
	if (IsCreated())
	{
		DestroyWorldInternal();
		auto Components = GetComponents(); // need a copy as we are modifying it when destroying comps
		for (auto& Component : Components)
		{
			if (auto Mesh = Cast<UVoxelProceduralMeshComponent>(Component))
			{
				Mesh->DestroyComponent();
			}
			if (auto Mesh = Cast<UHierarchicalInstancedStaticMeshComponent>(Component))
			{
				if (Mesh->HasAnyFlags(RF_Transient))
				{
					Mesh->DestroyComponent();
				}
			}
		}
	}
}

void AVoxelWorld::Recreate()
{
	check(IsCreated());

	Render.Reset();
	auto Components = GetComponents();
	for (auto& Component : Components)
	{
		auto Mesh = Cast<UVoxelProceduralMeshComponent>(Component);
		if (Mesh)
		{
			Mesh->DestroyComponent();
		}
	}
	Render = FVoxelRenderFactory::GetVoxelRender(RenderType, this);
	InvokerComponentChangeVersion = -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::UpdateCollisionProfile()
{
	CollisionPresets.LoadProfileData(false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UMaterialInterface* AVoxelWorld::GetVoxelMaterial(const FVoxelBlendedMaterial& Index) const
{
	auto* Value = MaterialCollection ? MaterialCollection->GetVoxelMaterial(Index) : nullptr;
	if (!Value)
	{
		if (!MaterialCollection)
		{
			FMessageLog("PIE").Error(LOCTEXT("InvalidMaterialCollection", "Invalid Material Collection"));
		}
		else
		{
			FMessageLog("PIE").Error(FText::FromString(FString::Printf(TEXT("Missing the following generated material in %s: %s"), *MaterialCollection->GetPathName(), *Index.ToString())));
		}
	}
	return Value ? Value : MissingMaterial;
}

UMaterialInterface* AVoxelWorld::GetVoxelMaterialWithTessellation(const FVoxelBlendedMaterial& Index) const
{
	auto* Value = MaterialCollection ? MaterialCollection->GetVoxelMaterialWithTessellation(Index) : nullptr;
	if (!Value)
	{
		if (!MaterialCollection)
		{
			FMessageLog("PIE").Error(LOCTEXT("InvalidMaterialCollection", "Invalid Material Collection"));
		}
		else
		{
			FMessageLog("PIE").Error(FText::FromString(FString::Printf(TEXT("Missing the following generated material in %s: Tessellation %s"), *MaterialCollection->GetPathName(), *Index.ToString())));
		}
	}
	return Value ? Value : MissingMaterial;
}

void AVoxelWorld::SetScalarParameterValue(FName ParameterName, float Value)
{
	CHECK_VOXELWORLD_IS_CREATED(SetScalarParameterValue);
	for (auto Prefix : { "", "INPUT0_", "INPUT1_", "INPUT2_" })
	{
		FName Name(*(Prefix + ParameterName.ToString()));
		MaterialInstance->SetScalarParameterValue(Name, Value);
		Render->SetScalarParameterValue(Name, Value);
	}
}

void AVoxelWorld::SetTextureParameterValue(FName ParameterName, UTexture* Value)
{
	CHECK_VOXELWORLD_IS_CREATED(SetTextureParameterValue);
	for (auto Prefix : { "", "INPUT0_", "INPUT1_", "INPUT2_" })
	{
		FName Name(*(Prefix + ParameterName.ToString()));
		MaterialInstance->SetTextureParameterValue(Name, Value);
		Render->SetTextureParameterValue(Name, Value);
	}
}

void AVoxelWorld::SetVectorParameterValue(FName ParameterName, FLinearColor Value)
{
	CHECK_VOXELWORLD_IS_CREATED(SetVectorParameterValue);
	for (auto Prefix : { "", "INPUT0_", "INPUT1_", "INPUT2_" })
	{
		FName Name(*(Prefix + ParameterName.ToString()));
		MaterialInstance->SetVectorParameterValue(Name, Value);
		Render->SetVectorParameterValue(Name, Value);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelWorldGeneratorInit AVoxelWorld::GetInitStruct() const
{
	return FVoxelWorldGeneratorInit(GetSeeds(), GetVoxelSize());
}

FVector AVoxelWorld::GetChunkRelativePosition(const FIntVector& Position) const
{
	return FVector(Position + WorldOffset) * GetVoxelSize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::SetOctreeDepth(int NewDepth)
{
	CHECK_VOXELWORLD_ISNT_CREATED(SetOctreeDepth);
	OctreeDepth = FMath::Clamp(NewDepth, 1, MAX_WORLD_DEPTH - 1);
	// Update world size
	WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<CHUNK_SIZE>(OctreeDepth);
}

int AVoxelWorld::GetOctreeDepth() const
{
	return OctreeDepth;
}

void AVoxelWorld::SetLODToMinDistance(const TMap<uint8, float>& NewLODToMinDistance)
{
	LODToMinDistance.Empty();
	for (auto& It : NewLODToMinDistance)
	{
		LODToMinDistance.Add(FString::FromInt(It.Key), It.Value);
	}
}

TMap<uint8, float> AVoxelWorld::GetLODToMinDistance() const
{
	TMap<uint8, float> Result;
	for (auto& It : LODToMinDistance)
	{
		Result.Add(TCString<TCHAR>::Atoi(*It.Key), It.Value);
	}
	return Result;
}

void AVoxelWorld::SetWorldGeneratorObject(UVoxelWorldGenerator* NewGenerator)
{
	if (!NewGenerator)
	{
		FMessageLog("PIE").Error(LOCTEXT("SetWorldGeneratorObjectNull", "SetWorldGeneratorObject: NewGenerator is NULL"));
		return;
	}

	CHECK_VOXELWORLD_ISNT_CREATED(SetWorldGeneratorObject);

	WorldGenerator.Type = EVoxelWorldGeneratorPickerType::Object;
	WorldGenerator.WorldGeneratorObject = NewGenerator;
}

void AVoxelWorld::SetWorldGeneratorClass(TSubclassOf<UVoxelWorldGenerator> NewGeneratorClass)
{
	if(!NewGeneratorClass)
	{
		FMessageLog("PIE").Error(LOCTEXT("SetWorldGeneratorClassNull", "SetWorldGeneratorClass: NewGeneratorClass is NULL"));
		return;
	}

	CHECK_VOXELWORLD_ISNT_CREATED(SetWorldGeneratorObject);

	WorldGenerator.Type = EVoxelWorldGeneratorPickerType::Class;
	WorldGenerator.WorldGeneratorClass = NewGeneratorClass;
}

void AVoxelWorld::AddSeeds(const TMap<FString, int>& NewSeeds)
{
	CHECK_VOXELWORLD_ISNT_CREATED(AddSeeds);
	Seeds.Append(NewSeeds);
}

void AVoxelWorld::AddSeed(FString Name, int Value)
{
	CHECK_VOXELWORLD_ISNT_CREATED(AddSeed);
	Seeds.Add(Name, Value);
}

void AVoxelWorld::ClearSeeds()
{
	CHECK_VOXELWORLD_ISNT_CREATED(ClearSeeds);
	Seeds.Reset();
}

TMap<FString, int> AVoxelWorld::GetSeeds() const
{
	return Seeds;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

GETTER_SETTER_CHECKNOTCREATED_ACCESSOR(VoxelSize, FMath::Max(New, 0.0001f));
GETTER_SETTER_NOCHECK(VoxelMassMultiplierInKg);
GETTER_SETTER_CHECKNOTCREATED(SaveObject);
GETTER_SETTER_CHECKNOTCREATED_BOOL(CreateWorldAutomatically);
GETTER_SETTER_CHECKNOTCREATED_BOOL(EnableUndoRedo);
GETTER_SETTER_NOCHECK_BOOL(EnableWorldRebasing);

GETTER_SETTER_CHECKNOTCREATED_BOOL(UseCustomWorldBounds);
GETTER_SETTER_CHECKNOTCREATED(CustomWorldBounds);

GETTER_SETTER_NOCHECK_ACCESSOR(LODLimit, FVoxelUtilities::ClampLOD(New));

GETTER_SETTER_CHECKNOTCREATED(UVConfig);
GETTER_SETTER_CHECKNOTCREATED(NormalConfig);
GETTER_SETTER_CHECKNOTCREATED(MaterialConfig);
GETTER_SETTER_CHECKNOTCREATED(VoxelMaterial);
GETTER_SETTER_CHECKNOTCREATED(VoxelMaterialWithTessellation);
GETTER_SETTER_CHECKNOTCREATED(MaterialCollection);
GETTER_SETTER_CHECKNOTCREATED_BOOL(EnableTessellation);
GETTER_SETTER_CHECKNOTCREATED(BoundsExtension);
GETTER_SETTER_CHECKNOTCREATED_ACCESSOR(TessellationMaxLOD, FVoxelUtilities::ClampLOD(New));

GETTER_SETTER_CHECKNOTCREATED(RenderType);
GETTER_SETTER_CHECKNOTCREATED_ACCESSOR(ChunksCullingLOD, FVoxelUtilities::ClampLOD(New));
GETTER_SETTER_NOCHECK_ACCESSOR(ChunksFadeDuration, FMath::Max(New, 0.f));
GETTER_SETTER_CHECKNOTCREATED_ACCESSOR(ProcMeshClass, New ? (UClass*)New : UVoxelProceduralMeshComponent::StaticClass());

	
GETTER_SETTER_CHECKNOTCREATED_ACCESSOR(MaxCollisionsLOD, FVoxelUtilities::ClampLOD(New));
GETTER_SETTER_CHECKNOTCREATED(CollisionPresets);
		
GETTER_SETTER_NOCHECK_ACCESSOR(LODUpdateRate, FMath::Max(New, 0.000001f));
GETTER_SETTER_CHECKNOTCREATED_BOOL(DontUseGlobalPool);
GETTER_SETTER_CHECKNOTCREATED_ACCESSOR(MeshThreadCount, FMath::Max(New, 1));
GETTER_SETTER_NOCHECK_BOOL(OptimizeIndices);
GETTER_SETTER_NOCHECK_BOOL(WaitForOtherChunksToAvoidHoles);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool AVoxelWorld::IsCreated() const
{
	return bIsCreated;
}

FIntBox AVoxelWorld::GetBounds() const
{
	CHECK_VOXELWORLD_IS_CREATED(GetBounds, FIntBox());
	return Data->GetBounds();
}

int32 AVoxelWorld::GetTaskCount() const
{
	return IsCreated() ? FMath::Max(Render->GetTaskCount(), 0) : 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntVector AVoxelWorld::GlobalToLocal(const FVector& Position) const
{
	FVector P = GetTransform().InverseTransformPosition(Position) / GetVoxelSize();

	FIntVector LocalPosition;

	switch (GetRenderType())
	{
	case EVoxelRenderType::Cubic:
	{
		LocalPosition = FIntVector(FMath::FloorToInt(P.X), FMath::FloorToInt(P.Y), FMath::FloorToInt(P.Z));
		break;
	}
	case EVoxelRenderType::MarchingCubes:
	default:
	{
		LocalPosition = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
		break;
	}
	}

	return LocalPosition - WorldOffset;
}

FVector AVoxelWorld::GlobalToLocalFloat(const FVector& Position) const
{
	return GetTransform().InverseTransformPosition(Position) / GetVoxelSize() - FVector(WorldOffset);
}

FVector AVoxelWorld::LocalToGlobal(const FIntVector& Position) const
{
	return GetTransform().TransformPosition(GetVoxelSize() * FVector(Position + WorldOffset));
}

FVector AVoxelWorld::LocalToGlobalFloat(const FVector& Position) const
{
	return GetTransform().TransformPosition(GetVoxelSize() * (Position + FVector(WorldOffset)));
}

TArray<FIntVector> AVoxelWorld::GetNeighboringPositions(const FVector& GlobalPosition) const
{
	FVector P = GlobalToLocalFloat(GlobalPosition);
	return TArray<FIntVector>({
		FIntVector(FMath::FloorToInt(P.X), FMath::FloorToInt(P.Y), FMath::FloorToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::FloorToInt(P.Y), FMath::FloorToInt(P.Z)),
		FIntVector(FMath::FloorToInt(P.X), FMath::CeilToInt(P.Y) , FMath::FloorToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::CeilToInt(P.Y) , FMath::FloorToInt(P.Z)),
		FIntVector(FMath::FloorToInt(P.X), FMath::FloorToInt(P.Y), FMath::CeilToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::FloorToInt(P.Y), FMath::CeilToInt(P.Z)),
		FIntVector(FMath::FloorToInt(P.X), FMath::CeilToInt(P.Y) , FMath::CeilToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::CeilToInt(P.Y) , FMath::CeilToInt(P.Z))
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::UpdateChunksAtPosition(const FIntVector& Position)
{
	CHECK_VOXELWORLD_IS_CREATED(UpdateChunksAtPosition);
	Render->UpdateBox(FIntBox(Position), false);
}

void AVoxelWorld::UpdateChunksOverlappingBox(const FIntBox& Box, bool bRemoveHoles)
{
	CHECK_VOXELWORLD_IS_CREATED(UpdateChunksOverlappingBox);
	Render->UpdateBox(Box, bRemoveHoles);
}

void AVoxelWorld::UpdateChunksOverlappingBox(const FIntBox& Box, TFunction<void()> CallbackWhenUpdated)
{
	Render->UpdateBox(Box, true, CallbackWhenUpdated);
}

void AVoxelWorld::UpdateAll()
{
	CHECK_VOXELWORLD_IS_CREATED(UpdateAll);
	Render->UpdateBox(FIntBox::Infinite, false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int AVoxelWorld::GetLODAt(const FIntVector& Position) const
{
	CHECK_VOXELWORLD_IS_CREATED(GetLODAt, 0);

	if (IsInWorld(Position))
	{
		return Render->GetLODAtPosition(Position);
	}
	else
	{
		FMessageLog("PIE").Error(FText::Format(LOCTEXT("GetLODAtNIW", "GetLODAt: Not in world: ({0}, {1}, {2})"), Position.X, Position.Y, Position.Z));
		return 0;
	}
}

bool AVoxelWorld::IsInWorld(const FIntVector& Position) const
{
	CHECK_VOXELWORLD_IS_CREATED(IsInWorld, false);
	return Data->IsInWorld(Position);
}

bool AVoxelWorld::IsInside(const FVector& Position) const
{
	CHECK_VOXELWORLD_IS_CREATED(IsInside, false);
	for (auto& P : GetNeighboringPositions(Position))
	{
		if (!FVoxelValue(GetValue(P)).IsEmpty())
		{
			return true;
		}
	}
	return false;
}

bool AVoxelWorld::GetIntersection(const FIntVector& Start, const FIntVector& End, FVector& OutGlobalPosition, FIntVector& OutVoxelPosition) const
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelWorld_GetIntersection);
	
	CHECK_VOXELWORLD_IS_CREATED(GetIntersection, false);

	FIntVector Diff = End - Start;
	if (Diff.X != 0)
	{
		if (Diff.Y != 0 || Diff.Z != 0)
		{
			FMessageLog("PIE").Error(LOCTEXT("GetIntersectionError", "GetIntersection: Start and end should have 2 common coordinates"));
			return false;
		}
	}
	else if (Diff.Y != 0)
	{
		if (Diff.X != 0 || Diff.Z != 0)
		{
			FMessageLog("PIE").Error(LOCTEXT("GetIntersectionError", "GetIntersection: Start and end should have 2 common coordinates"));
			return false;
		}
	}
	else if (Diff.Z != 0)
	{
		if (Diff.X != 0 || Diff.Y != 0)
		{
			FMessageLog("PIE").Error(LOCTEXT("GetIntersectionError", "GetIntersection: Start and end should have 2 common coordinates"));
			return false;
		}
	}

	FIntVector RealStart(FMath::Min(Start.X, End.X), FMath::Min(Start.Y, End.Y), FMath::Min(Start.Z, End.Z));
	FIntVector RealEnd(FMath::Max(Start.X, End.X) + 1, FMath::Max(Start.Y, End.Y) + 1, FMath::Max(Start.Z, End.Z) + 1);

	bool bFound = false;
	
	{
		FVoxelScopeGetLock Lock(Data, FIntBox(RealStart, RealEnd));

		FVoxelValue OldValue = Data->GetValue(RealStart.X, RealStart.Y, RealStart.Z, 0);
		FIntVector OldPosition = RealStart;
		for (int X = RealStart.X; X < RealEnd.X; X++)
		{
			for (int Y = RealStart.Y; Y < RealEnd.Y; Y++)
			{
				for (int Z = RealStart.Z; Z < RealEnd.Z; Z++)
				{
					if (UNLIKELY(!Data->IsInWorld(X, Y, Z)))
					{
						FMessageLog("PIE").Error(LOCTEXT("GetIntersectionOOB", "GetIntersection: Out of world!"));
						return false;
					}

					FVoxelValue Value = Data->GetValue(X, Y, Z, 0);
					FIntVector Position(X, Y, Z);

					if (!FVoxelUtilities::HaveSameSign(OldValue, Value))
					{
						bool bSuccess;
						const float t = OldValue.ThisDividedByThisMinusA(Value, bSuccess);
						check(bSuccess);

						FVector Q = t * FVector(Position) + (1 - t) * FVector(OldPosition);
						OutGlobalPosition = LocalToGlobalFloat(Q);
						OutVoxelPosition = Position;
						bFound = true;
					}

					OldValue = Value;
					OldPosition = Position;

					if (bFound)
					{
						break;
					}
				}
				if (bFound)
				{
					break;
				}
			}
			if (bFound)
			{
				break;
			}
		}
	}

	return bFound;
}

FVector AVoxelWorld::GetNormal(const FIntVector& Position) const
{	
	CHECK_VOXELWORLD_IS_CREATED(GetNormal, FVector::ZeroVector);

	FVoxelScopeGetLock Lock(Data, FIntBox(FIntVector(Position.X - 1, Position.Y - 1, Position.Z - 1), FIntVector(Position.X + 2, Position.Y + 2, Position.Z + 2)));
	FVector Gradient = Data->GetGradient(Position, 0);

	return Gradient;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float AVoxelWorld::GetValue(const FIntVector& Position) const
{
	CHECK_VOXELWORLD_IS_CREATED(GetValue, 0);

	if (IsInWorld(Position))
	{
		FVoxelValue Value;

		{
			FVoxelScopeGetLock Lock(Data, FIntBox(Position));
			Value = Data->GetValue(Position, 0);
		}

		return Value.ToFloat();
	}
	else
	{
		FMessageLog("PIE").Error(FText::Format(LOCTEXT("GetValueOOB", "Get value: Not in world: ({0}, {1}, {2})"), Position.X, Position.Y, Position.Z));
		return 0;
	}
}

void AVoxelWorld::SetValue(const FIntVector& Position, float Value, bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED(SetValue);

	if (IsInWorld(Position))
	{
		{
			FVoxelScopeSetLock Lock(Data, FIntBox(Position));
			Data->SetValue(Position, Value);
		}

		if (bUpdateRender)
		{
			UpdateChunksAtPosition(Position);
		}
	}
	else
	{
		FMessageLog("PIE").Error(FText::Format(LOCTEXT("SetValueOOB", "Set Value: Not in world: ({0}, {1}, {2})"), Position.X, Position.Y, Position.Z));
	}
}

FVoxelMaterial AVoxelWorld::GetMaterial(const FIntVector& Position) const
{
	CHECK_VOXELWORLD_IS_CREATED(GetMaterial, FVoxelMaterial());

	if (IsInWorld(Position))
	{
		FVoxelMaterial Material;
		
		{
			FVoxelScopeGetLock Lock(Data, FIntBox(Position));
			Material = Data->GetMaterial(Position, 0);
		}

		return Material;
	}
	else
	{
		FMessageLog("PIE").Error(FText::Format(LOCTEXT("GetMaterialOOB", "Get material: Not in world: ({0}, {1}, {2})"), Position.X, Position.Y, Position.Z));
		return FVoxelMaterial();
	}
}

void AVoxelWorld::SetMaterial(const FIntVector& Position, const FVoxelMaterial& Material, bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED(SetMaterial);

	if (IsInWorld(Position))
	{
		
		{
			FVoxelScopeSetLock Lock(Data, FIntBox(Position));
			Data->SetMaterial(Position, Material);
		}

		if (bUpdateRender)
		{
			UpdateChunksAtPosition(Position);
		}
	}
	else
	{
		FMessageLog("PIE").Error(FText::Format(LOCTEXT("SetMaterialOOB", "Set material: Not in world: ({0}, {1}, {2})"), Position.X, Position.Y, Position.Z));
	}
}

float AVoxelWorld::GetFloatOutput(FName Name, int X, int Y, int Z) const
{
	CHECK_VOXELWORLD_IS_CREATED(GetFloatOutput, 0);

	int Index = Data->WorldGenerator->GetOutputIndex(Name);
	if (Index < 0)
	{
		check(Index == -1);
		FMessageLog("PIE").Error(FText::FromString("GetFloatOutput: No output named " + Name.ToString() + " found!"));
		return 0;
	}
	return Data->WorldGenerator->GetFloatOutput(X, Y, Z, Index);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::GetSave(FVoxelUncompressedWorldSave& OutSave) const
{
	CHECK_VOXELWORLD_IS_CREATED(GetSave);
	Data->GetSave(OutSave);
}

void AVoxelWorld::GetCompressedSave(FVoxelCompressedWorldSave& OutSave) const
{
	CHECK_VOXELWORLD_IS_CREATED(GetCompressedSave);
	FVoxelUncompressedWorldSave Save;
	Data->GetSave(Save);
	UVoxelSaveUtilities::CompressVoxelSave(Save, OutSave);
}

void AVoxelWorld::LoadFromSave(const FVoxelUncompressedWorldSave& Save)
{
	CHECK_VOXELWORLD_IS_CREATED(LoadFromSave, );
	if (Save.GetDepth() == -1)
	{
		FMessageLog("PIE").Error(LOCTEXT("LoadFromSaveError", "LoadFromSave: Invalid save"));
	}
	else
	{
		if (Save.GetDepth() > OctreeDepth)
		{
			FMessageLog("PIE").Warning(LOCTEXT("LoadFromSaveWarning", "LoadFromSave: Save depth is bigger than world depth, possible loss of data"));
		}
		TArray<FIntBox> BoundsToUpdate;
		Data->LoadFromSave(Save, BoundsToUpdate);
		for (auto& Bounds : BoundsToUpdate)
		{
			UpdateChunksOverlappingBox(Bounds, false);
		}
	}
}

void AVoxelWorld::LoadFromCompressedSave(FVoxelCompressedWorldSave& Save)
{
	CHECK_VOXELWORLD_IS_CREATED(LoadFromCompressedSave);
	FVoxelUncompressedWorldSave UncompressedSave;
	UVoxelSaveUtilities::DecompressVoxelSave(Save, UncompressedSave);
	LoadFromSave(UncompressedSave);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::Undo()
{
	CHECK_VOXELWORLD_IS_CREATED(Undo);

	if (!bEnableUndoRedo)
	{
		FMessageLog("PIE").Error(LOCTEXT("UndobEnableUndoRedo", "Undo: Undo called but bEnableUndoRedo is false"));
	}
	else if (!GetData()->CheckIfCurrentFrameIsEmpty())
	{
		FMessageLog("PIE").Error(LOCTEXT("UndoSaveFrame", "Undo: Undo called before SaveFrame"));
	}
	else
	{
		TArray<FIntBox> BoundsToUpdate;
		GetData()->Undo(BoundsToUpdate);
		for (auto& Bounds : BoundsToUpdate)
		{
			UpdateChunksOverlappingBox(Bounds, false);
		}
	}
}

void AVoxelWorld::Redo()
{
	CHECK_VOXELWORLD_IS_CREATED(Redo);

	if (!bEnableUndoRedo)
	{
		FMessageLog("PIE").Error(LOCTEXT("RedobEnableUndoRedo", "Redo: Redo called but bEnableUndoRedo is false"));
	}
	else if (!GetData()->CheckIfCurrentFrameIsEmpty())
	{
		FMessageLog("PIE").Error(LOCTEXT("RedoSaveFrame", "Redo: Redo called before SaveFrame"));
	}
	else
	{
		TArray<FIntBox> BoundsToUpdate;
		GetData()->Redo(BoundsToUpdate);
		for (auto& Bounds : BoundsToUpdate)
		{
			UpdateChunksOverlappingBox(Bounds, false);
		}
	}
}

void AVoxelWorld::SaveFrame()
{
	CHECK_VOXELWORLD_IS_CREATED(SaveFrame);
	if (!bEnableUndoRedo)
	{
		FMessageLog("PIE").Error(LOCTEXT("SaveFrame", "SaveFrame: SaveFrame called but bEnableUndoRedo is false"));
	}
	else
	{
		GetData()->SaveFrame();
	}
}

void AVoxelWorld::ClearFrames()
{
	CHECK_VOXELWORLD_IS_CREATED(ClearFrames);
	if (!bEnableUndoRedo)
	{
		FMessageLog("PIE").Error(LOCTEXT("ClearFrames", "ClearFrames: ClearFrames called but bEnableUndoRedo is false"));
	}
	else
	{
		GetData()->ClearFrames();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::DrawDebugIntBox(const FIntBox& Box, float Lifetime, float Thickness, FLinearColor Color) const
{
	DrawDebugIntBox(Box, Lifetime, Thickness, Color.ToFColor(false));
}

void AVoxelWorld::DrawDebugIntBox(const FIntBox& Box, float Lifetime, float Thickness, FColor Color, float BorderOffset) const
{
	FVector Min = LocalToGlobal(Box.Min);
	FVector Max = LocalToGlobal(Box.Max);

	FBox DebugBox(Min + BorderOffset, Max - BorderOffset);

	DrawDebugBox(GetWorld(), DebugBox.GetCenter(), DebugBox.GetExtent(), Color, false, Lifetime, 0, Thickness);
}

void AVoxelWorld::DebugVoxelsInsideBox(FIntBox Box, FLinearColor Color, float Lifetime, float Thickness, bool bDebugDensities, FLinearColor TextColor) const
{
	CHECK_VOXELWORLD_IS_CREATED(DebugVoxelsInsideBox);
	for (int X = Box.Min.X; X < Box.Max.X; X++)
	{
		for (int Y = Box.Min.Y; Y < Box.Max.Y; Y++)
		{
			for (int Z = Box.Min.Z; Z < Box.Max.Z; Z++)
			{
				DrawDebugIntBox(FIntBox(X, Y, Z), Lifetime, Thickness, Color);

				if (bDebugDensities)
				{
					FVoxelScopeGetLock Lock(GetData(), FIntBox(X, Y, Z));
					FVoxelValue Value = GetData()->GetValue(X, Y, Z, 0);
					DrawDebugString(GetWorld(), LocalToGlobal(FIntVector(X, Y, Z)), Value.ToString(), nullptr, TextColor.ToFColor(false), Lifetime);
				}
			}
		}
	}
}

void AVoxelWorld::ClearCache(const TArray<FIntBox>& BoundsToKeepCached)
{
	CHECK_VOXELWORLD_IS_CREATED(ClearCache);
	CacheManager->ClearCache(BoundsToKeepCached);
}

void AVoxelWorld::Cache(const TArray<FIntBox>& BoundsToCache)
{
	CHECK_VOXELWORLD_IS_CREATED(Cache);
	CacheManager->Cache(BoundsToCache);
}

void AVoxelWorld::AddOffset(const FIntVector& OffsetInVoxels, bool bMoveActor)
{
	CHECK_VOXELWORLD_IS_CREATED(AddOffset);
	if (bMoveActor)
	{
		SetActorLocation(GetTransform().TransformPosition(GetVoxelSize() * FVector(OffsetInVoxels)));
	}
	WorldOffset -= OffsetInVoxels;
	Render->RecomputeMeshPositions();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	FVoxelCrashReporter::ResetIgnoreMessages();

	UpdateCollisionProfile();

	if (!IsCreated() && bCreateWorldAutomatically)
	{
		CreateWorld();
	}
}

void AVoxelWorld::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsCreated())
	{
		DestroyWorld(false);
	}
}

void AVoxelWorld::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelWorld_Tick);

	Super::Tick(DeltaTime);

	if (IsCreated())
	{
		const float Time = GetWorld()->GetTimeSeconds();

		// Debug
		{
			if (bShowWorldBounds)
			{
				FIntBox Bounds = Data->GetBounds();
				DrawDebugIntBox(Bounds, 2 * DeltaTime, WorldBoundsThickness, FLinearColor::Red);
			}
			if (bShowInvokersPositions)
			{
				for (auto& Invoker : Invokers)
				{
					if (Invoker.IsValid())
					{
						DrawDebugPoint(
							GetWorld(),
							LocalToGlobal(GlobalToLocal(Invoker->GetPosition())),
							InvokersPointSize, 
							Invoker->IsLocalInvoker() ? FColor::Red : FColor::Silver, 
							2 * DeltaTime);
					}
				}
			}
		}

		auto TaskCount = Render->GetTaskCount();
		if (TaskCount >= 0)
		{
			if (TaskCount > 0)
			{
				GEngine->AddOnScreenDebugMessage((uint64)((PTRINT)this), 0.1, FColor::White, FString::Printf(TEXT("Tasks remaining: %d"), TaskCount));
			}
			else
			{
				if (!bWorldLoadAlreadyFinished)
				{
					bWorldLoadAlreadyFinished = true;
					OnWorldLoaded.Broadcast();
				}
			}
		}

		if (CacheManager->IsCaching())
		{			
			GEngine->AddOnScreenDebugMessage((uint64)((PTRINT)this) + 1, 0.1, FColor::White, TEXT("Building cache"));
		}

		if (InvokerComponentChangeVersion < UVoxelInvokerComponent::GetChangeVersion())
		{
			InvokerComponentChangeVersion = UVoxelInvokerComponent::GetChangeVersion();
			Invokers = UVoxelInvokerComponent::GetInvokers(GetWorld());
			Invokers.RemoveAll([](auto Ptr) { return !Ptr.IsValid(); });
		}

		Render->Tick(DeltaTime);

	}
}

void AVoxelWorld::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// Temp fix
	if (IsTemplate())
	{
		CollisionPresets.FixupData(this);
	}
}

void AVoxelWorld::ApplyWorldOffset(const FVector& InOffset, bool bWorldShift)
{
	if (!IsCreated() || !bEnableWorldRebasing)
	{
		Super::ApplyWorldOffset(InOffset, bWorldShift);
	}
	else
	{
		const FVector RelativeOffset = InOffset / GetVoxelSize();
		const FIntVector IntegerOffset = FVoxelIntVector::RoundToInt(RelativeOffset);
		const FVector GlobalIntegerOffset = (FVector)IntegerOffset * GetVoxelSize();
		const FVector Diff = InOffset - GlobalIntegerOffset;

		Super::ApplyWorldOffset(Diff, bWorldShift);

		WorldOffset += IntegerOffset;

		Render->RecomputeMeshPositions();
	}
}

void AVoxelWorld::PostLoad()
{
	Super::PostLoad();

	if (LOD_DEPRECATED > 0)
	{
		OctreeDepth = LOD_DEPRECATED;
	}

	if (ChunkCullingLOD_DEPRECATED > 0)
	{
		ChunksCullingLOD = ChunkCullingLOD_DEPRECATED;
	}

	if (WorldBounds_DEPRECATED.Min != FIntVector::ZeroValue || WorldBounds_DEPRECATED.Max != FIntVector::ZeroValue)
	{
		CustomWorldBounds = WorldBounds_DEPRECATED;
	}

	if (bOverrideBounds_DEPRECATED)
	{
		bUseCustomWorldBounds = true;
	}

	if (!ProcMeshClass)
	{
		ProcMeshClass = UVoxelProceduralMeshComponent::StaticClass();
	}
}

#if WITH_EDITOR
bool AVoxelWorld::ShouldTickIfViewportsOnly() const
{
	return true;
}

bool AVoxelWorld::CanEditChange(const UProperty* InProperty) const
{
	if (!InProperty)
	{
		return Super::CanEditChange(InProperty);
	}

	const FString Name = InProperty->GetNameCPP();

	bool bCanEdit;

	if (Name == GET_MEMBER_NAME_STRING_CHECKED(AVoxelWorld, NormalConfig))
	{
		bCanEdit = RenderType == EVoxelRenderType::MarchingCubes;
	}
	else if (Name == GET_MEMBER_NAME_STRING_CHECKED(AVoxelWorld, VoxelMaterial))
	{
		bCanEdit = MaterialConfig == EVoxelMaterialConfig::RGB;
	}
	else if (Name == GET_MEMBER_NAME_STRING_CHECKED(AVoxelWorld, VoxelMaterialWithTessellation))
	{
		bCanEdit = MaterialConfig == EVoxelMaterialConfig::RGB;
	}
	else if (Name == GET_MEMBER_NAME_STRING_CHECKED(AVoxelWorld, MaterialCollection))
	{
		bCanEdit = MaterialConfig != EVoxelMaterialConfig::RGB;
	}
	else
	{
		bCanEdit = true;
	}

	checkf(!__NoCheckVariables.Contains(Name) || InProperty->HasMetaData("EditWhenCreated"), TEXT("Invalid voxel property access rights: %s"), *InProperty->GetNameCPP());

	if (IsCreated())
	{
		return bCanEdit 
			&& (InProperty->HasMetaData("EditWhenCreated") ||
				__NoCheckVariables.Contains(Name) ||
				Name.StartsWith(GET_MEMBER_NAME_STRING_CHECKED(AVoxelWorld, LODToMinDistance)))
			&& Super::CanEditChange(InProperty);
	}
	else
	{
		return bCanEdit && Super::CanEditChange(InProperty);
	}
}

void AVoxelWorld::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	OctreeDepth = FMath::Clamp(OctreeDepth, 1, MAX_WORLD_DEPTH - 1);

	if (!ProcMeshClass)
	{
		ProcMeshClass = UVoxelProceduralMeshComponent::StaticClass();
	}

	if (PropertyChangedEvent.MemberProperty)
	{
		auto Name = PropertyChangedEvent.MemberProperty->GetNameCPP();
		if (Name == GET_MEMBER_NAME_STRING_CHECKED(AVoxelWorld, OctreeDepth))
		{
			WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<CHUNK_SIZE>(OctreeDepth);
		}
		else if (Name == GET_MEMBER_NAME_STRING_CHECKED(AVoxelWorld, WorldSizeInVoxel))
		{
			OctreeDepth = FVoxelUtilities::GetDepthFromSize<CHUNK_SIZE>(WorldSizeInVoxel);
			WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<CHUNK_SIZE>(OctreeDepth);
		}
		else if (Name == GET_MEMBER_NAME_STRING_CHECKED(AVoxelWorld, LODToMinDistance))
		{
			if (!LODToMinDistance.Contains("0"))
			{
				LODToMinDistance.Add("0", 1000);
			}

			for (auto& It : LODToMinDistance)
			{
				if (!It.Key.IsEmpty())
				{
					It.Key = FString::FromInt(FVoxelUtilities::ClampLOD(TCString<TCHAR>::Atoi(*It.Key)));
				}
			}
			LODToMinDistance.KeySort([](const FString& A, const FString& B) { return TCString<TCHAR>::Atoi(*A) < TCString<TCHAR>::Atoi(*B); });

			float LastValue = 0;
			for (auto& It : LODToMinDistance)
			{
				It.Value = FMath::Max(LastValue, It.Value);
				LastValue = It.Value;
			}
		}
		if (IsCreated())
		{
			if (PropertyChangedEvent.MemberProperty->HasMetaData("Recreate"))
			{
				Recreate();
			}
			else if (PropertyChangedEvent.MemberProperty->HasMetaData("UpdateAll"))
			{
				UpdateAll();
			}
		}
	}

	FixWorldBounds(CustomWorldBounds, OctreeDepth);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::CreateWorldInternal(
	EPlayType InPlayType,
	bool bInMultiplayer,
	bool bInEnableUndoRedo,
	bool bInDontUseGlobalPool)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelWorld_CreateWorldInternal);

	check(!IsCreated());
	check(!Data.IsValid());
	check(!Render.IsValid());
	check(!Pool.IsValid());

	UE_LOG(LogVoxel, Warning, TEXT("Loading world"));
	
	// Create Pool
	if (!bInDontUseGlobalPool)
	{
		if (FVoxelPool::IsGlobalVoxelPoolCreated())
		{
			Pool = FVoxelPool::GetGlobalPool();
		}
		else
		{
			FMessageLog("PIE").Error(LOCTEXT("GlobalPoolNotCreated", "DontUseGlobalPool = false but global pool isn't created!"));
		}
	}
	if (!Pool.IsValid())
	{
		Pool = FVoxelPool::Create(MeshThreadCount);
	}
	
	// Create Data
	{
		auto InstancedWorldGenerator = WorldGenerator.GetWorldGenerator();
		InstancedWorldGenerator->Init(GetInitStruct());

		FIntBox Bounds = FVoxelUtilities::GetBoundsFromDepth<CHUNK_SIZE>(OctreeDepth);
		if (bUseCustomWorldBounds)
		{
			Bounds = CustomWorldBounds;
		}
		FixWorldBounds(Bounds, OctreeDepth);

		Data = MakeShared<FVoxelData, ESPMode::ThreadSafe>(
			OctreeDepth + DATA_OCTREE_DEPTH_DIFF,
			Bounds,
			InstancedWorldGenerator,
			bInMultiplayer,
			bInEnableUndoRedo);
	}

	// Create Render AFTER Data & Pool
	Render = FVoxelRenderFactory::GetVoxelRender(RenderType, this);

	// Create cache manager
	CacheManager = MakeShared<FVoxelCacheManager>(this);

	
	// Set variables
	PlayType = InPlayType;
	bIsCreated = true;
	InvokerComponentChangeVersion = -1;
	WorldOffset = FIntVector::ZeroValue;
	bWorldLoadAlreadyFinished = false;
	MaterialInstance = UMaterialInstanceDynamic::Create(MissingMaterial, this);
	bIsDedicatedServer = IsDedicatedServerInternal(GetWorld());

	// Load if possible
	if (SaveObject)
	{
		LoadFromCompressedSave(SaveObject->Save);
	}


	// Start timers
	auto& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(CacheDebugHandle, [this]
	{
		if (bShowDataOctreeLeavesStatus)
		{
			CacheManager->DebugCache(bHideEmptyDataOctreeLeaves, 0.5f, DataOctreeLeavesThickness);
		}
	}, 0.5f, true);
}

void AVoxelWorld::DestroyWorldInternal()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelWorld_DestroyWorldInternal);

	check(IsCreated());
	check(Render.IsValid());
	check(Data.IsValid());
	check(Pool.IsValid());

	UE_LOG(LogVoxel, Warning, TEXT("Unloading world"));
	
	CacheManager.Reset();
	Render.Reset();
	Data.Reset();
	Pool.Reset();


	bIsCreated = false;
	PlayType = EPlayType::Destroyed;

	// Cancel timers
	auto& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(CacheDebugHandle);
}

#undef LOCTEXT_NAMESPACE
