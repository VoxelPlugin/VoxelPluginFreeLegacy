// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelRender/VoxelPrimitiveComponentSettings.h"

void FVoxelPrimitiveComponentSettings::ApplyToComponent(UPrimitiveComponent& Component) const
{
#define COPY(VariableName) \
	Component.VariableName = VariableName;

	Component.CastShadow = bCastShadow;
	COPY(IndirectLightingCacheQuality);
	COPY(LightmapType);
	COPY(bEmissiveLightSource);
	COPY(bAffectDynamicIndirectLighting);
	COPY(bAffectIndirectLightingWhileHidden);
	COPY(bAffectDistanceFieldLighting);
	COPY(bCastDynamicShadow);
	COPY(bCastStaticShadow);
#if VOXEL_ENGINE_VERSION >= 503
	Component.ShadowCacheInvalidationBehavior = [this]
	{
		switch (ShadowCacheInvalidationBehavior)
		{
		default: ensure(false);
		case EVoxelShadowCacheInvalidationBehavior::Auto: return EShadowCacheInvalidationBehavior::Auto;
		case EVoxelShadowCacheInvalidationBehavior::Always: return EShadowCacheInvalidationBehavior::Always;
		case EVoxelShadowCacheInvalidationBehavior::Rigid: return EShadowCacheInvalidationBehavior::Rigid;
		case EVoxelShadowCacheInvalidationBehavior::Static: return EShadowCacheInvalidationBehavior::Static;
		}
	}();
#endif
	COPY(bCastVolumetricTranslucentShadow);
	COPY(bCastContactShadow);
	COPY(bSelfShadowOnly);
	COPY(bCastFarShadow);
	COPY(bCastInsetShadow);
	COPY(bCastCinematicShadow);
	COPY(bCastHiddenShadow);
	COPY(bCastShadowAsTwoSided);
	COPY(bLightAttachmentsAsGroup);
	COPY(bExcludeFromLightAttachmentGroup);
	COPY(bSingleSampleShadowFromStationaryLights);
	COPY(LightingChannels);
	COPY(bVisibleInReflectionCaptures);
	COPY(bVisibleInRealTimeSkyCaptures);
	COPY(bVisibleInRayTracing);
	COPY(bRenderInMainPass);
	COPY(bRenderInDepthPass);
	COPY(bReceivesDecals);
	COPY(bOwnerNoSee);
	COPY(bOnlyOwnerSee);
	COPY(bTreatAsBackgroundForOcclusion);
	COPY(bUseAsOccluder);
	COPY(bRenderCustomDepth);
	COPY(CustomDepthStencilValue);
	COPY(bVisibleInSceneCaptureOnly);
	COPY(bHiddenInSceneCapture);
	COPY(TranslucencySortPriority);
	COPY(TranslucencySortDistanceOffset);
	COPY(CustomDepthStencilWriteMask);
	COPY(RuntimeVirtualTextures);
	COPY(VirtualTextureLodBias);
	COPY(VirtualTextureCullMips);
	COPY(VirtualTextureMinCoverage);
	COPY(VirtualTextureRenderPassType);

#undef COPY

	Component.MarkRenderStateDirty();
}