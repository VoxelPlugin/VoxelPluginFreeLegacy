// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManagerTools.h"
#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/VoxelSphereTools.h"
#include "VoxelTools/VoxelProjectionTools.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelHardnessHandler.h"
#include "VoxelTools/VoxelAssetTools.h"
#include "VoxelImporters/VoxelMeshImporter.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelMessages.h"
#include "VoxelThreadingUtilities.h"
#include "VoxelDebugUtilities.h"
#include "VoxelTexture.h"
#include "VoxelWorld.h"

#include "VoxelData/VoxelData.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

FVoxelToolManagerTool::FVoxelToolManagerTool(const UVoxelToolManager& ToolManager, const FToolSettings& ToolSettings)
	: ToolManager(ToolManager)
	, ToolSettings(ToolSettings)
{
}

FVoxelToolManagerTool::~FVoxelToolManagerTool()
{
	ClearVoxelWorld();
}

void FVoxelToolManagerTool::TriggerTick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData, const FHitResult& Hit)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (&World != VoxelWorld) ClearVoxelWorld();
	VoxelWorld = &World;

	if (ToolSettings.bWaitForUpdates && NumPendingUpdates > 0)
	{
		if (ToolManager.bDebug)
		{
			GEngine->AddOnScreenDebugMessage(
				OBJECT_LINE_ID(),
				1.5f * TickData.DeltaTime,
				FColor::Yellow,
				FString::Printf(TEXT("Waiting for %d updates"), NumPendingUpdates));
		}
		return;
	}

	MouseMovementSize = (LastFrameTickData.MousePosition - TickData.MousePosition).Size();

	if (ToolSettings.bViewportSpaceMovement)
	{
		if (*ToolSettings.Alignment == EVoxelToolManagerAlignment::Surface)
		{
			UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(TickData.bClick);
			if (Hit.bBlockingHit)
			{
				CurrentPosition = Hit.ImpactPoint;
				CurrentNormal = Hit.ImpactNormal;
			}
		}
		else
		{
			if (!LastFrameTickData.bClick)
			{
				FVector Normal = FVector::UpVector;
				switch (*ToolSettings.Alignment)
				{
				case EVoxelToolManagerAlignment::View:
					Normal = -TickData.CameraViewDirection;
					break;
				case EVoxelToolManagerAlignment::Ground:
					Normal = FVector::UpVector;
					break;
				case EVoxelToolManagerAlignment::Up:
					Normal = -TickData.CameraViewDirection;
					Normal.Z = 0;
					if (!Normal.Normalize())
					{
						ensure(TickData.CameraViewDirection.GetAbs() == FVector::UpVector);
						Normal = FVector::RightVector;
					}
					break;
				default: ensure(false);
				}
				const bool bAirMode = !Hit.bBlockingHit || *ToolSettings.bAirMode;
				const FVector Point = bAirMode
					? TickData.RayOrigin + TickData.RayDirection * *ToolSettings.DistanceToCamera
					: FVector(Hit.ImpactPoint);
				LastClickPlane = FPlane(Point, Normal);
				LastClickNormal = bAirMode ? FVector::UpVector : Hit.ImpactNormal;
			}
			CurrentPosition = FMath::RayPlaneIntersection(TickData.RayOrigin, TickData.RayDirection, LastClickPlane);
			CurrentNormal = LastClickNormal;
		}
	}
	else
	{
		if (Hit.bBlockingHit)
		{
			CurrentPosition = Hit.ImpactPoint;
			CurrentNormal = Hit.ImpactNormal;
		}
	}

	if (ToolSettings.bSupportToolDirection)
	{
		const FVector NewMovementTangent = (CurrentPosition - LastPositionUsedForTangent).GetSafeNormal();
		MovementTangent = FMath::Lerp(
			MovementTangent,
			NewMovementTangent,
			FMath::Clamp((1 - ToolManager.AlignToMovementSmoothness) * GetMouseMovementSize() / 10, 0.f, 1.f)).GetSafeNormal();
		LastPositionUsedForTangent = CurrentPosition;
	}

	if (ToolSettings.bSupportStride)
	{
		const double Time = FPlatformTime::Seconds();
		if (Time > LastStridePositionUpdate + *ToolSettings.Stride)
		{
			StridePosition = CurrentPosition;
			StrideNormal = CurrentNormal;
			if (ToolSettings.bSupportToolDirection)
			{
				StrideDirection = MovementTangent;
			}
			LastStridePositionUpdate = Time;
		}
	}

	if (ToolSettings.bSaveFrameOnEndClick && !TickData.bClick && PendingFrameBounds.IsValid())
	{
		ToolManager.SaveFrame(World, PendingFrameBounds.GetBox(), ToolSettings.ToolName);
		PendingFrameBounds.Reset();
	}

	if (ToolManager.bDebug)
	{
		if (ToolSettings.bSupportToolDirection)
		{
			DrawDebugDirectionalArrow(
				TickData.World,
				GetToolPosition(),
				GetToolPosition() + GetToolDirection() * World.VoxelSize * 5,
				World.VoxelSize * 5,
				FColor::Red,
				false,
				1.5f * TickData.DeltaTime,
				0,
				World.VoxelSize / 2);
		}
		if (ToolSettings.bViewportSpaceMovement && TickData.bClick && *ToolSettings.Alignment != EVoxelToolManagerAlignment::Surface)
		{
			DrawDebugSolidPlane(
				TickData.World,
				LastClickPlane,
				CurrentPosition,
				1000000,
				FColor::Red,
				false,
				1.5f * TickData.DeltaTime);
		}
		DrawDebugDirectionalArrow(
			TickData.World,
			GetToolPosition(),
			GetToolPosition() + GetToolNormal() * World.VoxelSize * 5,
			World.VoxelSize * 5,
			FColor::Blue,
			false,
			1.5f * TickData.DeltaTime,
			0,
			World.VoxelSize / 2);
	}
	
	if (ToolSettings.bNeedToolRendering)
	{
		auto& ToolRenderingManager = World.GetToolRenderingManager();
		if (!ToolRenderingId.IsValid() || !ToolRenderingManager.IsValidTool(ToolRenderingId))
		{
			ToolRenderingId = ToolRenderingManager.CreateTool(true);
		}

		auto* const ToolMaterial = *ToolSettings.ToolMaterial;
		if (!ToolMaterial)
		{
			FVoxelMessages::Error("VoxelToolManager: " + ToolSettings.ToolName.ToString() + ": Invalid ToolMaterial!");
			return;
		}
		if (!ToolMaterialInstance || ToolMaterialInstance->GetMaterial() != ToolMaterial)
		{
			ToolMaterialInstance = UMaterialInstanceDynamic::Create(ToolMaterial, GetTransientPackage());
			if (!ensure(ToolMaterialInstance)) return;
		}
		check(ToolMaterialInstance);
		
		ToolRenderingManager.EditTool(ToolRenderingId, [&](FVoxelToolRendering& Tool)
		{
			if (!Tool.Material.IsValid() || Tool.Material->GetMaterial() != ToolMaterialInstance) 
			{
				Tool.Material = FVoxelMaterialInterface::Create(ToolMaterialInstance);
			}
		});
	}

	if (ToolSettings.bNeedToolMesh && StaticMeshActor.IsValid())
	{
		auto& MeshComponent = *StaticMeshActor->GetStaticMeshComponent();
		if (MeshComponent.GetStaticMesh() != *ToolSettings.ToolMesh)
		{
			MeshComponent.SetStaticMesh(*ToolSettings.ToolMesh);
			for (int32 Index = 0; Index < MeshComponent.GetNumMaterials(); Index++)
			{
				MeshComponent.SetMaterial(Index, *ToolSettings.ToolMeshMaterial);
			}
		}
	}

	Tick(World, TickData);

	LastFrameTickData = TickData;
}

