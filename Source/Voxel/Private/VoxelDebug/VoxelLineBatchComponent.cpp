// Copyright 2020 Phyronnaz

#include "VoxelDebug/VoxelLineBatchComponent.h"
#include "VoxelMinimal.h"

#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "Engine/Engine.h"
#include "MaterialShared.h"
#include "Materials/Material.h"
#include "Engine/CollisionProfile.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"

DECLARE_DWORD_COUNTER_STAT(TEXT("Num Debug Lines Drawn"), STAT_NumDebugLinesDrawn, STATGROUP_VoxelCounters);

UVoxelLineBatchComponent::UVoxelLineBatchComponent()
{
	bAutoActivate = true;
	bTickInEditor = true;
	PrimaryComponentTick.bCanEverTick = true;

	UPrimitiveComponent::SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	bUseEditorCompositing = true;
	SetGenerateOverlapEvents(false);

	// Ignore streaming updates since GetUsedMaterials() is not implemented.
	bIgnoreStreamingManagerUpdate = true;
}

void UVoxelLineBatchComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	VOXEL_FUNCTION_COUNTER();

	bool bDirty = false;
	// Update the life time of batched lines, removing the lines which have expired.
	for (int32 LineIndex = 0; LineIndex < BatchedLines.Num(); LineIndex++)
	{
		FBatchedLine& Line = BatchedLines[LineIndex];
		if (Line.RemainingLifeTime > 0.0f)
		{
			Line.RemainingLifeTime -= DeltaTime;
			if (Line.RemainingLifeTime <= 0.0f)
			{
				// The line has expired, remove it.
				BatchedLines.RemoveAtSwap(LineIndex--);
				bDirty = true;
			}
		}
	}

	// Update the life time of batched points, removing the points which have expired.
	for (int32 PtIndex = 0; PtIndex < BatchedPoints.Num(); PtIndex++)
	{
		FBatchedPoint& Pt = BatchedPoints[PtIndex];
		if (Pt.RemainingLifeTime > 0.0f)
		{
			Pt.RemainingLifeTime -= DeltaTime;
			if (Pt.RemainingLifeTime <= 0.0f)
			{
				// The point has expired, remove it.
				BatchedPoints.RemoveAtSwap(PtIndex--);
				bDirty = true;
			}
		}
	}

	// Update the life time of batched meshes, removing the meshes which have expired.
	for (int32 MeshIndex = 0; MeshIndex < BatchedMeshes.Num(); MeshIndex++)
	{
		FBatchedMesh& Mesh = BatchedMeshes[MeshIndex];
		if (Mesh.RemainingLifeTime > 0.0f)
		{
			Mesh.RemainingLifeTime -= DeltaTime;
			if (Mesh.RemainingLifeTime <= 0.0f)
			{
				// The mesh has expired, remove it.
				BatchedMeshes.RemoveAtSwap(MeshIndex--);
				bDirty = true;
			}
		}
	}

	if (bDirty)
	{
		MarkRenderStateDirty();
	}
}

void UVoxelLineBatchComponent::ApplyWorldOffset(const FVector& InOffset, bool bWorldShift)
{
	Super::ApplyWorldOffset(InOffset, bWorldShift);
	
	VOXEL_FUNCTION_COUNTER();

	bool bDirty = false;
	for (FBatchedLine& Line : BatchedLines)
	{
		Line.Start += InOffset;
		Line.End += InOffset;
		bDirty = true;
	}

	for (FBatchedPoint& Point : BatchedPoints)
	{
		Point.Position += InOffset;
		bDirty = true;
	}

	for (FBatchedMesh& Mesh : BatchedMeshes)
	{
		for (FVector& Vert : Mesh.MeshVerts)
		{
			Vert += InOffset;
			bDirty = true;
		}
	}

	if (bDirty)
	{
		MarkRenderStateDirty();
	}
}

FPrimitiveSceneProxy* UVoxelLineBatchComponent::CreateSceneProxy()
{
	if (BatchedLines.Num() == 0 &&
		BatchedPoints.Num() == 0 &&
		BatchedMeshes.Num() == 0)
	{
		return nullptr;
	}
	
	return new FVoxelLineBatcherSceneProxy(this);
}

FBoxSphereBounds UVoxelLineBatchComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!bCalculateAccurateBounds)
	{
		const FVector BoxExtent(HALF_WORLD_MAX);
		return FBoxSphereBounds(FVector::ZeroVector, BoxExtent, BoxExtent.Size());
	}

	FBox BBox(ForceInit);
	for (const FBatchedLine& Line : BatchedLines)
	{
		BBox += Line.Start;
		BBox += Line.End;
	}

	for (const FBatchedPoint& Point : BatchedPoints)
	{
		BBox += Point.Position;
	}

	for (const FBatchedMesh& Mesh : BatchedMeshes)
	{
		for (const FVector& Vert : Mesh.MeshVerts)
		{
			BBox += Vert;
		}
	}

	if (BBox.IsValid)
	{
		// Points are in world space, so no need to transform.
		return FBoxSphereBounds(BBox);
	}
	else
	{
		const FVector BoxExtent(1.f);
		return FBoxSphereBounds(LocalToWorld.GetLocation(), BoxExtent, 1.f);
	}
}

