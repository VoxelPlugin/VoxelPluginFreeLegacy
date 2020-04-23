// Copyright 2020 Phyronnaz

#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"
#include "VoxelRender/VoxelMaterialIndices.h"
#include "VoxelData/VoxelData.h"
#include "VoxelMessages.h"
#include "VoxelPriorityHandler.h"
#include "VoxelWorld.h"
#include "VoxelUniqueError.h"
#include "VoxelMaterialUtilities.h"

#include "Logging/MessageLog.h"

FVoxelRendererSettingsBase::FVoxelRendererSettingsBase(
	const AVoxelWorld* InWorld,
	EVoxelPlayType InPlayType,
	UPrimitiveComponent* RootComponent,	
	const FVoxelData* Data)
	: VoxelSize(InWorld->VoxelSize)
	, WorldOffset(Data
		? MakeVoxelShared<FIntVector>(0)
		: InWorld->GetWorldOffsetPtr())
	, ProcMeshClass(InPlayType == EVoxelPlayType::Game && InWorld->ProcMeshClass 
		? InWorld->ProcMeshClass.Get() 
		: UVoxelProceduralMeshComponent::StaticClass())
	, bCastFarShadow(InWorld->bCastFarShadow)

	, PlayType(InPlayType)

	, World(InWorld->GetWorld())
	, RootComponent(RootComponent)

	, UVConfig(InWorld->UVConfig)
	, UVScale(InWorld->UVScale)
	, NormalConfig(InWorld->NormalConfig)
	, MaterialConfig(InWorld->MaterialConfig)

	, BoundsExtension(InWorld->BoundsExtension)

	, CollisionTraceFlag(InWorld->CollisionTraceFlag)
	, NumConvexHullsPerAxis(InWorld->NumConvexHullsPerAxis)
	, bCleanCollisionMeshes(InWorld->bCleanCollisionMeshes)

	, RenderType(InWorld->RenderType)
	, bCreateMaterialInstances(InPlayType == EVoxelPlayType::Game
		? InWorld->bCreateMaterialInstances && !InWorld->bMergeChunks
		: false /* we don't want to created dynamic material instances in editor */)
	, bDitherChunks(InWorld->bDitherChunks && bCreateMaterialInstances)
	, ChunksDitheringDuration(InWorld->ChunksDitheringDuration)

	, bOptimizeIndices(InWorld->bOptimizeIndices)
	, MaxDistanceFieldLOD(InWorld->bGenerateDistanceFields ? InWorld->MaxDistanceFieldLOD : -1)
	, bOneMaterialPerCubeSide(InWorld->MaterialConfig == EVoxelMaterialConfig::SingleIndex && InWorld->bOneMaterialPerCubeSide)
	, bHalfPrecisionCoordinates(InWorld->bHalfPrecisionCoordinates)
	, bInterpolateColors(InWorld->bInterpolateColors)
	, bInterpolateUVs(InWorld->bInterpolateUVs)

	, bRenderWorld(InWorld->bRenderWorld)

	, MeshUpdatesBudget(InPlayType == EVoxelPlayType::Game
		? FMath::Max(0.001f, InWorld->MeshUpdatesBudget)
		: 1000)

	, HolesMaterials(InWorld->HolesMaterials)
	, MaterialsMeshConfigs(InWorld->MaterialsMeshConfigs)

	, bMergeChunks(InWorld->bMergeChunks)
	, ChunksClustersSize(FMath::Max(RENDER_CHUNK_SIZE, InWorld->ChunksClustersSize))
	, bDoNotMergeCollisionsAndNavmesh(InWorld->bMergeChunks && InWorld->bDoNotMergeCollisionsAndNavmesh)

	, bStaticWorld(InPlayType == EVoxelPlayType::Game
		? InWorld->bStaticWorld
		: false)

	, PriorityDuration(InWorld->PriorityDuration)
	, DynamicSettings(InWorld->GetRendererDynamicSettings())
{
}

FVoxelRendererSettings::FVoxelRendererSettings(
	const AVoxelWorld* World,
	EVoxelPlayType PlayType,
	UPrimitiveComponent* RootComponent,
	const TVoxelSharedRef<const FVoxelData>& Data,
	const TVoxelSharedRef<IVoxelPool>& Pool,
	const TVoxelSharedPtr<FVoxelToolRenderingManager>& ToolRenderingManager,
	const TVoxelSharedRef<FVoxelDebugManager>& DebugManager,
	bool bUseDataSettings)
	: FVoxelRendererSettingsBase(World, PlayType, RootComponent, bUseDataSettings ? &Data.Get() : nullptr)
	, Data(Data)
	, Pool(Pool)
	, ToolRenderingManager(ToolRenderingManager)
	, DebugManager(DebugManager)
{

}

