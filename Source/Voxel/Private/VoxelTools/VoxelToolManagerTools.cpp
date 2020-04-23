// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManagerTools.h"
#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/VoxelSphereTools.h"
#include "VoxelTools/VoxelProjectionTools.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelHardnessHandler.h"
#include "VoxelTools/VoxelAssetTools.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelDataTools.inl"
#include "VoxelTools/VoxelWorldGeneratorTools.h"
#include "VoxelTools/VoxelToolManagerToolsHelpers.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelMessages.h"
#include "VoxelThreadingUtilities.h"
#include "VoxelDebug/VoxelDebugUtilities.h"
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

	if (ToolManager.bWaitForUpdates && NumPendingUpdates > 0)
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

	// Update position/normal to the hit ones
	if (Hit.bBlockingHit)
	{
		CurrentPosition = Hit.ImpactPoint;
		CurrentNormal = Hit.ImpactNormal;
	}

	// Viewport-aligned movement
	if (ToolSettings.bViewportSpaceMovement)
	{
		bool bShowPlane = false;
		if (*ToolSettings.Ptr_Alignment == EVoxelToolManagerAlignment::Surface)
		{
			UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(TickData.bClick);

			// No need to override position/normal
		}
		else
		{
			if (!LastFrameTickData.bClick)
			{
				FVector Normal = FVector::UpVector;
				switch (*ToolSettings.Ptr_Alignment)
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
						ensure(TickData.CameraViewDirection.GetAbs().Equals(FVector::UpVector));
						Normal = FVector::RightVector;
					}
					break;
				default: ensure(false);
				}

				const bool bAirMode = !Hit.bBlockingHit || *ToolSettings.Ptr_bAirMode;
				const FVector Point = bAirMode
					? TickData.RayOrigin + TickData.RayDirection * *ToolSettings.Ptr_DistanceToCamera
					: FVector(Hit.ImpactPoint);
				ViewportSpaceMovement.LastClickPlane = FPlane(Point, Normal);
				ViewportSpaceMovement.LastClickPoint = Point;
				ViewportSpaceMovement.LastClickNormal = bAirMode ? FVector::UpVector : Hit.ImpactNormal;
			}

			UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(false);
			
			// Override position/normal
			CurrentPosition = FMath::RayPlaneIntersection(TickData.RayOrigin, TickData.RayDirection, ViewportSpaceMovement.LastClickPlane);
			CurrentNormal = ViewportSpaceMovement.LastClickNormal;

			if (TickData.bClick)
			{
				bShowPlane = true;
			}
		}

		static const FName PlaneMeshId = "ViewportMovementPlane";
		if (bShowPlane && ToolSettings.Ptr_bShowPlanePreview && *ToolSettings.Ptr_bShowPlanePreview)
		{
			const FVector PlaneNormal = FVector(ViewportSpaceMovement.LastClickPlane);
			const FTransform Transform(
				FRotationMatrix::MakeFromZ(PlaneNormal).ToQuat(),
				ViewportSpaceMovement.LastClickPoint + PlaneNormal * 0.5f, // Prevent Z fighting
				FVector(10000.f));

			if (!PlaneMeshMaterialInstance)
			{
				PlaneMeshMaterialInstance = UMaterialInstanceDynamic::Create(ToolManager.PlaneMaterial, GetTransientPackage());
			}
			if (!ensure(PlaneMeshMaterialInstance))
			{
				return;
			}
			PlaneMeshMaterialInstance->SetScalarParameterValue(STATIC_FNAME("VoxelSize"), World.VoxelSize);
			
			UpdateToolMesh(
				TickData.World,
				ToolManager.PlaneMesh,
				PlaneMeshMaterialInstance,
				Transform,
				PlaneMeshId);
		}
		else
		{
			UpdateToolMesh(TickData.World, nullptr, nullptr, {}, PlaneMeshId);
		}
	}

	// Movement direction
	{
		const FVector NewMovementTangent = (CurrentPosition - LastPositionUsedForTangent).GetSafeNormal();
		MovementTangent = FMath::Lerp(
			MovementTangent,
			NewMovementTangent,
			FMath::Clamp((1 - ToolManager.AlignToMovementSmoothness) * GetMouseMovementSize() / 10, 0.f, 1.f)).GetSafeNormal();
		LastPositionUsedForTangent = CurrentPosition;
	}

	// Fixed normal
	if (ToolSettings.Ptr_bFixedNormal && *ToolSettings.Ptr_bFixedNormal)
	{
		CurrentNormal = ToolSettings.FixedNormal;
	}

	// Stride
	if (!ToolSettings.Ptr_Stride ||
		!LastFrameTickData.bClick || // If not clicking always keep the position under the cursor
		FVector::Dist(CurrentPosition, StridePosition) >= *ToolSettings.Ptr_Stride * 2 * ToolManager.Radius) // 2x : we want the diameter
	{
		StridePosition = CurrentPosition;
		StrideNormal = CurrentNormal;
		StrideDirection = MovementTangent;
		
		bCanEdit = TickData.bClick;

		if (!ToolSettings.bViewportSpaceMovement)
		{
			// When we can edit, we flush the collisions and freeze them again
			// This is so that when we cannot edit, we do the raycasts on the old geometry
			// and the tool travel distance isn't artificially done by the edits
			// Without that, you can get a continuous stream of edits when keeping click pressed
			// without moving the mouse, which is unexpected when using stride
			// Not needed in viewport space movement
			UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(false);
			UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(true);
		}
	}
	else
	{
		bCanEdit = false;
	}

	// End of click
	if (!TickData.bClick && PendingFrameBounds.IsValid())
	{
		ToolManager.SaveFrame(World, PendingFrameBounds.GetBox(), ToolSettings.ToolName);


		if (ToolManager.bCheckForSingleValues)
		{
			UVoxelDataTools::CheckForSingleValues(&World, PendingFrameBounds.GetBox());
			UVoxelDataTools::CheckForSingleMaterials(&World, PendingFrameBounds.GetBox());
		}

		if (ToolManager.bDebug)
		{
			UVoxelDebugUtilities::DrawDebugIntBox(&World, PendingFrameBounds.GetBox(), 0.5f, 0, FColor::Purple);
		}
		
		PendingFrameBounds.Reset();
	}

	// Debug
	if (ToolManager.bDebug)
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

	// Tool material overlay on the voxel world
	if (ToolSettings.Ptr_ToolMaterial)
	{
		auto& ToolRenderingManager = World.GetToolRenderingManager();
		if (!ToolRenderingId.IsValid() || !ToolRenderingManager.IsValidTool(ToolRenderingId))
		{
			ToolRenderingId = ToolRenderingManager.CreateTool(true);
		}

		auto* const ToolMaterial = *ToolSettings.Ptr_ToolMaterial;
		if (!ToolMaterial)
		{
			FVoxelMessages::Error("VoxelToolManager: " + ToolSettings.ToolName.ToString() + ": Invalid ToolMaterial!");
			return;
		}
		if (!ToolMaterialInstance || ToolMaterialInstance->Parent != ToolMaterial)
		{
			ToolMaterialInstance = UMaterialInstanceDynamic::Create(ToolMaterial, GetTransientPackage());
			if (!ensure(ToolMaterialInstance)) return;
		}
		check(ToolMaterialInstance);
		
		ToolRenderingManager.EditTool(ToolRenderingId, [&](FVoxelToolRendering& Tool)
		{
			if (!Tool.Material.IsValid() || Tool.Material->GetMaterial() != ToolMaterialInstance) 
			{
				Tool.Material = FVoxelMaterialInterfaceManager::Get().CreateMaterial(ToolMaterialInstance);
			}
		});
	}

	Tick(World, TickData);

	LastFrameTickData = TickData;
	bLastFrameCanEdit = bCanEdit;
}

