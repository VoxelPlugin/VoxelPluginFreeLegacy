// Copyright 2021 Phyronnaz

#include "VoxelTools/Tools/VoxelSurfaceTool.h"
#include "VoxelWorld.h"
#include "VoxelMessages.h"
#include "VoxelUtilities/VoxelExampleUtilities.h"
#include "VoxelGenerators/VoxelGeneratorTools.h"
#include "VoxelTools/Impl/VoxelSurfaceEditToolsImpl.inl"
#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/VoxelSurfaceToolsImpl.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelHardnessHandler.h"

#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"

UVoxelSurfaceTool::UVoxelSurfaceTool()
{
	ToolName = TEXT("Surface");
	bShowPaintMaterial = true;
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Surface"));
	ToolMaterial = ToolMaterialFinder.Object;
	Mask.Texture = FVoxelExampleUtilities::LoadExampleObject<UTexture2D>(TEXT("/Voxel/Examples/VoxelDefaultBrushMask"));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTool::GetToolConfig(FVoxelToolBaseConfig& OutConfig) const
{
	OutConfig.Stride = Stride;
	
	OutConfig.OverlayMaterial = ToolMaterial;

	OutConfig.bUseFixedDirection = !bAlignToMovement;
	OutConfig.FixedDirection = FixedDirection;

	OutConfig.bUseFixedNormal = b2DBrush;
	OutConfig.FixedNormal = FVector::UpVector;
}

void UVoxelSurfaceTool::Tick()
{
	Super::Tick();
	
	Falloff = GetValueAfterAxisInput(FVoxelToolAxes::Falloff, Falloff);
	if (bSculpt)
	{
		SculptStrength = GetValueAfterAxisInput(FVoxelToolAxes::Strength, SculptStrength);
	}
	else if (bPaint)
	{
		PaintStrength = GetValueAfterAxisInput(FVoxelToolAxes::Strength, PaintStrength);
	}
}

void UVoxelSurfaceTool::UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();

	if (!OverlayMaterialInstance)
	{
		return;
	}

	const float VoxelSize = GetVoxelWorld()->VoxelSize;

	if (ShouldUseMask())
	{
	}

	const float Radius = SharedConfig->BrushSize / 2.f;
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Radius"), Radius);
	OverlayMaterialInstance->SetVectorParameterValue(STATIC_FNAME("Position"), GetToolPreviewPosition());
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Falloff"), Falloff);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("EnableFalloff"), bEnableFalloff);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(FalloffType));

	float SignedSculptStrength;
	float SignedPaintStrength;
	GetStrengths(SignedSculptStrength, SignedPaintStrength);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("SculptHeight"),  bSculpt ? SignedSculptStrength * VoxelSize : 0.f);

	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("UseMask"), ShouldUseMask());
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("2DBrush"), b2DBrush);

	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Opacity"), SharedConfig->ToolOpacity);

	SetToolOverlayBounds(FBox(GetToolPreviewPosition() - Radius, GetToolPreviewPosition() + Radius));
}