void FVoxelToolManagerTool::ClearVoxelWorld()
{
	VOXEL_FUNCTION_COUNTER();
	
	if (VoxelWorld.IsValid() && VoxelWorld->IsCreated())
	{
		if (ToolSettings.bSaveFrameOnEndClick && PendingFrameBounds.IsValid())
		{
			ToolManager.SaveFrame(*VoxelWorld, PendingFrameBounds.GetBox(), ToolSettings.ToolName);
			PendingFrameBounds.Reset();
		}
		if (ToolRenderingId.IsValid())
		{
			auto& ToolRenderingManager = VoxelWorld->GetToolRenderingManager();
			if (ToolRenderingManager.IsValidTool(ToolRenderingId)) // Could be invalid if the voxel world was toggled off & on
			{
				ToolRenderingManager.RemoveTool(ToolRenderingId);
			}
		}
	}
	VoxelWorld.Reset();
	ToolRenderingId.Reset();

	if (StaticMeshActor.IsValid())
	{
		StaticMeshActor->Destroy();
	}

	UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(false);
}

///////////////////////////////////////////////////////////////////////////////

FVector FVoxelToolManagerTool::GetToolDirection() const
{
	ensure(ToolSettings.bSupportToolDirection);
	if (!ToolSettings.bAlignToMovement || *ToolSettings.bAlignToMovement)
	{
		return ToolSettings.bSupportStride && *ToolSettings.bEnableStride ? StrideDirection : MovementTangent;
	}
	else
	{
		return ToolSettings.Direction->Vector();
	}
}

