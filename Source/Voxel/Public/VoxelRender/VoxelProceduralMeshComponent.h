// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelRender/VoxelProcMeshTangent.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "PhysicsEngine/ConvexElem.h"
#include "VoxelProceduralMeshComponent.generated.h"

class UMaterialInterface;
class UBodySetup;
class FVoxelAsyncPhysicsCooker;
class AVoxelWorld;
class IVoxelPool;

DECLARE_MEMORY_STAT(TEXT("Voxel Proc Mesh Section Memory"), STAT_VoxelProcMeshSectionMemory, STATGROUP_VoxelMemory);

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
	~FVoxelProcMeshSection()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelProcMeshSectionMemory, LastAllocatedSize);
	}

	void Check();

	inline int32 GetAllocatedSize() const
	{
		return Indices.GetAllocatedSize()
			+ Positions.GetAllocatedSize()
			+ Normals.GetAllocatedSize()
			+ Tangents.GetAllocatedSize()
			+ Colors.GetAllocatedSize()
			+ TextureCoordinates.GetAllocatedSize()
			+ AdjacencyIndices.GetAllocatedSize();
	}

	inline void UpdateStat()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelProcMeshSectionMemory, LastAllocatedSize);
		LastAllocatedSize = GetAllocatedSize();
		INC_DWORD_STAT_BY(STAT_VoxelProcMeshSectionMemory, LastAllocatedSize);
	}

private:
	int32 LastAllocatedSize = 0;
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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel|Collisions")
	static bool AreVoxelCollisionsFrozen();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions")
	static void FreezeVoxelCollisions();
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions")
	static void ResumeVoxelCollisions();

private:
	static bool bAreCollisionsFrozen;
	static TSet<TWeakObjectPtr<UVoxelProceduralMeshComponent>> PendingCollisions;

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

protected:
#if WITH_EDITOR
	/**
	 * Horrible hack to make the foliage editor believe we are a static mesh.
	 */
	static void GetPrivateStaticClassBody(
		const TCHAR* PackageName,
		const TCHAR* Name,
		UClass*& ReturnClass,
		void(*RegisterNativeFunc)(),
		uint32 InSize,
		EClassFlags InClassFlags,
		EClassCastFlags InClassCastFlags,
		const TCHAR* InConfigName,
		UClass::ClassConstructorType InClassConstructor,
		UClass::ClassVTableHelperCtorCallerType InClassVTableHelperCtorCaller,
		UClass::ClassAddReferencedObjectsType InClassAddReferencedObjects,
		UClass::StaticClassFunctionType InSuperClassFn,
		UClass::StaticClassFunctionType InWithinClassFn)
	{
		::GetPrivateStaticClassBody(
			PackageName, 
			Name, 
			ReturnClass, 
			RegisterNativeFunc, 
			InSize, 
			InClassFlags, 
			InClassCastFlags, 
			InConfigName, 
			InClassConstructor, 
			InClassVTableHelperCtorCaller, 
			InClassAddReferencedObjects, 
			FCString::Stristr(FCommandLine::Get(), TEXT("-voxelfoliageedmodehack")) ? &UStaticMeshComponent::StaticClass : InSuperClassFn,
			InWithinClassFn);
	}
#endif
};
