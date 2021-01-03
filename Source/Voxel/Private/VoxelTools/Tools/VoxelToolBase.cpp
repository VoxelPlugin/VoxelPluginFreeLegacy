// Copyright 2021 Phyronnaz

#include "VoxelTools/Tools/VoxelToolBase.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelDebug/VoxelDebugUtilities.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelRender/IVoxelLODManager.h"

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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelToolBase::Tick()
{
	SharedConfig->BrushSize = GetValueAfterAxisInput(FVoxelToolAxes::BrushSize, SharedConfig->BrushSize, 0, 20000);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelToolBase::EnableTool()
{
	Super::EnableTool();
	
	ensure(!VoxelWorld);
}

void UVoxelToolBase::DisableTool()
{
	Super::DisableTool();

	ClearVoxelWorld();
}

void UVoxelToolBase::CallTool(AVoxelWorld* InVoxelWorld, const FVoxelToolTickData& InTickData, const FCallToolParameters& Parameters)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(SharedConfig))
	{
		return;
	}
	
	if (InVoxelWorld && VoxelWorld != InVoxelWorld) 
	{
		ClearVoxelWorld();
		VoxelWorld = InVoxelWorld;
	}

	if (!VoxelWorld || !VoxelWorld->IsCreated())
	{
		return;
	}

	LastFrameTickData = TickData;
	TickData = InTickData;

	K2_GetToolConfig({}, ToolBaseConfig);

	if (Parameters.Mode == ECallToolMode::Apply)
	{
		MouseMovementSize = 1;
	}
	else
	{
		MouseMovementSize = (LastFrameTickData.MousePosition - TickData.MousePosition).Size();
	}

	// Update position/normal to the hit ones
	if (Parameters.Mode == ECallToolMode::Apply || Parameters.bBlockingHit)
	{
		CurrentPosition = Parameters.Position;
		CurrentNormal = Parameters.Normal;
	}

	// Viewport-aligned movement
	if (Parameters.Mode == ECallToolMode::Tick && ToolBaseConfig.bHasAlignment)
	{
		bool bShowPlane = false;
		if (ToolBaseConfig.Alignment == EVoxelToolAlignment::Surface)
		{
			UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(VoxelWorld, TickData.bEdit);

			// No need to override position/normal
		}
		else
		{
			if (!LastFrameTickData.bEdit)
			{
				FVector Normal = FVector::UpVector;
				switch (ToolBaseConfig.Alignment)
				{
				case EVoxelToolAlignment::View:
					Normal = -TickData.CameraViewDirection;
					break;
				case EVoxelToolAlignment::Ground:
					Normal = FVector::UpVector;
					break;
				case EVoxelToolAlignment::Up:
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

				const bool bAirMode = !Parameters.bBlockingHit || ToolBaseConfig.bAirMode;
				const FVector Point = bAirMode
					? TickData.GetRayOrigin() + TickData.GetRayDirection() * ToolBaseConfig.DistanceToCamera
					: FVector(Parameters.Position);
				ViewportSpaceMovement.LastClickPlane = FPlane(Point, Normal);
				ViewportSpaceMovement.LastClickPoint = Point;
				ViewportSpaceMovement.LastClickNormal = bAirMode ? FVector::UpVector : Parameters.Normal;
			}

			UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(VoxelWorld, false);
			
			// Override position/normal
			CurrentPosition = FMath::RayPlaneIntersection(TickData.GetRayOrigin(), TickData.GetRayDirection(), ViewportSpaceMovement.LastClickPlane);
			CurrentNormal = ViewportSpaceMovement.LastClickNormal;

			if (TickData.bEdit)
			{
				bShowPlane = true;
			}
		}

		static const FName PlaneMeshId = "ViewportMovementPlane";
		if (bShowPlane && ToolBaseConfig.bShowPlanePreview && SharedConfig->PlaneMesh && SharedConfig->PlaneMaterial)
		{
			const FVector PlaneNormal = FVector(ViewportSpaceMovement.LastClickPlane);
			const FTransform Transform(
				FRotationMatrix::MakeFromZ(PlaneNormal).ToQuat(),
				ViewportSpaceMovement.LastClickPoint + PlaneNormal * 0.5f, // Prevent Z fighting
				FVector(10000.f));

			if (!PlaneMeshMaterialInstance)
			{
				PlaneMeshMaterialInstance = UMaterialInstanceDynamic::Create(SharedConfig->PlaneMaterial, GetTransientPackage());
			}
			if (!ensure(PlaneMeshMaterialInstance))
			{
				return;
			}
			PlaneMeshMaterialInstance->SetScalarParameterValue(STATIC_FNAME("VoxelSize"), VoxelWorld->VoxelSize);
			
			UpdateToolMesh(
				SharedConfig->PlaneMesh,
				PlaneMeshMaterialInstance,
				Transform,
				PlaneMeshId);
		}
		else
		{
			UpdateToolMesh(nullptr, nullptr, {}, PlaneMeshId);
		}
	}

	// Movement direction
	{
		const FVector NewMovementTangent = (CurrentPosition - LastPositionUsedForTangent).GetSafeNormal();
		MovementTangent = FMath::Lerp(
			MovementTangent,
			NewMovementTangent,
			FMath::Clamp((1 - SharedConfig->AlignToMovementSmoothness) * GetMouseMovementSize() / 10, 0.f, 1.f)).GetSafeNormal();
		LastPositionUsedForTangent = CurrentPosition;
	}

	// Fixed normal
	if (ToolBaseConfig.bUseFixedNormal)
	{
		CurrentNormal = ToolBaseConfig.FixedNormal;
	}

	// Stride
	if (Parameters.Mode == ECallToolMode::Apply ||
		ToolBaseConfig.Stride == 0.f ||
		!LastFrameTickData.bEdit || // If not clicking always keep the position under the cursor
		FVector::Dist(CurrentPosition, StridePosition) >= ToolBaseConfig.Stride * SharedConfig->BrushSize)
	{
		StridePosition = CurrentPosition;
		StrideNormal = CurrentNormal;
		StrideDirection = MovementTangent;
		
		bCanEdit = TickData.bEdit;

		if (Parameters.Mode == ECallToolMode::Tick && !ToolBaseConfig.bHasAlignment)
		{
			// When we can edit, we flush the collisions and freeze them again
			// This is so that when we cannot edit, we do the raycasts on the old geometry
			// and the tool travel distance isn't artificially done by the edits
			// Without that, you can get a continuous stream of edits when keeping click pressed
			// without moving the mouse, which is unexpected when using stride
			// Not needed in viewport space movement
			UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(VoxelWorld, false);
			UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(VoxelWorld, true);
		}
	}
	else
	{
		bCanEdit = false;
	}

	// End of click
	if (!TickData.bEdit)
	{
		ensure(Parameters.Mode == ECallToolMode::Tick);
		ApplyPendingFrameBounds();
	}

	// Debug
	if (SharedConfig->bDebug)
	{
		DrawDebugDirectionalArrow(
			VoxelWorld->GetWorld(),
			GetToolPosition(),
			GetToolPosition() + GetToolDirection() * VoxelWorld->VoxelSize * 5,
			VoxelWorld->VoxelSize * 5,
			FColor::Red,
			false,
			1.5f * GetDeltaTime(),
			0,
			VoxelWorld->VoxelSize / 2);
		DrawDebugDirectionalArrow(
			VoxelWorld->GetWorld(),
			GetToolPosition(),
			GetToolPosition() + GetToolNormal() * VoxelWorld->VoxelSize * 5,
			VoxelWorld->VoxelSize * 5,
			FColor::Blue,
			false,
			1.5f * GetDeltaTime(),
			0,
			VoxelWorld->VoxelSize / 2);
	}
	if (Parameters.Mode == ECallToolMode::Tick && CVarDebugMaterialUnderCursor.GetValueOnGameThread() && Parameters.bBlockingHit)
	{
		FVoxelFindClosestNonEmptyVoxelResult Result;
		UVoxelDataTools::FindClosestNonEmptyVoxel(Result, VoxelWorld, Parameters.Position);

		if (Result.bSuccess)
		{
			FVoxelMaterial Material;
			UVoxelDataTools::GetMaterial(Material, VoxelWorld, Result.Position);

			FString Message;
			Message += FString::Printf(TEXT("R: %d; G: %d; B: %d; A: %d\n"), Material.GetR(), Material.GetG(), Material.GetB(), Material.GetA());
			if (VOXEL_MATERIAL_ENABLE_UV0) Message += FString::Printf(TEXT("U0: %d; V0: %d\n"), Material.GetU0(), Material.GetV0());
			if (VOXEL_MATERIAL_ENABLE_UV1) Message += FString::Printf(TEXT("U1: %d; V1: %d\n"), Material.GetU1(), Material.GetV1());
			if (VOXEL_MATERIAL_ENABLE_UV2) Message += FString::Printf(TEXT("U2: %d; V2: %d\n"), Material.GetU2(), Material.GetV2());
			if (VOXEL_MATERIAL_ENABLE_UV3) Message += FString::Printf(TEXT("U3: %d; V3: %d\n"), Material.GetU3(), Material.GetV3());
			
			if (VoxelWorld->MaterialConfig == EVoxelMaterialConfig::RGB)
			{
				const TVoxelStaticArray<float, 5> Strengths = FVoxelUtilities::GetFiveWayBlendStrengths(Material);
				
				Message += FString::Printf(TEXT("Blend0: %.4f\n"), Strengths[0]);
				Message += FString::Printf(TEXT("Blend1: %.4f\n"), Strengths[1]);
				Message += FString::Printf(TEXT("Blend2: %.4f\n"), Strengths[2]);
				Message += FString::Printf(TEXT("Blend3: %.4f\n"), Strengths[3]);
				Message += FString::Printf(TEXT("Blend4: %.4f\n"), Strengths[4]);
			}
			else if (VoxelWorld->MaterialConfig == EVoxelMaterialConfig::RGB)
			{
				const TVoxelStaticArray<float, 4> Strengths = FVoxelUtilities::GetFourWayBlendStrengths(Material);
				
				Message += FString::Printf(TEXT("Blend0: %.4f\n"), Strengths[0]);
				Message += FString::Printf(TEXT("Blend1: %.4f\n"), Strengths[1]);
				Message += FString::Printf(TEXT("Blend2: %.4f\n"), Strengths[2]);
				Message += FString::Printf(TEXT("Blend3: %.4f\n"), Strengths[3]);
			}
			else if (VoxelWorld->MaterialConfig == EVoxelMaterialConfig::MultiIndex)
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

	if (Parameters.Mode == ECallToolMode::Tick)
	{
		// Tool material overlay on the voxel world
		if (ToolBaseConfig.OverlayMaterial)
		{
			auto& ToolRenderingManager = VoxelWorld->GetSubsystemChecked<FVoxelToolRenderingManager>();
			if (!ToolRenderingId.IsValid() || !ToolRenderingManager.IsValidTool(ToolRenderingId))
			{
				ToolRenderingId = ToolRenderingManager.CreateTool(true);
			}

			if (!ToolOverlayMaterialInstance || ToolOverlayMaterialInstance->Parent != ToolBaseConfig.OverlayMaterial)
			{
				ToolOverlayMaterialInstance = UMaterialInstanceDynamic::Create(ToolBaseConfig.OverlayMaterial, GetTransientPackage());
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
		else if (ToolRenderingId.IsValid())
		{
			// Some tools can enabled/disable overlay
			auto& ToolRenderingManager = VoxelWorld->GetSubsystemChecked<FVoxelToolRenderingManager>();
			ToolRenderingManager.RemoveTool(ToolRenderingId);
			ToolRenderingId.Reset();

			ToolOverlayMaterialInstance = nullptr;
		}
		
		if (ToolBaseConfig.MeshMaterial)
		{
			if (!ToolMeshMaterialInstance || ToolMeshMaterialInstance->Parent != ToolBaseConfig.MeshMaterial)
			{
				ToolMeshMaterialInstance = UMaterialInstanceDynamic::Create(ToolBaseConfig.MeshMaterial, GetTransientPackage());
				if (!ensure(ToolMeshMaterialInstance)) return;
			}
		}
		else
		{
			ToolMeshMaterialInstance = nullptr;
		}
	}

	Tick();
	K2_Tick();

	if (Parameters.Mode == ECallToolMode::Tick)
	{
		K2_UpdateRender(ToolOverlayMaterialInstance, ToolMeshMaterialInstance);
	}
	
	if (Parameters.Mode == ECallToolMode::Tick && SharedConfig->bWaitForUpdates && NumPendingUpdates > 0)
	{
		if (SharedConfig->bDebug)
		{
			GEngine->AddOnScreenDebugMessage(
				OBJECT_LINE_ID(),
				1.5f * VoxelWorld->GetWorld()->GetDeltaSeconds(),
				FColor::Yellow,
				FString::Printf(TEXT("Waiting for %d updates"), NumPendingUpdates));
		}
	}
	else
	{
		if (Parameters.Mode == ECallToolMode::Apply)
		{
			// Can't use LastTickTime in Apply
			DeltaTime = SharedConfig->FixedDeltaTime;
		}
		else
		{
			const double Now = FPlatformTime::Seconds();
			DeltaTime = float(Now - LastTickTime);
			LastTickTime = Now;
		}

		if (bCanEdit)
		{
			FVoxelIntBoxWithValidity ModifiedBounds;

			if (Parameters.DoEditOverride)
			{
				// Do not do the actual edit, just call the override
				Parameters.DoEditOverride(GetToolPosition(), GetToolNormal());
			}
			else
			{
				ModifiedBounds = K2_DoEdit();
			}

			if (ModifiedBounds.IsValid())
			{
				PendingFrameBounds += ModifiedBounds;

				NumPendingUpdates += VoxelWorld->GetSubsystemChecked<IVoxelLODManager>().UpdateBounds_OnAllFinished(
					ModifiedBounds.GetBox(),
					FSimpleDelegate::CreateWeakLambda(this, [this, ModifiedBounds = ModifiedBounds.GetBox(), OldVoxelWorld = VoxelWorld]()
					{
						if (VoxelWorld == OldVoxelWorld)
						{
							SharedConfig->OnBoundsUpdated.Broadcast(VoxelWorld, ModifiedBounds);
							if (SharedConfig->bDebug)
							{
								UVoxelDebugUtilities::DrawDebugIntBox(VoxelWorld, ModifiedBounds, FTransform(), 0.1f);
							}
						}
					}),
					FVoxelOnChunkUpdateFinished::FDelegate::CreateWeakLambda(this, [this](const FVoxelIntBox& /*ChunkBounds*/)
					{
						NumPendingUpdates--;
						ensure(NumPendingUpdates >= 0);
					}));
			}
		}

		// Only copy it when we're not waiting for tasks, as that's the value that matters
		LastFrameTickData = TickData;
		bLastFrameCanEdit = bCanEdit;
	}

	if (Parameters.Mode == ECallToolMode::Apply)
	{
		// Apply right away, tick won't be called again
		ApplyPendingFrameBounds();
	}
}

///////////////////////////////////////////////////////////////////////////////

FVector UVoxelToolBase::GetToolPosition() const
{
	return StridePosition;
}

FVector UVoxelToolBase::GetToolPreviewPosition() const
{
	// Ignore stride for preview
	return CurrentPosition;
}

FVector UVoxelToolBase::GetToolNormal() const
{
	return StrideNormal;
}

FVector UVoxelToolBase::GetToolDirection() const
{
	if (ToolBaseConfig.bUseFixedDirection)
	{
		return ToolBaseConfig.FixedDirection.Vector();
	}
	else
	{
		return StrideDirection;
	}
}

void UVoxelToolBase::SetToolOverlayBounds(const FBox& Bounds)
{
	if (ensure(VoxelWorld) && ensure(VoxelWorld->IsCreated()))
	{
		VoxelWorld->GetSubsystemChecked<FVoxelToolRenderingManager>().EditTool(ToolRenderingId, [&](FVoxelToolRendering& Tool)
		{
			Tool.WorldBounds = Bounds;
		});
	}
}

void UVoxelToolBase::UpdateToolMesh(
	UStaticMesh* Mesh,
	UMaterialInterface* Material,
	const FTransform& Transform,
	FName Id)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(VoxelWorld))
	{
		return;
	}

	auto& StaticMeshActor = StaticMeshActors.FindOrAdd(Id);
	if (!StaticMeshActor.IsValid())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.bDeferConstruction = true;
		SpawnParameters.ObjectFlags = RF_Transient;
		StaticMeshActor = VoxelWorld->GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParameters);
		StaticMeshActor->SetMobility(EComponentMobility::Movable);
#if WITH_EDITOR
		StaticMeshActor->SetActorLabel("VoxelToolMeshActor");
#endif
		StaticMeshActor->SetActorEnableCollision(false);

		UStaticMeshComponent* MeshComponent = StaticMeshActor->GetStaticMeshComponent();
		if (ensure(MeshComponent))
		{
			MeshComponent->CastShadow = false;
		}
		
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

FVoxelIntBox UVoxelToolBase::GetBoundsToCache(const FVoxelIntBox& Bounds) const
{
	const auto BoundsToCache = UVoxelBlueprintLibrary::GetRenderBoundsOverlappingDataBounds(VoxelWorld, Bounds);

	if (SharedConfig->bDebug)
	{
		UVoxelDebugUtilities::DrawDebugIntBox(VoxelWorld, Bounds, 1.5f * GetDeltaTime(), 0, FColor::Green);
		UVoxelDebugUtilities::DrawDebugIntBox(VoxelWorld, BoundsToCache, 1.5f * GetDeltaTime(), 0, FColor::Red);
	}
	
	return BoundsToCache;
}

float UVoxelToolBase::GetValueAfterAxisInput(FName AxisName, float CurrentValue, float Min, float Max) const
{
	const float AxisValue = GetTickData().GetAxis(AxisName);
	if (AxisValue != 0)
	{
		return FMath::Clamp(CurrentValue * (1.f + SharedConfig->ControlSpeed * AxisValue), Min, Max); 
	}
	else
	{
		// Don't clamp if no axis input!
		return CurrentValue;
	}
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelToolBase::ClearVoxelWorld()
{
	VOXEL_FUNCTION_COUNTER();
	
	if (VoxelWorld && VoxelWorld->IsCreated())
	{
		ApplyPendingFrameBounds();
		
		if (ToolRenderingId.IsValid())
		{
			auto& ToolRenderingManager = VoxelWorld->GetSubsystemChecked<FVoxelToolRenderingManager>();
			if (ToolRenderingManager.IsValidTool(ToolRenderingId)) // Could be invalid if the voxel world was toggled off & on
			{
				ToolRenderingManager.RemoveTool(ToolRenderingId);
			}
		}
	}
	VoxelWorld = nullptr;
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

	UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(VoxelWorld, false);
}

void UVoxelToolBase::ApplyPendingFrameBounds()
{
	if (!PendingFrameBounds.IsValid())
	{
		return;
	}
	
	if (ensure(VoxelWorld) && ensure(VoxelWorld->IsCreated()))
	{
		auto& Data = VoxelWorld->GetSubsystemChecked<FVoxelData>();
		if (Data.bEnableUndoRedo)
		{
			Data.SaveFrame(PendingFrameBounds.GetBox());
			SharedConfig->RegisterTransaction.Broadcast(GetToolName(), VoxelWorld);
		}
	}


	if (SharedConfig->bCheckForSingleValues)
	{
		UVoxelDataTools::CheckForSingleValues(VoxelWorld, PendingFrameBounds.GetBox());
		UVoxelDataTools::CheckForSingleMaterials(VoxelWorld, PendingFrameBounds.GetBox());
	}

	if (SharedConfig->bDebug)
	{
		UVoxelDebugUtilities::DrawDebugIntBox(VoxelWorld, PendingFrameBounds.GetBox(), 0.5f, 0, FColor::Purple);
	}

	PendingFrameBounds.Reset();
}