FVector FVoxelToolManagerTool::GetToolPosition() const
{
	return ToolSettings.bSupportStride && *ToolSettings.bEnableStride ? StridePosition : CurrentPosition;
}

FVector FVoxelToolManagerTool::GetToolNormal() const
{
	return ToolSettings.bSupportStride && *ToolSettings.bEnableStride ? StrideNormal : CurrentNormal;
}

UMaterialInstanceDynamic& FVoxelToolManagerTool::GetToolMaterialInstance() const
{
	check(ToolSettings.bNeedToolRendering);
	check(ToolMaterialInstance);
	return *ToolMaterialInstance;
}

void FVoxelToolManagerTool::UpdateWorld(AVoxelWorld& World, const FIntBox& Bounds)
{
	NumPendingUpdates += World.GetLODManager().UpdateBounds_OnAllFinished(
		Bounds,
		FVoxelUtilities::MakeVoxelWeakPtrDelegate(this, 
		[VoxelWorld = MakeWeakObjectPtr(&World), Bounds](FVoxelToolManagerTool& This)
		{
			if (VoxelWorld.IsValid())
			{
				This.ToolManager.OnBoundsUpdated.Broadcast(VoxelWorld.Get(), Bounds);
				if (This.ToolManager.bDebug)
				{
					UVoxelDebugUtilities::DrawDebugIntBox(VoxelWorld.Get(), Bounds, FTransform(), 0.1f);
				}
			}
		}),
		FVoxelUtilities::MakeVoxelWeakPtrDelegate<void, FIntBox>(this, 
		[](FVoxelToolManagerTool& This, const FIntBox& ChunkBounds)
		{
			ensure(This.NumPendingUpdates-- >= 0);
		}));
}

void FVoxelToolManagerTool::SaveFrameOnEndClick(const FIntBox& Bounds)
{
	ensure(ToolSettings.bSaveFrameOnEndClick);
	PendingFrameBounds += Bounds;
}

void FVoxelToolManagerTool::SetToolRenderingBounds(AVoxelWorld& World, const FBox& Bounds)
{
	if (!ensure(&World == VoxelWorld)) return;
	World.GetToolRenderingManager().EditTool(ToolRenderingId, [&](FVoxelToolRendering& Tool)
	{
		Tool.WorldBounds = Bounds;
	});
}

void FVoxelToolManagerTool::SetToolMeshTransform(UWorld* World, const FTransform& Transform)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(ToolSettings.bNeedToolMesh);
	if (!StaticMeshActor.IsValid())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.bDeferConstruction = true;
		SpawnParameters.ObjectFlags = RF_Transient;
		StaticMeshActor = World->SpawnActor<AStaticMeshActor>(SpawnParameters);
		StaticMeshActor->SetMobility(EComponentMobility::Movable);
#if WITH_EDITOR
		StaticMeshActor->SetActorLabel("VoxelToolMeshActor");