IVoxelRenderer::IVoxelRenderer(const FVoxelRendererSettings& Settings)
	: Settings(Settings)
	, InvokersPositionsForPriorities(MakeVoxelShared<FInvokerPositionsArray>(32))
{
}

void IVoxelRenderer::SetInvokersPositionsForPriorities(const TArray<FIntVector>& NewInvokersPositionsForPriorities)
{
	while (InvokersPositionsForPriorities->GetMax() < NewInvokersPositionsForPriorities.Num())
	{
		InvokersPositionsForPriorities = MakeVoxelShared<FInvokerPositionsArray>(2 * InvokersPositionsForPriorities->GetMax());
	}
	InvokersPositionsForPriorities->Set(NewInvokersPositionsForPriorities);
}

inline UObject* GetRootOwner(const TWeakObjectPtr<UPrimitiveComponent>& RootComponent)
{
	return RootComponent.IsValid() ? RootComponent->GetOwner() : nullptr;
}

UMaterialInterface* FVoxelRendererSettingsBase::GetVoxelMaterial(const FVoxelMaterialIndices& MaterialIndices, bool bTessellation) const
{
	const auto MaterialCollection = DynamicSettings->MaterialCollection;
	if (!MaterialCollection.IsValid())
	{
		static TVoxelUniqueError<> UniqueError;
		FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
			UniqueError(UniqueId, {}),
			"Invalid Material Collection",
			GetRootOwner(RootComponent));
		return FVoxelUtilities::GetDefaultMaterial(bTessellation, MaterialIndices.NumIndices);
	}
	
	UMaterialInterface* MaterialInterface = MaterialCollection->GetVoxelMaterial(MaterialIndices, bTessellation, UniqueId);
	if (MaterialInterface && FVoxelUtilities::IsMaterialTessellated(MaterialInterface) == bTessellation)
	{
		return MaterialInterface;
	}
	else
	{
		return FVoxelUtilities::GetDefaultMaterial(bTessellation, MaterialIndices.NumIndices);
	}
}

UMaterialInterface* FVoxelRendererSettingsBase::GetVoxelMaterial(bool bTessellation) const
{
	const auto Material = bTessellation ? DynamicSettings->VoxelMaterialWithTessellation : DynamicSettings->VoxelMaterialWithoutTessellation;
	if (Material.IsValid() && FVoxelUtilities::IsMaterialTessellated(Material.Get()) == bTessellation)
	{
		return Material.Get();
	}

	if (bTessellation)
	{
		if (DynamicSettings->VoxelMaterialWithTessellation.IsValid())
		{
			static TVoxelUniqueError<> UniqueError;
			FVoxelMessages::CondError(
				UniqueError(UniqueId, {}),
				"TessellatedVoxelMaterial must have tessellation enabled! If you don't need tessellation, set EnableTessellation to false and use VoxelMaterial",
				GetRootOwner(RootComponent));
		}
		else
		{
			static TVoxelUniqueError<> UniqueError;
			FVoxelMessages::CondError(
				UniqueError(UniqueId, {}),
				"Invalid TessellatedVoxelMaterial! EnableTessellation = true requires TessellatedVoxelMaterial to be set",
				GetRootOwner(RootComponent));
		}
	}
	else
	{
		if (DynamicSettings->VoxelMaterialWithoutTessellation.IsValid())
		{
			static TVoxelUniqueError<> UniqueError;
			FVoxelMessages::CondError(
				UniqueError(UniqueId, {}),
				"VoxelMaterial must have tessellation disabled! If you need tessellation, set EnableTessellation to true and use TessellatedVoxelMaterial",
				GetRootOwner(RootComponent));
		}
		else
		{
			static TVoxelUniqueError<> UniqueError;
			FVoxelMessages::CondError(
				UniqueError(UniqueId, {}),
				"Invalid VoxelMaterial",
				GetRootOwner(RootComponent));
		}
	}
	
	return FVoxelUtilities::GetDefaultMaterial(bTessellation, 0);
}

uint64 FVoxelRendererSettingsBase::UniqueIdCounter = 0;