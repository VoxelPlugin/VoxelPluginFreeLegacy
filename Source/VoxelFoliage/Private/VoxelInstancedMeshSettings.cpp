// Copyright 2021 Phyronnaz

#include "VoxelInstancedMeshSettings.h"
#include "VoxelHierarchicalInstancedStaticMeshComponent.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"
#include "Engine/CollisionProfile.h"

FVoxelInstancedMeshSettings::FVoxelInstancedMeshSettings()
{
	BodyInstance.SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
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

inline uint32 GetTypeHash(const TArray<TWeakObjectPtr<UMaterialInterface>>& Materials)
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

bool operator==(const FVoxelInstancedMeshWeakKey& A, const FVoxelInstancedMeshWeakKey& B)
{
	return
		A.Mesh               == B.Mesh               &&
		A.ActorClass	     == B.ActorClass	     &&
		A.Materials			 == B.Materials			 &&
		A.InstanceSettings   == B.InstanceSettings;
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

uint32 GetTypeHash(const FVoxelInstancedMeshWeakKey& Settings)
{
	return
		HASH(Mesh)
		HASH(ActorClass)
		HASH(Materials)
		HASH(InstanceSettings)
		0;
}
#undef HASH