#endif
		auto& MeshComponent = *StaticMeshActor->GetStaticMeshComponent();
		MeshComponent.SetStaticMesh(*ToolSettings.ToolMesh);
		for (int32 Index = 0; Index < MeshComponent.GetNumMaterials(); Index++)
		{
			MeshComponent.SetMaterial(Index, *ToolSettings.ToolMeshMaterial);
		}
		StaticMeshActor->SetActorEnableCollision(false);
		StaticMeshActor->FinishSpawning(Transform);
	}
	if (!ensure(StaticMeshActor.IsValid())) return;
	StaticMeshActor->SetActorTransform(Transform);
}

FIntBox FVoxelToolManagerTool::GetAndDebugBoundsToCache(AVoxelWorld& World, const FIntBox& Bounds, const FVoxelToolManagerTickData& TickData) const
{
	const auto BoundsToCache = UVoxelBlueprintLibrary::GetRenderBoundsOverlappingDataBounds(&World, Bounds);

	if (ToolManager.bDebug)
	{
		UVoxelDebugUtilities::DrawDebugIntBox(&World, Bounds, 1.5f * TickData.DeltaTime, 0, FColor::Green);
		UVoxelDebugUtilities::DrawDebugIntBox(&World, BoundsToCache, 1.5f * TickData.DeltaTime, 0, FColor::Red);
	}

	return BoundsToCache;
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelToolManagerTool::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ToolMaterialInstance);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline FVoxelToolManagerTool::FToolSettings SurfaceToolSettings(const FVoxelToolManager_SurfaceSettings& SurfaceSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Surface Tool");
	
	Settings.bSupportStride = true;
	Settings.bEnableStride = &SurfaceSettings.bEnableStride;
	Settings.Stride = &SurfaceSettings.Stride;
	
	Settings.bNeedToolRendering = true;
	Settings.ToolMaterial = &SurfaceSettings.ToolMaterial;

	Settings.bSupportToolDirection = true;
	Settings.bAlignToMovement = &SurfaceSettings.bAlignToMovement;
	Settings.Direction = &SurfaceSettings.FixedDirection;

	Settings.bWaitForUpdates = true;
	Settings.bSaveFrameOnEndClick = true;
	
	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings TrimToolSettings(const FVoxelToolManager_TrimSettings& TrimSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Trim Tool");

	Settings.bNeedToolRendering = true;
	Settings.ToolMaterial = &TrimSettings.ToolMaterial;

	Settings.bWaitForUpdates = true;
	Settings.bSaveFrameOnEndClick = true;
	
	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings SmoothToolSettings(const FVoxelToolManager_SmoothSettings& SmoothSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Smooth Tool");

	Settings.bNeedToolRendering = true;
	Settings.ToolMaterial = &SmoothSettings.ToolMaterial;

	Settings.bWaitForUpdates = true;
	Settings.bSaveFrameOnEndClick = true;
	
	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings SphereToolSettings(const FVoxelToolManager_SphereSettings& SphereSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Sphere Tool");

	Settings.bNeedToolMesh = true;
	Settings.ToolMesh = &SphereSettings.SphereMesh;
	Settings.ToolMeshMaterial = &SphereSettings.ToolMaterial;
	
	Settings.bViewportSpaceMovement = true;
	Settings.Alignment = &SphereSettings.Alignment;
	Settings.bAirMode = &SphereSettings.bAirMode;
	Settings.DistanceToCamera = &SphereSettings.DistanceToCamera;
	
	Settings.bWaitForUpdates = true;
	Settings.bSaveFrameOnEndClick = true;
	
	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings MeshToolSettings(const FVoxelToolManager_MeshSettings& MeshSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Mesh Tool");
	
	Settings.bSupportStride = true;
	Settings.bEnableStride = &MeshSettings.bEnableStride;
	Settings.Stride = &MeshSettings.Stride;

	Settings.bNeedToolMesh = true;
	Settings.ToolMesh = &MeshSettings.Mesh;
	Settings.ToolMeshMaterial = &MeshSettings.ToolMaterial;

	Settings.bSupportToolDirection = true;
	
	Settings.bViewportSpaceMovement = true;
	Settings.Alignment = &MeshSettings.Alignment;
	Settings.bAirMode = &MeshSettings.bAirMode;
	Settings.DistanceToCamera = &MeshSettings.DistanceToCamera;
	
	Settings.bWaitForUpdates = true;
	Settings.bSaveFrameOnEndClick = true;
	
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

	struct FMaskData
	{
		TVoxelTexture<float> MaskTexture;
		float MaskScaleX = 0;
		float MaskScaleY = 0;
		FVector MaskPlaneX;
		FVector MaskPlaneY;
	};
	FMaskData MaskData;

	const float RadiusWithFalloff = ToolManager.Radius * (1 + SurfaceSettings.Falloff);

	auto& MaterialInstance = GetToolMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Radius"), ToolManager.Radius);
	MaterialInstance.SetVectorParameterValue(STATIC_FNAME("Position"), GetToolPosition());
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(SurfaceSettings.FalloffType));
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Falloff"), ToolManager.Radius * SurfaceSettings.Falloff);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("UseMask"), false);

	if (SurfaceSettings.bUseMask && SurfaceSettings.Mask)
	{
		FVoxelMessages::ShowVoxelPluginProError("Using masks requires the Pro version of Voxel Plugin");
		return;
	}

	SetToolRenderingBounds(World, FBox(GetToolPosition() - RadiusWithFalloff, GetToolPosition() + RadiusWithFalloff));

	if (!TickData.bClick)
	{
		return;
	}

	const FIntBox Bounds = UVoxelBlueprintLibrary::MakeIntBoxFromGlobalPositionAndRadius(&World, GetToolPosition(), RadiusWithFalloff);
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		Data.CacheBounds<FVoxelValue>(BoundsToCache);
		
		TArray<FSurfaceVoxel> Voxels;
		UVoxelSurfaceTools::FindSurfaceVoxelsImpl(Data, Bounds, Voxels);
		
		TArray<FSurfaceVoxelWithStrength> Strengths;
		{
			Strengths = UVoxelSurfaceTools::AddStrengthToSurfaceVoxels(Voxels);
		
			const FVector ToolVoxelPosition = World.GlobalToLocalFloat(GetToolPosition());
			const auto ApplyFalloff = [&](auto F)
			{
				UVoxelSurfaceTools::ApplyStrengthFunctionImpl(
					Strengths,
					ToolVoxelPosition,
					F);
			};

			const float RadiusInVoxels = ToolManager.Radius / World.VoxelSize;
			const float FalloffInVoxels = RadiusInVoxels * SurfaceSettings.Falloff;
			switch (SurfaceSettings.FalloffType)
			{
			case EVoxelToolManagerFalloff::Linear:
				ApplyFalloff([=](float Distance) { return FVoxelUtilities::LinearFalloff(Distance, RadiusInVoxels, FalloffInVoxels); });
				break;
			case EVoxelToolManagerFalloff::Smooth:
				ApplyFalloff([=](float Distance) { return FVoxelUtilities::SmoothFalloff(Distance, RadiusInVoxels, FalloffInVoxels); });
				break;
			case EVoxelToolManagerFalloff::Spherical:
				ApplyFalloff([=](float Distance) { return FVoxelUtilities::SphericalFalloff(Distance, RadiusInVoxels, FalloffInVoxels); });
				break;
			case EVoxelToolManagerFalloff::Tip:
				ApplyFalloff([=](float Distance) { return FVoxelUtilities::TipFalloff(Distance, RadiusInVoxels, FalloffInVoxels); });
				break;
			default: ensure(false);
			}

			if (SurfaceSettings.bUseMask && SurfaceSettings.Mask)
			{
			}
		}

		const float StrengthMultiplier = SurfaceSettings.bMovementAffectsStrength ? GetMouseMovementSize() / 100 : 1;
		const float SculptStrength = SurfaceSettings.SculptStrength * StrengthMultiplier * (TickData.bAlternativeMode ? 1 : -1);
		const float PaintStrength = SurfaceSettings.PaintStrength * StrengthMultiplier * (TickData.bAlternativeMode ? -1 : 1);
		if (SurfaceSettings.bSculpt && SurfaceSettings.bPaint)
		{
			auto SculptStrengths = Strengths;
			UVoxelSurfaceTools::ApplyConstantStrengthImpl(SculptStrengths, SculptStrength);

			TArray<FModifiedVoxelValue> ModifiedVoxels;
			UVoxelSurfaceTools::EditVoxelValuesImpl(Data, ModifiedVoxels, Bounds, UVoxelSurfaceTools::CreateValueEditsFromSurfaceVoxels(SculptStrengths), FVoxelHardnessHandler(World));
			
			TArray<FVoxelMaterialEdit> Materials;
			Materials.Reserve(ModifiedVoxels.Num());
			for (auto& Voxel : ModifiedVoxels)
			{
				if (Voxel.OldValue > 0 && Voxel.NewValue <= 0)
				{
					Materials.Emplace(FVoxelEditBase(Voxel.Position, SurfaceSettings.PaintStrength), ToolManager.PaintMaterial);
				}
			}
			UVoxelSurfaceTools::EditVoxelMaterialsImpl(Data, Bounds, Materials);
		}
		else if (SurfaceSettings.bSculpt)
		{
			auto SculptStrengths = Strengths;
			UVoxelSurfaceTools::ApplyConstantStrengthImpl(SculptStrengths, SculptStrength);
			UVoxelSurfaceTools::EditVoxelValuesImpl(Data, Bounds, UVoxelSurfaceTools::CreateValueEditsFromSurfaceVoxels(SculptStrengths), FVoxelHardnessHandler(World));
		}
		else if (SurfaceSettings.bPaint)
		{
			auto MaterialStrengths = Strengths;
			UVoxelSurfaceTools::ApplyConstantStrengthImpl(MaterialStrengths, PaintStrength);
			UVoxelSurfaceTools::EditVoxelMaterialsImpl(Data, Bounds, UVoxelSurfaceTools::CreateMaterialEditsFromSurfaceVoxels(MaterialStrengths, ToolManager.PaintMaterial));
		}

		if (ToolManager.bDebug)
		{
			for (auto& Strength : Strengths)
			{
				const FVector Position = World.LocalToGlobal(Strength.Position);
				const FLinearColor Color = FMath::Lerp(
					FLinearColor::Black,
					Strength.Strength > 0 ? FLinearColor::Red : FLinearColor::Green,
					FMath::Clamp(FMath::Abs(Strength.Strength), 0.f, 1.f));
				DrawDebugPoint(
					TickData.World,
					Position,
					World.VoxelSize / 20,
					Color.ToFColor(false),
					false,
					2 * TickData.DeltaTime);
			}
		}
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Trim::FVoxelToolManagerTool_Trim(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, TrimToolSettings(ToolManager.TrimSettings))
	, TrimSettings(ToolManager.TrimSettings)
{
}

void FVoxelToolManagerTool_Trim::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();
	
	const float RadiusWithFalloff = ToolManager.Radius * (1 + TrimSettings.Falloff);

	FVoxelLineTraceParameters Parameters;
	Parameters.CollisionChannel = World.CollisionPresets.GetObjectType();
	Parameters.DrawDebugType = ToolManager.bDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	TArray<FVoxelProjectionHit> Hits;
	const float RaysRadius = FMath::Max(RadiusWithFalloff * (1 - TrimSettings.Roughness), World.VoxelSize);
	UVoxelProjectionTools::FindProjectionVoxels(
		Hits,
		&World,
		Parameters,
		GetToolPosition() - TickData.RayDirection * RadiusWithFalloff,
		TickData.RayDirection,
		RaysRadius,
		EVoxelProjectionShape::Circle,
		100.f,
		WORLD_MAX);

	const FVector Position = UVoxelProjectionTools::GetHitsAveragePosition(Hits);

	SetToolRenderingBounds(World, FBox(Position - RadiusWithFalloff, Position + RadiusWithFalloff));

	auto& MaterialInstance = GetToolMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Radius"), ToolManager.Radius);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Falloff"), ToolManager.Radius * TrimSettings.Falloff);
	MaterialInstance.SetVectorParameterValue(STATIC_FNAME("Position"), Position);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Roughness"), TrimSettings.Roughness);

	if (!TickData.bClick)
	{
		return;
	}

	const FVector Normal = UVoxelProjectionTools::GetHitsAverageNormal(Hits);
	
	const FVector VoxelPosition = World.GlobalToLocalFloat(Position);
	const float VoxelRadius = ToolManager.Radius / World.VoxelSize;
	const float VoxelRadiusWithFalloff = VoxelRadius * (1 + TrimSettings.Falloff);

	const FIntBox Bounds = UVoxelSphereTools::GetSphereBounds(VoxelPosition, VoxelRadiusWithFalloff);
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		Data.CacheBounds<FVoxelValue>(BoundsToCache);

		UVoxelSphereTools::TrimSphereImpl(
			Data,
			VoxelPosition,
			Normal,
			VoxelRadius,
			VoxelRadius * TrimSettings.Falloff,
			!TickData.bAlternativeMode);
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Smooth::FVoxelToolManagerTool_Smooth(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, SmoothToolSettings(ToolManager.SmoothSettings))
	, SmoothSettings(ToolManager.SmoothSettings)
{
}

void FVoxelToolManagerTool_Smooth::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();
	
	SetToolRenderingBounds(World, FBox(GetToolPosition() - ToolManager.Radius, GetToolPosition() + ToolManager.Radius));

	auto& MaterialInstance = GetToolMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Radius"), ToolManager.Radius);
	MaterialInstance.SetVectorParameterValue(STATIC_FNAME("Position"), GetToolPosition());

	if (!TickData.bClick)
	{
		return;
	}

	const FVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition());
	const float VoxelRadius = ToolManager.Radius / World.VoxelSize;

	const FIntBox Bounds = UVoxelSphereTools::GetSphereBounds(VoxelPosition, VoxelRadius);
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		Data.CacheBounds<FVoxelValue>(BoundsToCache);

		if (TickData.bAlternativeMode)
		{
			UVoxelSphereTools::SharpenSphereImpl(
				Data,
				VoxelPosition,
				VoxelRadius,
				SmoothSettings.Strength);
		}
		else
		{
			UVoxelSphereTools::SmoothSphereImpl(
				Data,
				VoxelPosition,
				VoxelRadius,
				SmoothSettings.Strength);
		}
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Sphere::FVoxelToolManagerTool_Sphere(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, SphereToolSettings(ToolManager.SphereSettings))
	, SphereSettings(ToolManager.SphereSettings)
{
}

void FVoxelToolManagerTool_Sphere::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SphereSettings.SphereMesh)
	{
		FVoxelMessages::Error("VoxelToolManager: Sphere Tool: Invalid SphereMesh!");
	}

	const FVector ToolPosition = GetToolPosition();
	const float Scale = ToolManager.Radius / 50;
	SetToolMeshTransform(TickData.World, FTransform(FQuat::Identity, ToolPosition, FVector(Scale)));

	if (!TickData.bClick)
	{
		return;
	}

	const FVector VoxelPosition = World.GlobalToLocalFloat(ToolPosition);
	const float VoxelRadius = ToolManager.Radius / World.VoxelSize;

	const FIntBox Bounds = UVoxelSphereTools::GetSphereBounds(VoxelPosition, VoxelRadius);
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		Data.CacheBounds<FVoxelValue>(BoundsToCache);

		if (TickData.bAlternativeMode)
		{
			UVoxelSphereTools::RemoveSphereImpl(
				Data,
				VoxelPosition,
				VoxelRadius);
		}
		else
		{
			if (SphereSettings.bPaint)
			{
				TArray<FModifiedVoxelValue> ModifiedVoxels;
				UVoxelSphereTools::AddSphereImpl(
					Data,
					VoxelPosition,
					VoxelRadius,
					ModifiedVoxels);

				TArray<FVoxelMaterialEdit> Materials;
				Materials.Reserve(ModifiedVoxels.Num());
				for (auto& Voxel : ModifiedVoxels)
				{
					if (Voxel.OldValue > 0 && Voxel.NewValue <= 0)
					{
						Materials.Emplace(FVoxelEditBase(Voxel.Position, SphereSettings.PaintStrength), ToolManager.PaintMaterial);
					}
				}
				UVoxelSurfaceTools::EditVoxelMaterialsImpl(Data, Bounds, Materials);

			}
			else
			{
				UVoxelSphereTools::AddSphereImpl(
					Data,
					VoxelPosition,
					VoxelRadius);
			}
		}
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

