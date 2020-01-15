// Copyright 2020 Phyronnaz

#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelMaterialCollection.h"
#include "VoxelRender/VoxelBlendedMaterial.h"
#include "VoxelData/VoxelData.h"
#include "VoxelMessages.h"
#include "VoxelPriorityHandler.h"
#include "VoxelWorld.h"

#include "Logging/MessageLog.h"
#include "Materials/Material.h"

#define LOCTEXT_NAMESPACE "Voxel"

template<typename TValue = bool, typename TKey = uint64>
class TVoxelUniqueWarning
{
public:
	TVoxelUniqueWarning() = default;

	bool NeedToRaiseWarning(TKey Key, TValue Value)
	{
		auto& Set = Map.FindOrAdd(Key);
		if (!Set.Contains(Value))
		{
			Set.Add(Value);
			return true;
		}
		else
		{
			return false;
		}
	}
	bool operator()(TKey Key, TValue Value)
	{
		return NeedToRaiseWarning(Key, Value);
	}

private:
	TMap<TKey, TSet<TValue>> Map;
};

FVoxelRendererSettingsBase::FVoxelRendererSettingsBase(
	const AVoxelWorld* InWorld,
	EVoxelPlayType InPlayType,
	UPrimitiveComponent* RootComponent,	
	const FVoxelData* Data)
	: VoxelSize(InWorld->VoxelSize)
	, OctreeDepth(Data
		? FVoxelUtilities::GetChunkDepthFromDataDepth(Data->Depth)
		: FVoxelUtilities::ClampChunkDepth(InWorld->OctreeDepth))
	, WorldBounds(Data
		? Data->WorldBounds
		: InWorld->GetWorldBounds())
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

	, TessellationBoundsExtension(InWorld->TessellationBoundsExtension)

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
	, InvokersPositions(MakeVoxelShared<FInvokerPositionsArray>(32))
{
}

void IVoxelRenderer::SetInvokersPositions(const TArray<FIntVector>& NewInvokersPositions)
{
	while (InvokersPositions->GetMax() < NewInvokersPositions.Num())
	{
		InvokersPositions = MakeVoxelShared<FInvokerPositionsArray>(2 * InvokersPositions->GetMax());
	}
	InvokersPositions->Set(NewInvokersPositions);
}

UMaterialInterface* FVoxelRendererSettingsBase::GetVoxelMaterial(FVoxelBlendedMaterialUnsorted BlendedIndex, bool bTessellation) const
{
	const auto DefaultMaterial = []()
	{
		return UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);
	};
	
	const auto MaterialCollection = DynamicSettings->MaterialCollection;
	if (!MaterialCollection.IsValid())
	{
		static TVoxelUniqueWarning<> InvalidCollection;
		FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
			InvalidCollection(UniqueId, {}), 
			LOCTEXT("InvalidMaterialCollection", "Invalid Material Collection"), RootComponent->GetOwner());
		return DefaultMaterial();
	}
	UMaterialInterface* Value = MaterialCollection->GetVoxelMaterial(BlendedIndex, bTessellation);
	if (Value)
	{
		return Value;
	}
	else
	{
		if (auto* Collection = Cast<UVoxelMaterialCollection>(MaterialCollection))
		{
			if (bTessellation && !Collection->bEnableTessellation)
			{
				static TVoxelUniqueWarning<> MissingTessellation;
				FVoxelMessages::CondError<EVoxelShowNotification::Hide>(MissingTessellation(UniqueId, {}), LOCTEXT("Tessellation", "You need to tick EnableTessellation"), Collection);
				return DefaultMaterial();
			}

			for (uint8 Index : BlendedIndex.GetElements())
			{
				if (!Collection->IsValidIndex(Index))
				{
					static TVoxelUniqueWarning<uint8> InvalidIndex;
					FVoxelMessages::CondError<EVoxelShowNotification::Hide>(InvalidIndex(UniqueId, Index),
						FText::Format(
							LOCTEXT(
								"MaterialCollectionInvalidIndex",
								"Missing index {0}. You need to add it to your material collection, or make sure you don't paint/generate it."),
							FText::AsNumber(Index)),
						Collection);
					return DefaultMaterial();
				}
			}

			static TVoxelUniqueWarning<FVoxelBlendedMaterialUnsorted> InvalidBlendedIndex;
			FVoxelMessages::CondError<EVoxelShowNotification::Hide>(InvalidBlendedIndex(UniqueId, BlendedIndex),
				FText::Format(
					LOCTEXT(
						"MaterialCollectionMissingGenerated",
						"Missing the following generated material: {0} {1}. You need to open the asset and click the Generate {2} button."),
					FText::FromString(bTessellation ? "Tessellation" : ""),
					FText::FromString(BlendedIndex.ToString()),
					FText::FromString(BlendedIndex.KindToString())),
				Collection);
			return DefaultMaterial();
		}
		else
		{
			ensure(MaterialCollection->IsA<UVoxelBasicMaterialCollection>());
			if (BlendedIndex.Kind != FVoxelBlendedMaterialUnsorted::Single)
			{
				static TVoxelUniqueWarning<> MustBeSingle;
				FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
					MustBeSingle(UniqueId, {}),
					LOCTEXT(
						"VoxelBasicMaterialCollectionMustBeSingle",
						"Basic material collections only support Single Index material configs! Check your voxel world material config"),
					Collection);
				return DefaultMaterial();
			}
			if (bTessellation)
			{
				static TVoxelUniqueWarning<uint8> MissingIndex;
				FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
					MissingIndex(UniqueId, BlendedIndex.Index0),
					FText::Format(
						LOCTEXT(
							"MissingIndex",
							"Missing a tessellated material for index {0}"),
						FText::AsNumber(BlendedIndex.Index0)),
					Collection);
				return DefaultMaterial();
			}
			else
			{
				static TVoxelUniqueWarning<uint8> MissingIndex;
				FVoxelMessages::CondError<EVoxelShowNotification::Hide>(
					MissingIndex(UniqueId, BlendedIndex.Index0),
					FText::Format(
						LOCTEXT(
							"MissingIndex",
							"Missing a material for index {0}"),
						FText::AsNumber(BlendedIndex.Index0)),
					Collection);
				return DefaultMaterial();
			}
		}
	}
}

UMaterialInterface* FVoxelRendererSettingsBase::GetVoxelMaterial(bool bTessellation) const
{
	const auto& Material = bTessellation ? DynamicSettings->VoxelMaterialWithTessellation : DynamicSettings->VoxelMaterialWithoutTessellation;
	if (!Material.IsValid())
	{
		static TVoxelUniqueWarning<> InvalidVoxelMaterial;
		FVoxelMessages::CondError(
			InvalidVoxelMaterial(UniqueId, {}),
			bTessellation ?
			LOCTEXT("InvalidVoxelMaterialWithTessellation", "Invalid Tessellated Voxel Material") :
			LOCTEXT("InvalidVoxelMaterial", "Invalid Voxel Material"),
			RootComponent->GetOwner());
		return nullptr;
	}
	return Material.Get();
}

uint64 FVoxelRendererSettingsBase::UniqueIdCounter = 0;

#undef LOCTEXT_NAMESPACE