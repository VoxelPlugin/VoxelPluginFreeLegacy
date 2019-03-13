// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelRender/VoxelProcMeshTangent.h"
#include "Components/MeshComponent.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "PhysicsEngine/ConvexElem.h"
#include "VoxelProceduralMeshComponent.generated.h"

class UMaterialInterface;
class UBodySetup;
class FVoxelAsyncPhysicsCooker;
class AVoxelWorld;
class IVoxelPool;

USTRUCT()
struct FVoxelProcMeshSection
{
	GENERATED_BODY()
		
	UPROPERTY()
	UMaterialInterface* Material = nullptr;
	
	TArray<uint32> Indices;
	TArray<FVector> Positions;
	TArray<FVector> Normals;
	TArray<FVoxelProcMeshTangent> Tangents;
	TArray<FColor> Colors;
	TArray<FVector2D> TextureCoordinates;
	TArray<uint32> AdjacencyIndices;

	FBox SectionLocalBox = FBox(ForceInit);
	bool bEnableCollision = false;
	bool bEnableNavmesh = false;
	bool bSectionVisible = true;

	FVoxelProcMeshSection() = default;

	void Check();
};

enum class EVoxelProcMeshSectionUpdate
{
	UpdateNow,
	DelayUpdate
};

UCLASS(BlueprintType, Blueprintable)
class VOXEL_API UVoxelProceduralMeshComponent : public UMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()

public:
	bool bUseComplexAsSimpleCollision = true;

	/** Collision data */
	UPROPERTY()
	UBodySetup* ProcMeshBodySetup;

	TSharedPtr<IVoxelPool> Pool;
	uint64 Priority = 0;

public:
	UVoxelProceduralMeshComponent();
	~UVoxelProceduralMeshComponent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Voxel")
	void InitChunk(uint8 LOD, FIntBox ChunkBounds);
	
public:
	inline void DisableCollisions()
	{
		check(!IsRegistered());
		bEnableCollisions = false;
	}

	void SetCollisionConvexMeshes(TArray<FKConvexElem>&& InCollisionConvexElems);
	TArrayView<FVoxelProcMeshSection> GetSections() { return ProcMeshSections; }
	void SetProcMeshSection(int32 SectionIndex, FVoxelProcMeshSection&& Section, EVoxelProcMeshSectionUpdate Update);
	void ClearSections(EVoxelProcMeshSectionUpdate Update = EVoxelProcMeshSectionUpdate::UpdateNow);
	void FinishSectionsUpdates();
	void UpdatePhysicalMaterials();
	
public:
	//~ Begin Interface_CollisionDataProvider Interface
	bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) final;
	bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const final;
	bool WantsNegXTriMesh() final { return false; }
	//~ End Interface_CollisionDataProvider Interface

	//~ Begin UPrimitiveComponent Interface.
	FPrimitiveSceneProxy* CreateSceneProxy() final;
	UBodySetup* GetBodySetup() final;
	UMaterialInterface* GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const final;
	void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const final;	
	UMaterialInterface* GetMaterial(int32 ElementIndex) const final;
	bool DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const final;
	int32 GetNumMaterials() const final { return ProcMeshSections.Num(); }
	FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const final;
	void PostLoad() final;
	//~ End UPrimitiveComponent Interface.
	
private:
	// Update LocalBounds member from the local box of each section
	void UpdateLocalBounds();
	// Update Navigation if needed
	void UpdateNavigation();
	// Mark collision data as dirty, and re-create on instance if necessary
	void UpdateCollision();
	void FinishCollisionUpdate();

public:
	void AsyncPhysicsCookerCallback(uint64 CookerId);

private:
	/** Ensure ProcMeshBodySetup is allocated and configured */
	void CreateProcMeshBodySetup();
	/** Helper to create new body setup objects */
	UBodySetup* CreateBodySetupHelper();
	
private:
	UPROPERTY()
	TArray<FVoxelProcMeshSection> ProcMeshSections;

	UPROPERTY()
	UBodySetup* BodySetupBeingCooked;

	FVoxelAsyncPhysicsCooker* AsyncCooker = nullptr;

	TArray<FKConvexElem> CollisionConvexElems;
	FBoxSphereBounds LocalBounds;
	bool bEnableCollisions = true;

	friend class FVoxelProceduralMeshSceneProxy;
};
