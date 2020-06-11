// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelInstancedMeshSettings.h"
#include "VoxelSpawners/VoxelHierarchicalInstancedStaticMeshComponent.h"
#include "VoxelSpawners/VoxelSpawnerActor.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"

FVoxelInstancedMeshSettings::FVoxelInstancedMeshSettings()
{
	BodyInstance.SetCollisionProfileName("BlockAll");
}

FVoxelSpawnerActorSettings::FVoxelSpawnerActorSettings()
{
	ActorClass = AVoxelMeshSpawnerActor::StaticClass();
	BodyInstance.SetCollisionProfileName("BlockAll");
}

FVoxelInstancedMeshAndActorSettings::FVoxelInstancedMeshAndActorSettings(
	TWeakObjectPtr<UStaticMesh> Mesh,
	const TMap<int32, UMaterialInterface*>& SectionMaterials,
	FVoxelInstancedMeshSettings MeshSettings,
	FVoxelSpawnerActorSettings ActorSettings)
	: Mesh(Mesh)
	, MeshSettings(MeshSettings)
	, ActorSettings(ActorSettings)
{
	SetSectionsMaterials(SectionMaterials);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TMap<int32, UMaterialInterface*> FVoxelInstancedMeshAndActorSettings::GetSectionsMaterials() const
{
	TMap<int32, UMaterialInterface*> SectionsMaterials;
	for (int32 Index = 0; Index < MaterialsOverrides.Num(); Index++)
	{
		const TWeakObjectPtr<UMaterialInterface> Material = MaterialsOverrides[Index];
		if (Material.IsValid())
		{
			SectionsMaterials.Add(Index, Material.Get());
		}
	}
	return SectionsMaterials;
}

void FVoxelInstancedMeshAndActorSettings::SetSectionsMaterials(const TMap<int32, UMaterialInterface*>& SectionMaterials)
{
	MaterialsOverrides.Reset();
	for (auto& It : SectionMaterials)
	{
		if (It.Key >= MaterialsOverrides.Num())
		{
			MaterialsOverrides.SetNum(It.Key + 1);
		}
		MaterialsOverrides[It.Key] = It.Value;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline bool operator==(FVoxelInt32Interval A, FVoxelInt32Interval B)
{
	return A.Min == B.Min && A.Max == B.Max;
}
inline uint32 GetTypeHash(FVoxelInt32Interval A)
{
	return HashCombine(GetTypeHash(A.Min), GetTypeHash(A.Max));
}

inline bool operator==(FLightingChannels A, FLightingChannels B)
{
	return
		A.bChannel0 == B.bChannel0 &&
		A.bChannel1 == B.bChannel1 &&
		A.bChannel2 == B.bChannel2;
}
inline uint32 GetTypeHash(FLightingChannels A)
{
	return A.bChannel0 + 2 * A.bChannel1 + 4 * A.bChannel2;
}

class FFoliageTypeCustomizationHelpers
{
public:
	inline static bool Equal(const FBodyInstance& A, const FBodyInstance& B)
	{
		return A.ObjectType == B.ObjectType && A.CollisionResponses == B.CollisionResponses;
	}
	inline static uint32 GetTypeHashBody(const FBodyInstance& A)
	{
		// Ignore collision responses
		return uint32(A.ObjectType);
	}
};

inline bool operator==(const FBodyInstance& A, const FBodyInstance& B)
{
	return FFoliageTypeCustomizationHelpers::Equal(A, B);
}
inline uint32 GetTypeHash(const FBodyInstance& A)
{
	return FFoliageTypeCustomizationHelpers::GetTypeHashBody(A);
}

inline uint32 GetTypeHash(TArray<TWeakObjectPtr<UMaterialInterface>> Materials)
{
	uint32 Hash = Materials.Num();
	for (auto& Material : Materials)
	{
		Hash = HashCombine(Hash, GetTypeHash(Material));
	}
	return Hash;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline bool operator==(const FVoxelInstancedMeshSettings& A, const FVoxelInstancedMeshSettings& B)
{
	return
		A.CullDistance                   == B.CullDistance                   &&
		A.bCastShadow                    == B.bCastShadow                    &&
		A.bAffectDynamicIndirectLighting == B.bAffectDynamicIndirectLighting &&
		A.bAffectDistanceFieldLighting   == B.bAffectDistanceFieldLighting   &&
		A.bCastShadowAsTwoSided          == B.bCastShadowAsTwoSided          &&
		A.bReceivesDecals                == B.bReceivesDecals                &&
		A.bUseAsOccluder                 == B.bUseAsOccluder                 &&
		A.BodyInstance                   == B.BodyInstance                   &&
		A.CustomNavigableGeometry        == B.CustomNavigableGeometry        &&
		A.LightingChannels               == B.LightingChannels               &&
		A.bRenderCustomDepth             == B.bRenderCustomDepth             &&
		A.CustomDepthStencilValue        == B.CustomDepthStencilValue        &&
		A.BuildDelay                     == B.BuildDelay                     &&
		A.HISMTemplate                   == B.HISMTemplate;
}

inline bool operator==(const FVoxelSpawnerActorSettings& A, const FVoxelSpawnerActorSettings& B)
{
	return
		A.ActorClass       == B.ActorClass       &&
		A.BodyInstance == B.BodyInstance &&
		A.Lifespan         == B.Lifespan;
}

bool operator==(const FVoxelInstancedMeshAndActorSettings& A, const FVoxelInstancedMeshAndActorSettings& B)
{
	return
		A.Mesh               == B.Mesh               &&
		A.MaterialsOverrides == B.MaterialsOverrides &&
		A.MeshSettings       == B.MeshSettings       &&
		A.ActorSettings      == B.ActorSettings;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define HASH(X) (GetTypeHash(Settings.X) * (FVoxelUtilities::MurmurHash32(__LINE__) & 0xFFFF)) ^

inline uint32 GetTypeHash(const FVoxelInstancedMeshSettings& Settings)
{
	return
		HASH(CullDistance)
		HASH(bCastShadow)
		HASH(bAffectDynamicIndirectLighting)
		HASH(bAffectDistanceFieldLighting)
		HASH(bCastShadowAsTwoSided)
		HASH(bReceivesDecals)
		HASH(bUseAsOccluder)
		HASH(BodyInstance)
		HASH(CustomNavigableGeometry)
		HASH(LightingChannels)
		HASH(bRenderCustomDepth)
		HASH(CustomDepthStencilValue)
		HASH(BuildDelay)
		HASH(HISMTemplate)
		0;
}

inline uint32 GetTypeHash(const FVoxelSpawnerActorSettings& Settings)
{
	return
		HASH(ActorClass)
		HASH(BodyInstance)
		HASH(Lifespan)
		0;
}

uint32 GetTypeHash(const FVoxelInstancedMeshAndActorSettings& Settings)
{
	return
		HASH(Mesh)
		HASH(MaterialsOverrides)
		HASH(MeshSettings)
		HASH(ActorSettings)
		0;
}
#undef HASH