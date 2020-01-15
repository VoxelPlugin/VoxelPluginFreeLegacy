// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelInstancedMeshSettings.h"
#include "VoxelSpawners/VoxelHierarchicalInstancedStaticMeshComponent.h"
#include "VoxelSpawners/VoxelActor.h"
#include "VoxelBaseUtilities.h"

inline bool operator==(FInt32Interval A, FInt32Interval B)
{
	return A.Min == B.Min && A.Max == B.Max;
}
inline bool operator==(FWalkableSlopeOverride A, FWalkableSlopeOverride B)
{
	return A.WalkableSlopeBehavior == B.WalkableSlopeBehavior && A.WalkableSlopeAngle == B.WalkableSlopeAngle;
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
		return
			A.ObjectType == B.ObjectType &&
			A.CollisionResponses == B.CollisionResponses &&
			A.WalkableSlopeOverride == B.WalkableSlopeOverride &&
			A.PhysMaterialOverride == B.PhysMaterialOverride;
	}
	inline static uint32 GetTypeHashBody(const FBodyInstance& A)
	{
		return uint32(A.ObjectType) ^ GetTypeHash(A.PhysMaterialOverride);
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

template<typename T>
inline uint32 GetTypeHash2(const TInterval<T>& Interval)
{
	return HashCombine(GetTypeHash(Interval.Min), GetTypeHash(Interval.Max));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelInstancedMeshSettings::FVoxelInstancedMeshSettings()
{
	BodyInstance.SetCollisionProfileName("NoCollision");
}

// Requires VoxelHISM include, so in .cpp
bool operator==(const FVoxelInstancedMeshSettings& A, const FVoxelInstancedMeshSettings& B)
{
	return
		A.Mesh                           == B.Mesh                           &&
		A.ActorTemplate                  == B.ActorTemplate                  &&
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

uint32 GetTypeHash(const FVoxelInstancedMeshSettings& Settings)
{
	// Ugly hash...
#define HASH_IMPL(G, X) (G(Settings.X) * (FVoxelUtilities::MurmurHash32(__LINE__) & 0xFFFF)) ^
#define HASH(X) HASH_IMPL(GetTypeHash, X)
	return
		HASH(Mesh)
		HASH(ActorTemplate)
		HASH_IMPL(GetTypeHash2, CullDistance)
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
#undef HASH
#undef HASH_IMPL
}