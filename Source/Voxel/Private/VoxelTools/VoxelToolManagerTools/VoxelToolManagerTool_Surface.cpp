// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool_Surface.h"
#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerToolsHelpers.h"

#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/VoxelSurfaceToolsImpl.h"
#include "VoxelTools/VoxelHardnessHandler.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelWorldGeneratorTools.h"

#include "VoxelWorld.h"
#include "VoxelMessages.h"

#include "VoxelData/VoxelData.h"

#include "Materials/MaterialInstanceDynamic.h"

inline FVoxelToolManagerTool::FToolSettings SurfaceToolSettings(const FVoxelToolManager_SurfaceSettings& SurfaceSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Surface Tool");
	
	Settings.Ptr_Stride = &SurfaceSettings.Stride;
	
	Settings.Ptr_ToolOverlayMaterial = &SurfaceSettings.ToolMaterial;

	Settings.Ptr_bAlignToMovement = &SurfaceSettings.bAlignToMovement;
	Settings.Ptr_Direction = &SurfaceSettings.FixedDirection;

	Settings.Ptr_bFixedNormal = &SurfaceSettings.b2DBrush;
	Settings.FixedNormal = FVector::UpVector;
	
	return Settings;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Surface::FVoxelToolManagerTool_Surface(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, SurfaceToolSettings(ToolManager.SurfaceSettings))
	, SurfaceSettings(ToolManager.SurfaceSettings)
{
}

void FVoxelToolManagerTool_Surface::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();
	
	auto& MaterialInstance = GetToolOverlayMaterialInstance();

	struct FMaskData
	{
		TVoxelTexture<float> MaskTexture;
		UTexture2D* MaskRenderTexture = nullptr;
		float MaskScaleX = 0;
		float MaskScaleY = 0;
		EVoxelRGBA MaskChannel = EVoxelRGBA::R;
	};
	FMaskData MaskData;

	const bool bUseMask = SurfaceSettings.bUseMask && (SurfaceSettings.MaskType == EVoxelToolManagerMaskType::Texture
		? SurfaceSettings.MaskTexture != nullptr
		: SurfaceSettings.MaskWorldGenerator.IsValid());

	// Mask
	if (bUseMask)
	{
		FVoxelMessages::ShowVoxelPluginProError("Using masks requires the Pro version of Voxel Plugin");
		return;
	}

	const bool bIsStrideEnabled = SurfaceSettings.Stride != 0;
	const float MovementStrengthMultiplier = SurfaceSettings.bMovementAffectsStrength ? GetMouseMovementSize() / 100 : 1;
	const float DeltaTimeMultiplier = bIsStrideEnabled ? 1.f : GetDeltaTime();
	const float RadiusMultiplier = bIsStrideEnabled ? ToolManager.Radius / World.VoxelSize : 1.f;

	// Default paint/sculpt strengths are too low to feel good
	const float SculptStrengthStaticMultiplier = bIsStrideEnabled ? 1.f : 50.f;
	const float PaintStrengthStaticMultiplier = 10.f;
	
	const float SculptStrength = SurfaceSettings.SculptStrength * MovementStrengthMultiplier * DeltaTimeMultiplier * SculptStrengthStaticMultiplier * RadiusMultiplier;
	const float PaintStrength = SurfaceSettings.PaintStrength * MovementStrengthMultiplier * DeltaTimeMultiplier * PaintStrengthStaticMultiplier;

	const float SignedSculptStrength = SculptStrength * (TickData.bAlternativeMode ? -1 : 1);
	const float SignedPaintStrength = PaintStrength * (TickData.bAlternativeMode ? -1 : 1);

	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Radius"), ToolManager.Radius);
	MaterialInstance.SetVectorParameterValue(STATIC_FNAME("Position"), GetToolPreviewPosition());
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Falloff"), SurfaceSettings.Falloff);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("EnableFalloff"), SurfaceSettings.bEnableFalloff);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(SurfaceSettings.FalloffType));

	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("SculptHeight"),  SurfaceSettings.bSculpt ? SignedSculptStrength * World.VoxelSize : 0.f);

	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("UseMask"), bUseMask);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("2DBrush"), SurfaceSettings.b2DBrush);

	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Opacity"), SurfaceSettings.ToolOpacity);

	SetToolRenderingBounds(World, FBox(GetToolPreviewPosition() - ToolManager.Radius, GetToolPreviewPosition() + ToolManager.Radius));

	if (!CanEdit())
	{
		return;
	}

	constexpr float DistanceDivisor = 4.f;

	const FVoxelIntBox BoundsToDoEditsIn = UVoxelBlueprintLibrary::MakeIntBoxFromGlobalPositionAndRadius(&World, GetToolPosition(), ToolManager.Radius);
	const FVoxelIntBox BoundsWhereEditsHappen =
		SurfaceSettings.b2DBrush
		? BoundsToDoEditsIn.Extend(FIntVector(0, 0, FMath::CeilToInt(SculptStrength + DistanceDivisor + 2)))
		: BoundsToDoEditsIn;
	
	if (!BoundsToDoEditsIn.IsValid() || !BoundsWhereEditsHappen.IsValid())
	{
		FVoxelMessages::Error("Invalid tool bounds!", &ToolManager);
		return;
	}

	// Don't cache the entire column
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, BoundsToDoEditsIn, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsWhereEditsHappen.Union(BoundsToCache), FUNCTION_FNAME);

		if (ToolManager.bCacheData) Data.CacheBounds<FVoxelValue>(BoundsToCache);

		FVoxelSurfaceEditsVoxels Voxels;
		if (SurfaceSettings.b2DBrush)
		{
			Voxels = UVoxelSurfaceTools::FindSurfaceVoxels2DImpl(Data, BoundsToDoEditsIn, false);
		}
		else
		{
			if (SurfaceSettings.bSculpt)
			{
				Voxels = UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceFieldImpl(Data, BoundsToDoEditsIn, SculptStrength + 3, true);
			}
			else
			{
				// No need to compute the distance field for paint
				// Only select voxels inside the surface
				Voxels = UVoxelSurfaceTools::FindSurfaceVoxelsImpl(Data, BoundsToDoEditsIn, false, true);
			}
		}

		FVoxelSurfaceEditsStack Stack;
		
		if (SurfaceSettings.bEnableFalloff)
		{
			Stack.Add(FVoxelToolManagerToolsHelpers::GetApplyFalloff(
				SurfaceSettings.FalloffType,
				World,
				GetToolPosition(),
				ToolManager.Radius,
				SurfaceSettings.Falloff));
		}

		if (bUseMask)
		{
		}
		
		FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels;
		if (SurfaceSettings.bSculpt && SurfaceSettings.bPaint)
		{
			auto SculptStack = Stack;
			SculptStack.Add(UVoxelSurfaceTools::ApplyConstantStrength(-SignedSculptStrength));
			ProcessedVoxels = SculptStack.Execute(Voxels, false);

			TArray<FModifiedVoxelValue> ModifiedVoxels;
			UVoxelSurfaceTools::EditVoxelValuesImpl(Data, BoundsWhereEditsHappen, ProcessedVoxels, FVoxelHardnessHandler(World), DistanceDivisor, &ModifiedVoxels);
			
			TArray<FVoxelSurfaceEditsVoxel> Materials;
			Materials.Reserve(ModifiedVoxels.Num());
			for (auto& Voxel : ModifiedVoxels)
			{
				if (Voxel.OldValue > 0 && Voxel.NewValue <= 0)
				{
					FVoxelSurfaceEditsVoxel NewVoxel;
					NewVoxel.Position = Voxel.Position;
					NewVoxel.Strength = SurfaceSettings.PaintStrength;
					Materials.Add(NewVoxel);
				}
			}
			UVoxelSurfaceTools::EditVoxelMaterialsImpl(Data, BoundsWhereEditsHappen, ToolManager.PaintMaterial, Materials);
		}
		else if (SurfaceSettings.bSculpt)
		{
			auto SculptStack = Stack;
			SculptStack.Add(UVoxelSurfaceTools::ApplyConstantStrength(-SignedSculptStrength));
			ProcessedVoxels = SculptStack.Execute(Voxels, false);
			
			UVoxelSurfaceTools::EditVoxelValuesImpl(Data, BoundsWhereEditsHappen, ProcessedVoxels, FVoxelHardnessHandler(World), DistanceDivisor);
		}
		else if (SurfaceSettings.bPaint)
		{
			// Note: Painting behaves the same with 2D edit on/off
			auto PaintStack = Stack;
			PaintStack.Add(UVoxelSurfaceTools::ApplyConstantStrength(SignedPaintStrength));
			ProcessedVoxels = PaintStack.Execute(Voxels, false);
			
			UVoxelSurfaceTools::EditVoxelMaterialsImpl(Data, BoundsWhereEditsHappen, ToolManager.PaintMaterial, *ProcessedVoxels.Voxels);
		}

		if (ToolManager.bDebug)
		{
			UVoxelSurfaceTools::DebugSurfaceVoxels(&World, ProcessedVoxels, SurfaceSettings.Stride > 0 ? 1 : 2 * GetDeltaTime());
		}
	}

	SaveFrameOnEndClick(BoundsWhereEditsHappen);
	UpdateWorld(World, BoundsWhereEditsHappen);
}

void FVoxelToolManagerTool_Surface::AddReferencedObjects(FReferenceCollector& Collector)
{
	FVoxelToolManagerTool::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(MaskWorldGeneratorCache.RenderTexture);
}