void FVoxelToolManagerTool::ClearVoxelWorld()
{
	VOXEL_FUNCTION_COUNTER();
	
	if (VoxelWorld.IsValid() && VoxelWorld->IsCreated())
	{
		if (PendingFrameBounds.IsValid())
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

	for (auto& It : StaticMeshActors)
	{
		auto& StaticMeshActor = It.Value;
		if (StaticMeshActor.IsValid())
		{
			StaticMeshActor->Destroy();
		}
	}
	StaticMeshActors.Empty();

	UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(false);
}

///////////////////////////////////////////////////////////////////////////////

FVector FVoxelToolManagerTool::GetToolPosition() const
{
	return StridePosition;
}

FVector FVoxelToolManagerTool::GetToolPreviewPosition() const
{
	// Ignore stride for preview
	return CurrentPosition;
}

FVector FVoxelToolManagerTool::GetToolNormal() const
{
	return StrideNormal;
}

FVector FVoxelToolManagerTool::GetToolDirection() const
{
	if (!ToolSettings.Ptr_bAlignToMovement || *ToolSettings.Ptr_bAlignToMovement)
	{
		return StrideDirection;
	}
	else
	{
		check(ToolSettings.Ptr_Direction);
		return ToolSettings.Ptr_Direction->Vector();
	}
}

UMaterialInstanceDynamic& FVoxelToolManagerTool::GetToolMaterialInstance() const
{
	check(ToolSettings.Ptr_ToolMaterial);
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
		[](FVoxelToolManagerTool& This, const FIntBox& /*ChunkBounds*/)
		{
			ensure(This.NumPendingUpdates-- >= 0);
		}));
}