void UVoxelLineBatchComponent::Flush()
{
	VOXEL_FUNCTION_COUNTER();

	if (BatchedLines.Num() > 0 || BatchedPoints.Num() > 0 || BatchedMeshes.Num() > 0)
	{
		BatchedLines.Empty();
		BatchedPoints.Empty();
		BatchedMeshes.Empty();
		MarkRenderStateDirty();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelLineBatcherSceneProxy::FVoxelLineBatcherSceneProxy(const UVoxelLineBatchComponent* InComponent)
	: FPrimitiveSceneProxy(InComponent)
	, Lines(InComponent->BatchedLines)
	, Points(InComponent->BatchedPoints)
	, Meshes(InComponent->BatchedMeshes)
{
	bWillEverBeLit = false;
}

SIZE_T FVoxelLineBatcherSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

void FVoxelLineBatcherSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	VOXEL_RENDER_FUNCTION_COUNTER();

	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = Views[ViewIndex];
			FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

			INC_DWORD_STAT_BY(STAT_NumDebugLinesDrawn, Lines.Num());
			if (ensure(!PDI->View)) // FSimpleElementCollector does not have a view
			{
				auto* SimpleCollector = static_cast<FSimpleElementCollector*>(PDI);
				auto& BatchedElements = SimpleCollector->BatchedElements; // No support for depth priority; Would need to use TopBatchedElements
				
				// Reserve all for thick and not thick - we don't care about losing a bit of memory there
				BatchedElements.AddReserveLines(Lines.Num(), false, false);
				BatchedElements.AddReserveLines(Lines.Num(), false, true);
				for (auto& Line : Lines)
				{
					BatchedElements.AddLine(Line.Start, Line.End, Line.Color, FHitProxyId(), Line.Thickness);
				}
			}
			else
			{
				// Slow path
				for (auto& Line : Lines)
				{
					PDI->DrawLine(Line.Start, Line.End, Line.Color, Line.DepthPriority, Line.Thickness);
				}
			}

			for (auto& Point : Points)
			{
				PDI->DrawPoint(Point.Position, Point.Color, Point.PointSize, Point.DepthPriority);
			}

			for (auto& Mesh : Meshes)
			{
				static FVector const PosX(1.f, 0, 0);
				static FVector const PosY(0, 1.f, 0);
				static FVector const PosZ(0, 0, 1.f);

				// this seems far from optimal in terms of perf, but it's for debugging
				FDynamicMeshBuilder MeshBuilder(View->GetFeatureLevel());

				for (int32 VertIdx = 0; VertIdx < Mesh.MeshVerts.Num(); ++VertIdx)
				{
					MeshBuilder.AddVertex(Mesh.MeshVerts[VertIdx], FVector2D::ZeroVector, PosX, PosY, PosZ, FColor::White);
				}
				for (int32 Idx = 0; Idx < Mesh.MeshIndices.Num(); Idx += 3)
				{
					MeshBuilder.AddTriangle(Mesh.MeshIndices[Idx], Mesh.MeshIndices[Idx + 1], Mesh.MeshIndices[Idx + 2]);
				}

				FMaterialRenderProxy* const MaterialRenderProxy = new FColoredMaterialRenderProxy(GEngine->DebugMeshMaterial->GetRenderProxy(), Mesh.Color);
				Collector.RegisterOneFrameMaterialProxy(MaterialRenderProxy);
				MeshBuilder.GetMesh(FMatrix::Identity, MaterialRenderProxy, Mesh.DepthPriority, false, false, ViewIndex, Collector);
			}
		}
	}
}

FPrimitiveViewRelevance FVoxelLineBatcherSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance ViewRelevance;
	ViewRelevance.bDrawRelevance = IsShown(View);
	ViewRelevance.bDynamicRelevance = true;
	// ideally the TranslucencyRelevance should be filled out by the material, here we do it conservative
	ViewRelevance.UE_25_SWITCH(bSeparateTranslucencyRelevance, bSeparateTranslucency) = true;
	ViewRelevance.UE_25_SWITCH(bNormalTranslucencyRelevance, bNormalTranslucency) = true;
	return ViewRelevance;
}

uint32 FVoxelLineBatcherSceneProxy::GetMemoryFootprint(void) const
{
	return sizeof(*this) + GetAllocatedSize();
}

uint32 FVoxelLineBatcherSceneProxy::GetAllocatedSize(void) const
{
	return FPrimitiveSceneProxy::GetAllocatedSize() + Lines.GetAllocatedSize() + Points.GetAllocatedSize() + Meshes.GetAllocatedSize();
}