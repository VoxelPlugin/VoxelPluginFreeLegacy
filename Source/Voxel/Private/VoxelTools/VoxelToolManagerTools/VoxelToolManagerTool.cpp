// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool.h"
#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerToolsHelpers.h"
#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/VoxelSphereTools.h"
#include "VoxelTools/VoxelProjectionTools.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelHardnessHandler.h"
#include "VoxelTools/VoxelAssetTools.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelDataTools.inl"
#include "VoxelTools/VoxelWorldGeneratorTools.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelMessages.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"
#include "VoxelDebug/VoxelDebugUtilities.h"
#include "VoxelWorld.h"

#include "VoxelData/VoxelData.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

static TAutoConsoleVariable<int32> CVarDebugMaterialUnderCursor(
	TEXT("voxel.tools.DebugMaterialUnderCursor"),
	0,
	TEXT("If true, will show the values of the voxel material under the cursor"),
	ECVF_Default);

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
				1.5f * TickData.World->GetDeltaSeconds(),
				FColor::Yellow,
				FString::Printf(TEXT("Waiting for %d updates"), NumPendingUpdates));
		}
		return;
	}

	{
		const double Now = FPlatformTime::Seconds();
		DeltaTime = float(Now - LastTickTime);
		LastTickTime = Now;
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
			1.5f * GetDeltaTime(),
			0,
			World.VoxelSize / 2);
		DrawDebugDirectionalArrow(
			TickData.World,
			GetToolPosition(),
			GetToolPosition() + GetToolNormal() * World.VoxelSize * 5,
			World.VoxelSize * 5,
			FColor::Blue,
			false,
			1.5f * GetDeltaTime(),
			0,
			World.VoxelSize / 2);
	}
	if (CVarDebugMaterialUnderCursor.GetValueOnGameThread() && Hit.bBlockingHit)
	{
		FVoxelFindClosestNonEmptyVoxelResult Result;
		UVoxelDataTools::FindClosestNonEmptyVoxel(Result, &World, Hit.ImpactPoint);

		if (Result.bSuccess)
		{
			FVoxelMaterial Material;
			UVoxelDataTools::GetMaterial(Material, &World, Result.Position);

			FString Message;
			Message += FString::Printf(TEXT("R: %d; G: %d; B: %d; A: %d\n"), Material.GetR(), Material.GetG(), Material.GetB(), Material.GetA());
			if (VOXEL_MATERIAL_ENABLE_UV0) Message += FString::Printf(TEXT("U0: %d; V0: %d\n"), Material.GetU0(), Material.GetV0());
			if (VOXEL_MATERIAL_ENABLE_UV1) Message += FString::Printf(TEXT("U1: %d; V1: %d\n"), Material.GetU1(), Material.GetV1());
			if (VOXEL_MATERIAL_ENABLE_UV2) Message += FString::Printf(TEXT("U2: %d; V2: %d\n"), Material.GetU2(), Material.GetV2());
			if (VOXEL_MATERIAL_ENABLE_UV3) Message += FString::Printf(TEXT("U3: %d; V3: %d\n"), Material.GetU3(), Material.GetV3());
			
			if (World.MaterialConfig == EVoxelMaterialConfig::RGB)
			{
				const TVoxelStaticArray<float, 5> Strengths = FVoxelUtilities::GetFiveWayBlendStrengths(Material);
				
				Message += FString::Printf(TEXT("Blend0: %.4f\n"), Strengths[0]);
				Message += FString::Printf(TEXT("Blend1: %.4f\n"), Strengths[1]);
				Message += FString::Printf(TEXT("Blend2: %.4f\n"), Strengths[2]);
				Message += FString::Printf(TEXT("Blend3: %.4f\n"), Strengths[3]);
				Message += FString::Printf(TEXT("Blend4: %.4f\n"), Strengths[4]);
			}
			else if (World.MaterialConfig == EVoxelMaterialConfig::RGB)
			{
				const TVoxelStaticArray<float, 4> Strengths = FVoxelUtilities::GetFourWayBlendStrengths(Material);
				
				Message += FString::Printf(TEXT("Blend0: %.4f\n"), Strengths[0]);
				Message += FString::Printf(TEXT("Blend1: %.4f\n"), Strengths[1]);
				Message += FString::Printf(TEXT("Blend2: %.4f\n"), Strengths[2]);
				Message += FString::Printf(TEXT("Blend3: %.4f\n"), Strengths[3]);
			}
			else if (World.MaterialConfig == EVoxelMaterialConfig::MultiIndex)
			{
				const TVoxelStaticArray<float, 4> Strengths = FVoxelUtilities::GetMultiIndexStrengths(Material);
				
				Message += FString::Printf(TEXT("Index0: %d; Blend0: %.4f\n"), Material.GetMultiIndex_Index0(), Strengths[0]);
				Message += FString::Printf(TEXT("Index1: %d; Blend1: %.4f\n"), Material.GetMultiIndex_Index1(), Strengths[1]);
				Message += FString::Printf(TEXT("Index2: %d; Blend2: %.4f\n"), Material.GetMultiIndex_Index2(), Strengths[2]);
				Message += FString::Printf(TEXT("Index3: %d; Blend3: %.4f\n"), Material.GetMultiIndex_Index3(), Strengths[3]);
			}
			
			GEngine->AddOnScreenDebugMessage(OBJECT_LINE_ID(), GetDeltaTime() * 1.5f, FColor::Green, Message);
		}
	}

	// Tool material overlay on the voxel world
	if (ToolSettings.Ptr_ToolOverlayMaterial)
	{
		auto& ToolRenderingManager = World.GetToolRenderingManager();
		if (!ToolRenderingId.IsValid() || !ToolRenderingManager.IsValidTool(ToolRenderingId))
		{
			ToolRenderingId = ToolRenderingManager.CreateTool(true);
		}

		auto* const ToolMaterial = *ToolSettings.Ptr_ToolOverlayMaterial;
		if (!ToolMaterial)
		{
			FVoxelMessages::Error("VoxelToolManager: " + ToolSettings.ToolName.ToString() + ": Invalid ToolMaterial!");
			return;
		}
		if (!ToolOverlayMaterialInstance || ToolOverlayMaterialInstance->Parent != ToolMaterial)
		{
			ToolOverlayMaterialInstance = UMaterialInstanceDynamic::Create(ToolMaterial, GetTransientPackage());
			if (!ensure(ToolOverlayMaterialInstance)) return;
		}
		check(ToolOverlayMaterialInstance);
		
		ToolRenderingManager.EditTool(ToolRenderingId, [&](FVoxelToolRendering& Tool)
		{
			if (!Tool.Material.IsValid() || Tool.Material->GetMaterial() != ToolOverlayMaterialInstance) 
			{
				Tool.Material = FVoxelMaterialInterfaceManager::Get().CreateMaterial(ToolOverlayMaterialInstance);
			}
		});
	}
	if (ToolSettings.Ptr_ToolMeshMaterial)
	{
		auto* const ToolMaterial = *ToolSettings.Ptr_ToolMeshMaterial;
		if (!ToolMaterial)
		{
			FVoxelMessages::Error("VoxelToolManager: " + ToolSettings.ToolName.ToString() + ": Invalid ToolMaterial!");
			return;
		}
		if (!ToolMeshMaterialInstance || ToolMeshMaterialInstance->Parent != ToolMaterial)
		{
			ToolMeshMaterialInstance = UMaterialInstanceDynamic::Create(ToolMaterial, GetTransientPackage());
			if (!ensure(ToolMeshMaterialInstance)) return;
		}
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

UMaterialInstanceDynamic& FVoxelToolManagerTool::GetToolOverlayMaterialInstance() const
{
	check(ToolSettings.Ptr_ToolOverlayMaterial);
	check(ToolOverlayMaterialInstance);
	return *ToolOverlayMaterialInstance;
}

UMaterialInstanceDynamic& FVoxelToolManagerTool::GetToolMeshMaterialInstance() const
{
	check(ToolSettings.Ptr_ToolMeshMaterial);
	check(ToolMeshMaterialInstance);
	return *ToolMeshMaterialInstance;
}

void FVoxelToolManagerTool::UpdateWorld(AVoxelWorld& World, const FVoxelIntBox& Bounds)
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
		FVoxelUtilities::MakeVoxelWeakPtrDelegate<void, FVoxelIntBox>(this, 
		[](FVoxelToolManagerTool& This, const FVoxelIntBox& /*ChunkBounds*/)
		{
			ensure(This.NumPendingUpdates-- >= 0);
		}));
}

void FVoxelToolManagerTool::SaveFrameOnEndClick(const FVoxelIntBox& Bounds)
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

FVoxelIntBox FVoxelToolManagerTool::GetAndDebugBoundsToCache(AVoxelWorld& World, const FVoxelIntBox& Bounds, const FVoxelToolManagerTickData& TickData) const
{
	const auto BoundsToCache = UVoxelBlueprintLibrary::GetRenderBoundsOverlappingDataBounds(&World, Bounds);

	if (ToolManager.bDebug)
	{
		UVoxelDebugUtilities::DrawDebugIntBox(&World, Bounds, 1.5f * GetDeltaTime(), 0, FColor::Green);
		UVoxelDebugUtilities::DrawDebugIntBox(&World, BoundsToCache, 1.5f * GetDeltaTime(), 0, FColor::Red);
	}
	
	return BoundsToCache;
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelToolManagerTool::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ToolOverlayMaterialInstance);
	Collector.AddReferencedObject(ToolMeshMaterialInstance);
	Collector.AddReferencedObject(PlaneMeshMaterialInstance);
}