FVoxelIntBoxWithValidity UVoxelSurfaceTool::DoEdit()
{
	VOXEL_FUNCTION_COUNTER();
	
	constexpr float DistanceDivisor = 4.f;
	
	float SignedSculptStrength;
	float SignedPaintStrength;
	GetStrengths(SignedSculptStrength, SignedPaintStrength);

	const float Radius = SharedConfig->BrushSize / 2.f;
	const FVoxelIntBox BoundsToDoEditsIn = UVoxelBlueprintLibrary::MakeIntBoxFromGlobalPositionAndRadius(GetVoxelWorld(), GetToolPosition(), Radius);
	const FVoxelIntBox BoundsWhereEditsHappen =
		b2DBrush
		? BoundsToDoEditsIn.Extend(FIntVector(0, 0, FMath::CeilToInt(FMath::Abs(SignedSculptStrength) + DistanceDivisor + 2)))
		: BoundsToDoEditsIn;
	
	if (!BoundsToDoEditsIn.IsValid() || !BoundsWhereEditsHappen.IsValid())
	{
		FVoxelMessages::Error("Invalid tool bounds!", this);
		return {};
	}
	
	// Don't cache the entire column
	const auto BoundsToCache = GetBoundsToCache(BoundsToDoEditsIn);

	FVoxelData& Data = GetVoxelWorld()->GetSubsystemChecked<FVoxelData>();
	auto DataImpl = GetDataImpl(Data);

	FVoxelWriteScopeLock Lock(Data, BoundsWhereEditsHappen.Union(BoundsToCache), FUNCTION_FNAME);
	CacheData<FVoxelValue>(Data, BoundsToCache);
	
	FVoxelSurfaceEditsVoxels Voxels;
	if (b2DBrush)
	{
		Voxels = UVoxelSurfaceTools::FindSurfaceVoxels2DImpl(Data, BoundsToDoEditsIn, false);
	}
	else
	{
		if (bSculpt)
		{
			Voxels = UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceFieldImpl(Data, BoundsToDoEditsIn, SharedConfig->bMultiThreaded, SharedConfig->GetComputeDevice());
		}
		else
		{
			// No need to compute the distance field for paint
			// Only select voxels inside the surface
			Voxels = UVoxelSurfaceTools::FindSurfaceVoxelsImpl(Data, BoundsToDoEditsIn, false, true);
		}
	}

	FVoxelSurfaceEditsStack Stack;
	
	if (bEnableFalloff)
	{
		Stack.Add(UVoxelSurfaceTools::ApplyFalloff(
			GetVoxelWorld(),
			FalloffType,
			GetToolPosition(),
			Radius,
			Falloff));
	}

	if (ShouldUseMask())
	{
		FVoxelMessages::Info("Using masks requires the Pro version of Voxel Plugin");
	}

	const FVoxelHardnessHandler HardnessHandler(*GetVoxelWorld());
	
	FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels;
	if (bSculpt && bPaint)
	{
		auto SculptStack = Stack;
		SculptStack.Add(UVoxelSurfaceTools::ApplyConstantStrength(-SignedSculptStrength));
		ProcessedVoxels = SculptStack.Execute(Voxels, false);

		auto RecordingDataImpl = GetDataImpl<FModifiedVoxelValue>(Data);
		FVoxelSurfaceEditToolsImpl::EditVoxelValues(DataImpl, HardnessHandler, BoundsWhereEditsHappen, ProcessedVoxels, DistanceDivisor);
		
		TArray<FVoxelSurfaceEditsVoxel> Materials;
		Materials.Reserve(RecordingDataImpl.ModifiedValues.Num());
		for (auto& Voxel : RecordingDataImpl.ModifiedValues)
		{
			if (Voxel.OldValue > 0 && Voxel.NewValue <= 0)
			{
				FVoxelSurfaceEditsVoxel NewVoxel;
				NewVoxel.Position = Voxel.Position;
				NewVoxel.Strength = PaintStrength;
				Materials.Add(NewVoxel);
			}
		}
		FVoxelSurfaceEditToolsImpl::EditVoxelMaterials(DataImpl, BoundsWhereEditsHappen, SharedConfig->PaintMaterial, Materials);
	}
	else if (bSculpt)
	{
		auto SculptStack = Stack;
		SculptStack.Add(UVoxelSurfaceTools::ApplyConstantStrength(-SignedSculptStrength));
		ProcessedVoxels = SculptStack.Execute(Voxels, false);

		if (bPropagateMaterials && !b2DBrush)
		{
			FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials(DataImpl, ProcessedVoxels);
		}
		
		FVoxelSurfaceEditToolsImpl::EditVoxelValues(DataImpl, HardnessHandler, BoundsWhereEditsHappen, ProcessedVoxels, DistanceDivisor);
	}
	else if (bPaint)
	{
		// Note: Painting behaves the same with 2D edit on/off
		auto PaintStack = Stack;
		PaintStack.Add(UVoxelSurfaceTools::ApplyConstantStrength(SignedPaintStrength));
		ProcessedVoxels = PaintStack.Execute(Voxels, false);
		
		FVoxelSurfaceEditToolsImpl::EditVoxelMaterials(DataImpl, BoundsWhereEditsHappen, SharedConfig->PaintMaterial, *ProcessedVoxels.Voxels);
	}

	if (SharedConfig->bDebug)
	{
		UVoxelSurfaceTools::DebugSurfaceVoxels(GetVoxelWorld(), ProcessedVoxels, Stride > 0 ? 1 : 2 * GetDeltaTime());
	}

	return BoundsWhereEditsHappen;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


bool UVoxelSurfaceTool::ShouldUseMask() const
{
	return bUseMask && (Mask.Type == EVoxelSurfaceToolMaskType::Texture ? Mask.Texture != nullptr : Mask.Generator.IsValid());
}

void UVoxelSurfaceTool::GetStrengths(float& OutSignedSculptStrength, float& OutSignedPaintStrength) const
{
	const bool bIsStrideEnabled = Stride != 0;
	const bool bUseDeltaTimeForSculpt = bModulateStrengthByDeltaTime && !bIsStrideEnabled;
	const bool bUseDeltaTimeForPaint = bUseDeltaTimeForSculpt && PaintStrength < 1.f;

	const float MovementStrengthMultiplier = bMovementAffectsStrength ? GetMouseMovementSize() / 100 : 1;
	const float RadiusMultiplier = bIsStrideEnabled ? SharedConfig->BrushSize / 2.f / GetVoxelWorld()->VoxelSize : 1.f;

	// Default paint/sculpt strengths are too low to feel good
	const float SculptStrengthStaticMultiplier = bIsStrideEnabled ? 1.f : 50.f;
	const float PaintStrengthStaticMultiplier = 10.f;
	
	const float ActualSculptStrength = SculptStrength * MovementStrengthMultiplier * (bUseDeltaTimeForSculpt ? GetDeltaTime() : 1.f) * SculptStrengthStaticMultiplier * RadiusMultiplier;
	const float ActualPaintStrength = PaintStrength * MovementStrengthMultiplier * (bUseDeltaTimeForPaint ? GetDeltaTime() : 1.f) * PaintStrengthStaticMultiplier;

	const bool bAlternativeMode = GetTickData().IsAlternativeMode();
	OutSignedSculptStrength = ActualSculptStrength * (bAlternativeMode ? -1 : 1);
	OutSignedPaintStrength = ActualPaintStrength * (bAlternativeMode ? -1 : 1);
}