void FVoxelToolManagerTool::SaveFrameOnEndClick(const FIntBox& Bounds)
{
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

void FVoxelToolManagerTool::UpdateToolMesh(
	UWorld* World, 
	UStaticMesh* Mesh, 
	UMaterialInterface* Material, 
	const FTransform& Transform, 
	FName Id)
{
	VOXEL_FUNCTION_COUNTER();

	auto& StaticMeshActor = StaticMeshActors.FindOrAdd(Id);
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
		StaticMeshActor->SetActorEnableCollision(false);
		StaticMeshActor->FinishSpawning(Transform);
	}
	
	if (!ensure(StaticMeshActor.IsValid()))
	{
		return;
	}
	
	UStaticMeshComponent* MeshComponent = StaticMeshActor->GetStaticMeshComponent();
	if (!ensure(MeshComponent))
	{
		return;
	}

	if (MeshComponent->GetStaticMesh() != Mesh)
	{
		MeshComponent->SetStaticMesh(Mesh);
		for (int32 Index = 0; Index < MeshComponent->GetNumMaterials(); Index++)
		{
			MeshComponent->SetMaterial(Index, Material);
		}
	}
	
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
	Collector.AddReferencedObject(PlaneMeshMaterialInstance);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline FVoxelToolManagerTool::FToolSettings SurfaceToolSettings(const FVoxelToolManager_SurfaceSettings& SurfaceSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Surface Tool");
	
	Settings.Ptr_Stride = &SurfaceSettings.Stride;
	
	Settings.Ptr_ToolMaterial = &SurfaceSettings.ToolMaterial;

	Settings.Ptr_bAlignToMovement = &SurfaceSettings.bAlignToMovement;
	Settings.Ptr_Direction = &SurfaceSettings.FixedDirection;

	Settings.Ptr_bFixedNormal = &SurfaceSettings.b2DBrush;
	Settings.FixedNormal = FVector::UpVector;
	
	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings FlattenToolSettings(const FVoxelToolManager_FlattenSettings& FlattenSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Flatten Tool");
	
	Settings.Ptr_ToolMaterial = &FlattenSettings.ToolMaterial;
	
	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings TrimToolSettings(const FVoxelToolManager_TrimSettings& TrimSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Trim Tool");

	Settings.Ptr_ToolMaterial = &TrimSettings.ToolMaterial;

	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings LevelToolSettings(const FVoxelToolManager_LevelSettings& LevelSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Level Tool");
	
	Settings.Ptr_Stride = &LevelSettings.Stride;

	Settings.bViewportSpaceMovement = true;

	static EVoxelToolManagerAlignment Alignment = EVoxelToolManagerAlignment::Ground;
	static bool bAirMode = false;
	static float DistanceToCamera = 1e5;

	Settings.Ptr_Alignment = &Alignment;
	Settings.Ptr_bAirMode = &bAirMode;
	Settings.Ptr_DistanceToCamera = &DistanceToCamera;

	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings SmoothToolSettings(const FVoxelToolManager_SmoothSettings& SmoothSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Smooth Tool");

	Settings.Ptr_ToolMaterial = &SmoothSettings.ToolMaterial;

	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings SphereToolSettings(const FVoxelToolManager_SphereSettings& SphereSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Sphere Tool");

	Settings.bViewportSpaceMovement = true;
	Settings.Ptr_bShowPlanePreview = &SphereSettings.bShowPlanePreview;
	Settings.Ptr_Alignment = &SphereSettings.Alignment;
	Settings.Ptr_bAirMode = &SphereSettings.bAirMode;
	Settings.Ptr_DistanceToCamera = &SphereSettings.DistanceToCamera;
	
	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings MeshToolSettings(const FVoxelToolManager_MeshSettings& MeshSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Mesh Tool");
	
	Settings.Ptr_Stride = &MeshSettings.Stride;

	Settings.bViewportSpaceMovement = true;
	Settings.Ptr_bShowPlanePreview = &MeshSettings.bShowPlanePreview;
	Settings.Ptr_Alignment = &MeshSettings.Alignment;
	Settings.Ptr_bAirMode = &MeshSettings.bAirMode;
	Settings.Ptr_DistanceToCamera = &MeshSettings.DistanceToCamera;
	
	return Settings;
}

inline FVoxelToolManagerTool::FToolSettings RevertToolSettings(const FVoxelToolManager_RevertSettings& RevertSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Revert Tool");

	Settings.bViewportSpaceMovement = true;
	Settings.Ptr_bShowPlanePreview = &RevertSettings.bShowPlanePreview;
	Settings.Ptr_Alignment = &RevertSettings.Alignment;
	Settings.Ptr_bAirMode = &RevertSettings.bAirMode;
	Settings.Ptr_DistanceToCamera = &RevertSettings.DistanceToCamera;
	
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
	
	auto& MaterialInstance = GetToolMaterialInstance();

	struct FMaskData
	{
		TVoxelTexture<float> MaskTexture;
		UTexture2D* MaskRenderTexture = nullptr;
		float MaskScaleX = 0;
		float MaskScaleY = 0;
		FVector MaskPlaneX;
		FVector MaskPlaneY;
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

	const float MovementStrengthMultiplier = SurfaceSettings.bMovementAffectsStrength ? GetMouseMovementSize() / 100 : 1;
	const float SculptStrength = SurfaceSettings.SculptStrength * MovementStrengthMultiplier * ToolManager.Radius / World.VoxelSize;
	const float PaintStrength = SurfaceSettings.PaintStrength * MovementStrengthMultiplier;

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

	SetToolRenderingBounds(World, FBox(GetToolPreviewPosition() - ToolManager.Radius, GetToolPreviewPosition() + ToolManager.Radius));

	if (!CanEdit())
	{
		return;
	}

	constexpr float DistanceDivisor = 4.f;

	const FIntBox BoundsToDoEditsIn = UVoxelBlueprintLibrary::MakeIntBoxFromGlobalPositionAndRadius(&World, GetToolPosition(), ToolManager.Radius);
	const FIntBox BoundsWhereEditsHappen =
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
		
		TArray<FSurfaceVoxel> Voxels;
		if (SurfaceSettings.b2DBrush)
		{
			UVoxelSurfaceTools::FindSurfaceVoxels2DImpl<false>(Data, BoundsToDoEditsIn, Voxels);
		}
		else
		{
			if (SurfaceSettings.bSculpt)
			{
				UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceFieldImpl(Data, BoundsToDoEditsIn, SculptStrength + 3, true, Voxels);
			}
			else
			{
				// No need to compute the distance field for paint
				// Only select voxels inside the surface
				UVoxelSurfaceTools::FindSurfaceVoxelsImpl<false, true>(Data, BoundsToDoEditsIn, Voxels);
			}
		}
		
		TArray<FSurfaceVoxelWithStrength> Strengths;
		{
			Strengths = UVoxelSurfaceTools::AddStrengthToSurfaceVoxels(Voxels);
		
			const FVoxelVector ToolVoxelPosition = World.GlobalToLocalFloat(GetToolPosition());

			if (SurfaceSettings.bEnableFalloff)
			{
				const auto ApplyFalloff = [&](auto F)
				{
					if (SurfaceSettings.b2DBrush)
					{
						UVoxelSurfaceTools::ApplyStrengthFunctionImpl<true>(
							Strengths,
							ToolVoxelPosition,
							F);
					}
					else
					{
						UVoxelSurfaceTools::ApplyStrengthFunctionImpl<false>(
							Strengths,
							ToolVoxelPosition,
							F);
					}
				};

				FVoxelToolManagerToolsHelpers::DispatchApplyFalloff(
					SurfaceSettings.FalloffType,
					ToolManager.Radius / World.VoxelSize,
					SurfaceSettings.Falloff,
					ApplyFalloff);
			}

			if (bUseMask)
			{
			}
		}
		
		if (SurfaceSettings.bSculpt && SurfaceSettings.bPaint)
		{
			auto SculptStrengths = Strengths;
			UVoxelSurfaceTools::ApplyConstantStrengthImpl(SculptStrengths, -SignedSculptStrength);

			TArray<FModifiedVoxelValue> ModifiedVoxels;
			const auto ValueEdits = UVoxelSurfaceTools::CreateValueEditsFromSurfaceVoxels(SculptStrengths);
			if (SurfaceSettings.b2DBrush)
			{
				UVoxelSurfaceTools::EditVoxelValues2DImpl(Data, BoundsWhereEditsHappen, ValueEdits, FVoxelHardnessHandler(World), DistanceDivisor);
			}
			else
			{
				UVoxelSurfaceTools::EditVoxelValuesImpl(Data, BoundsWhereEditsHappen, ValueEdits, FVoxelHardnessHandler(World), DistanceDivisor);
			}
			
			TArray<FVoxelMaterialEdit> Materials;
			Materials.Reserve(ModifiedVoxels.Num());
			for (auto& Voxel : ModifiedVoxels)
			{
				if (Voxel.OldValue > 0 && Voxel.NewValue <= 0)
				{
					Materials.Emplace(FVoxelEditBase(Voxel.Position, SurfaceSettings.PaintStrength), ToolManager.PaintMaterial);
				}
			}
			UVoxelSurfaceTools::EditVoxelMaterialsImpl(Data, BoundsWhereEditsHappen, Materials);
		}
		else if (SurfaceSettings.bSculpt)
		{
			auto SculptStrengths = Strengths;
			UVoxelSurfaceTools::ApplyConstantStrengthImpl(SculptStrengths, -SignedSculptStrength);
			
			const auto ValueEdits = UVoxelSurfaceTools::CreateValueEditsFromSurfaceVoxels(SculptStrengths);
			
			if (SurfaceSettings.b2DBrush)
			{
				UVoxelSurfaceTools::EditVoxelValues2DImpl(Data, BoundsWhereEditsHappen, ValueEdits, FVoxelHardnessHandler(World), DistanceDivisor);
			}
			else
			{
				UVoxelSurfaceTools::EditVoxelValuesImpl(Data, BoundsWhereEditsHappen, ValueEdits, FVoxelHardnessHandler(World), DistanceDivisor);
			}
		}
		else if (SurfaceSettings.bPaint)
		{
			// Note: Painting behaves the same with 2D edit on/off
			auto MaterialStrengths = Strengths;
			UVoxelSurfaceTools::ApplyConstantStrengthImpl(MaterialStrengths, SignedPaintStrength);
			const auto MaterialEdits = UVoxelSurfaceTools::CreateMaterialEditsFromSurfaceVoxels(MaterialStrengths, ToolManager.PaintMaterial);
			UVoxelSurfaceTools::EditVoxelMaterialsImpl(Data, BoundsWhereEditsHappen, MaterialEdits);
		}

		if (ToolManager.bDebug)
		{
			UVoxelSurfaceTools::DebugSurfaceVoxels(&World, Strengths, SurfaceSettings.Stride > 0 ? 1 : 2 * TickData.DeltaTime);
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Flatten::FVoxelToolManagerTool_Flatten(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, FlattenToolSettings(ToolManager.FlattenSettings))
	, FlattenSettings(ToolManager.FlattenSettings)
{
}

void FVoxelToolManagerTool_Flatten::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();

	auto& MaterialInstance = GetToolMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Radius"), ToolManager.Radius);
	MaterialInstance.SetVectorParameterValue(STATIC_FNAME("Position"), GetToolPreviewPosition());
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Falloff"), FlattenSettings.Falloff);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("EnableFalloff"), FlattenSettings.bEnableFalloff);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(FlattenSettings.FalloffType));

	SetToolRenderingBounds(World, FBox(GetToolPreviewPosition() - ToolManager.Radius, GetToolPreviewPosition() + ToolManager.Radius));

	if (!CanEdit())
	{
		return;
	}

	constexpr float DistanceDivisor = 4.f;

	const FIntBox Bounds = UVoxelBlueprintLibrary::MakeIntBoxFromGlobalPositionAndRadius(&World, GetToolPosition(), ToolManager.Radius);
	if (!Bounds.IsValid())
	{
		FVoxelMessages::Error("Invalid tool bounds!", &ToolManager);
		return;
	}
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	FVector FlattenPosition;
	FVector FlattenNormal;
	if (FlattenSettings.bUseAverage)
	{
		FVoxelLineTraceParameters Parameters;
		Parameters.CollisionChannel = World.CollisionPresets.GetObjectType();
		Parameters.DrawDebugType = ToolManager.bDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

		const float RaysRadius = FMath::Max(ToolManager.Radius, World.VoxelSize);

		TArray<FVoxelProjectionHit> Hits;
		UVoxelProjectionTools::FindProjectionVoxels(
			Hits,
			&World,
			Parameters,
			GetToolPosition() - TickData.RayDirection * ToolManager.Radius,
			TickData.RayDirection,
			RaysRadius,
			EVoxelProjectionShape::Circle,
			100.f,
			2 * RaysRadius);

		FlattenPosition = UVoxelProjectionTools::GetHitsAveragePosition(Hits);
		FlattenNormal = UVoxelProjectionTools::GetHitsAverageNormal(Hits);
	}
	else
	{
		FlattenPosition = GetToolPosition();
		FlattenNormal = GetToolNormal();
	}

	if (!GetLastFrameTickData().bClick)
	{
		LastClickFlattenPosition = FlattenPosition;
		LastClickFlattenNormal = FlattenNormal;
	}

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		if (ToolManager.bCacheData) Data.CacheBounds<FVoxelValue>(BoundsToCache);
		
		TArray<FSurfaceVoxel> Voxels;
		UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceFieldImpl(Data, Bounds, FlattenSettings.Strength + 3, true, Voxels);
		
		const FVoxelVector ToolVoxelPosition = World.GlobalToLocalFloat(GetToolPosition());

		TArray<FSurfaceVoxelWithStrength> Strengths = UVoxelSurfaceTools::AddStrengthToSurfaceVoxels(Voxels);

		if (FlattenSettings.bEnableFalloff)
		{
			const auto ApplyFalloff = [&](auto F)
			{
				UVoxelSurfaceTools::ApplyStrengthFunctionImpl<false>(
					Strengths,
					ToolVoxelPosition,
					F);
			};

			FVoxelToolManagerToolsHelpers::DispatchApplyFalloff(
				FlattenSettings.FalloffType,
				ToolManager.Radius / World.VoxelSize,
				FlattenSettings.Falloff,
				ApplyFalloff);
		}

		UVoxelSurfaceTools::ApplyConstantStrengthImpl(Strengths, FlattenSettings.Strength);

		const FVector PlanePosition = FlattenSettings.bFreezeOnClick ? LastClickFlattenPosition : FlattenPosition;
		const FVector PlaneNormal = FlattenSettings.bFreezeOnClick ? LastClickFlattenNormal : FlattenNormal;

		const FPlane Plane(
			World.GlobalToLocalFloat(PlanePosition).ToFloat(),
			World.GetActorTransform().InverseTransformVector(PlaneNormal).GetSafeNormal());
		
		UVoxelSurfaceTools::ApplyFlattenImpl(
			Strengths,
			Plane,
			TickData.bAlternativeMode ? EVoxelSDFMergeMode::Intersection : EVoxelSDFMergeMode::Union,
			true);

		const auto ValueEdits = UVoxelSurfaceTools::CreateValueEditsFromSurfaceVoxels(Strengths);
		UVoxelSurfaceTools::EditVoxelValuesImpl(Data, Bounds, ValueEdits, FVoxelHardnessHandler(World), DistanceDivisor);

		if (ToolManager.bDebug)
		{
			UVoxelSurfaceTools::DebugSurfaceVoxels(&World, Strengths, 2 * TickData.DeltaTime);

			DrawDebugSolidPlane(
				TickData.World,
				FPlane(PlanePosition, PlaneNormal),
				PlanePosition,
				1000000,
				FColor::Red,
				false,
				1.5f * TickData.DeltaTime);
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
	
	FVoxelLineTraceParameters Parameters;
	Parameters.CollisionChannel = World.CollisionPresets.GetObjectType();
	Parameters.DrawDebugType = ToolManager.bDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	TArray<FVoxelProjectionHit> Hits;
	const float RaysRadius = FMath::Max(ToolManager.Radius * (1 - TrimSettings.Roughness), World.VoxelSize);
	UVoxelProjectionTools::FindProjectionVoxels(
		Hits,
		&World,
		Parameters,
		GetToolPosition() - TickData.RayDirection * ToolManager.Radius,
		TickData.RayDirection,
		RaysRadius,
		EVoxelProjectionShape::Circle,
		100.f,
		2 * RaysRadius);

	const FVector Position = UVoxelProjectionTools::GetHitsAveragePosition(Hits);

	SetToolRenderingBounds(World, FBox(Position - ToolManager.Radius, Position + ToolManager.Radius));

	auto& MaterialInstance = GetToolMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Radius"), ToolManager.Radius);
	MaterialInstance.SetVectorParameterValue(STATIC_FNAME("Position"), Position);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Falloff"), TrimSettings.Falloff);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Roughness"), TrimSettings.Roughness);

	if (!CanEdit())
	{
		return;
	}

	const FVector Normal = UVoxelProjectionTools::GetHitsAverageNormal(Hits);
	
	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(Position);
	const float VoxelRadius = ToolManager.Radius / World.VoxelSize;

	const FIntBox Bounds = UVoxelSphereTools::GetSphereBounds(VoxelPosition, VoxelRadius);
	if (!Bounds.IsValid())
	{
		FVoxelMessages::Error("Invalid tool bounds!", &ToolManager);
		return;
	}
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		if (ToolManager.bCacheData) Data.CacheBounds<FVoxelValue>(BoundsToCache);

		UVoxelSphereTools::TrimSphereImpl(
			Data,
			VoxelPosition,
			Normal,
			VoxelRadius * (1 - TrimSettings.Falloff),
			VoxelRadius * TrimSettings.Falloff,
			!TickData.bAlternativeMode);
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Level::FVoxelToolManagerTool_Level(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, LevelToolSettings(ToolManager.LevelSettings))
	, LevelSettings(ToolManager.LevelSettings)
{
}

void FVoxelToolManagerTool_Level::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();

	const FVector ToolOffset = FVector(0.f, 0.f,  LevelSettings.Offset * LevelSettings.Height * (TickData.bAlternativeMode ? -1 : 1));
	
	const float ScaleXY = ToolManager.Radius / 50.f;
	const float ScaleZ = LevelSettings.Height / 100.f + 0.001f;
	const FTransform PreviewTransform(
		FQuat::Identity,
		GetToolPreviewPosition() + ToolOffset + FVector(0, 0, (TickData.bAlternativeMode ? LevelSettings.Height : -LevelSettings.Height) / 2),
		FVector(ScaleXY, ScaleXY, ScaleZ));

	UpdateToolMesh(
		TickData.World,
		LevelSettings.CylinderMesh,
		LevelSettings.ToolMaterial,
		PreviewTransform);

	if (!CanEdit())
	{
		return;
	}

	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition() + ToolOffset);
	const float VoxelRadius = ToolManager.Radius / World.VoxelSize;
	const float VoxelHeight = LevelSettings.Height / World.VoxelSize;

	const FIntBox Bounds = UVoxelDataTools::GetLevelToolBounds(VoxelPosition, VoxelRadius, VoxelHeight, !TickData.bAlternativeMode);
	if (!Bounds.IsValid())
	{
		FVoxelMessages::Error("Invalid tool bounds!", &ToolManager);
		return;
	}
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		if (ToolManager.bCacheData) Data.CacheBounds<FVoxelValue>(BoundsToCache);

		UVoxelDataTools::LevelImpl(
			Data,
			VoxelPosition,
			VoxelRadius,
			LevelSettings.Falloff,
			VoxelHeight,
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
	
	SetToolRenderingBounds(World, FBox(GetToolPreviewPosition() - ToolManager.Radius, GetToolPreviewPosition() + ToolManager.Radius));

	auto& MaterialInstance = GetToolMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Radius"), ToolManager.Radius);
	MaterialInstance.SetVectorParameterValue(STATIC_FNAME("Position"), GetToolPreviewPosition());

	if (!CanEdit())
	{
		return;
	}

	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition());
	const float VoxelRadius = ToolManager.Radius / World.VoxelSize;

	const FIntBox Bounds = UVoxelSphereTools::GetSphereBounds(VoxelPosition, VoxelRadius);
	if (!Bounds.IsValid())
	{
		FVoxelMessages::Error("Invalid tool bounds!", &ToolManager);
		return;
	}
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		if (ToolManager.bCacheData) Data.CacheBounds<FVoxelValue>(BoundsToCache);

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

	const float Scale = ToolManager.Radius / 50;
	const FTransform PreviewTransform(FQuat::Identity, GetToolPreviewPosition(), FVector(Scale));

	UpdateToolMesh(
		TickData.World,
		SphereSettings.SphereMesh,
		SphereSettings.ToolMaterial,
		PreviewTransform);

	if (!CanEdit())
	{
		return;
	}

	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition());
	const float VoxelRadius = ToolManager.Radius / World.VoxelSize;

	const FIntBox Bounds = UVoxelSphereTools::GetSphereBounds(VoxelPosition, VoxelRadius);
	if (!Bounds.IsValid())
	{
		FVoxelMessages::Error("Invalid tool bounds!", &ToolManager);
		return;
	}
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		if (ToolManager.bCacheData) Data.CacheBounds<FVoxelValue>(BoundsToCache);

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


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Revert::FVoxelToolManagerTool_Revert(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, RevertToolSettings(ToolManager.RevertSettings))
	, RevertSettings(ToolManager.RevertSettings)
{
}

void FVoxelToolManagerTool_Revert::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();

	const_cast<FVoxelToolManager_RevertSettings&>(RevertSettings).CurrentHistoryPosition = World.GetData().GetHistoryPosition();
	const_cast<FVoxelToolManager_RevertSettings&>(RevertSettings).HistoryPosition = FMath::Clamp(RevertSettings.HistoryPosition, 0, RevertSettings.CurrentHistoryPosition);
	
	if (!RevertSettings.SphereMesh)
	{
		FVoxelMessages::Error("VoxelToolManager: Revert Tool: Invalid SphereMesh!");
	}

	const float Scale = ToolManager.Radius / 50;
	const FTransform PreviewTransform(FQuat::Identity, GetToolPreviewPosition(), FVector(Scale));

	UpdateToolMesh(
		TickData.World,
		RevertSettings.SphereMesh,
		RevertSettings.ToolMaterial,
		PreviewTransform);

	if (!CanEdit())
	{
		return;
	}

	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition());
	const float VoxelRadius = ToolManager.Radius / World.VoxelSize;

	const FIntBox Bounds = UVoxelSphereTools::GetSphereBounds(VoxelPosition, VoxelRadius);
	if (!Bounds.IsValid())
	{
		FVoxelMessages::Error("Invalid tool bounds!", &ToolManager);
		return;
	}
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		if (ToolManager.bCacheData) Data.CacheBounds<FVoxelValue>(BoundsToCache);

		if (TickData.bAlternativeMode)
		{
			UVoxelSphereTools::RevertSphereToGeneratorImpl(
				Data,
				VoxelPosition,
				VoxelRadius,
				RevertSettings.bRevertValues,
				RevertSettings.bRevertMaterials);
		}
		else
		{
			UVoxelSphereTools::RevertSphereImpl(
				Data,
				VoxelPosition,
				VoxelRadius,
				RevertSettings.HistoryPosition,
				RevertSettings.bRevertValues,
				RevertSettings.bRevertMaterials);
		}
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}