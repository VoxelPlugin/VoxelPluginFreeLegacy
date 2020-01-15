// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelPriorityHandler.h"
#include "VoxelRender/VoxelProcMeshSectionSettings.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "VoxelProceduralMeshComponent.generated.h"

struct FKConvexElem;
struct FVoxelProcMeshBuffers;
struct FVoxelRendererSettings;
class FVoxelToolRenderingManager;
class FDistanceFieldVolumeData;
class FVoxelAsyncPhysicsCooker;
class UBodySetup;
class UMaterialInterface;
class UVoxelProceduralMeshComponent;
class AVoxelWorld;
class IVoxelPool;
class IVoxelProceduralMeshComponent_PhysicsCallbackHandler;

enum class EVoxelProcMeshSectionUpdate : uint8
{
	UpdateNow,
	DelayUpdate
};

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Voxel), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelProceduralMeshComponent : public UMeshComponent
{
	GENERATED_BODY()
	
public:
	void Init(
		int32 InDebugLOD,
		uint32 InDebugChunkId,
		const FVoxelPriorityHandler& InPriorityHandler,
		const TVoxelWeakPtr<IVoxelProceduralMeshComponent_PhysicsCallbackHandler>& InPhysicsCallbackHandler,
		const FVoxelRendererSettings& RendererSettings);
	void ClearInit();
	
private:
	bool bInit = false;
	// Used for convex collisions
	uint64 UniqueId = 0;
	// Used to show LOD color in the mesh LOD visualization & for convex collision cooking
	int32 LOD = 0;
	// For debug
	uint32 DebugChunkId = 0;
	// Priority for physics cooking tasks
	FVoxelPriorityHandler PriorityHandler;
	// Will be triggered by the async cooker on an async thread, and then will trigger us on game thread
	TVoxelWeakPtr<IVoxelProceduralMeshComponent_PhysicsCallbackHandler> PhysicsCallbackHandler;
	// Weak ptr else the pool stays created until GC
	TVoxelWeakPtr<IVoxelPool> Pool;
	// Used to show tools overlays
	TVoxelWeakPtr<const FVoxelToolRenderingManager> ToolRenderingManager;
	// For cooking tasks
	float PriorityDuration = 0;
	// Collisions settings
	ECollisionTraceFlag CollisionTraceFlag = ECollisionTraceFlag::CTF_UseDefault;
	// For convex collisions
	int32 NumConvexHullsPerAxis = 2;
	// Cooks slower, but won't crash in case of weird complex geometry
	bool bCleanCollisionMesh = false;
	// Will clear the proc mesh buffers once navmesh + collisions have been built
	bool bClearProcMeshBuffersOnFinishUpdate = false;
	// If we were just init, and no proxy have yet been created
	// Used to fix motion blur
	bool bNewInit = false;
	
public:
	UVoxelProceduralMeshComponent();
	~UVoxelProceduralMeshComponent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Voxel")
	void InitChunk(uint8 ChunkLOD, FIntBox ChunkBounds);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel|Collisions")
	static bool AreVoxelCollisionsFrozen();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions")
	static void SetVoxelCollisionsFrozen(bool bFrozen);

private:
	static bool bAreCollisionsFrozen;
	static TSet<TWeakObjectPtr<UVoxelProceduralMeshComponent>> PendingCollisions;

public:
	void SetDistanceFieldData(const TVoxelSharedPtr<const FDistanceFieldVolumeData>& InDistanceFieldData);
	void SetProcMeshSection(int32 Index, FVoxelProcMeshSectionSettings Settings, TUniquePtr<FVoxelProcMeshBuffers> Buffers, EVoxelProcMeshSectionUpdate Update);
	int32 AddProcMeshSection(FVoxelProcMeshSectionSettings Settings, TUniquePtr<FVoxelProcMeshBuffers> Buffers, EVoxelProcMeshSectionUpdate Update);
	void ReplaceProcMeshSection(FVoxelProcMeshSectionSettings Settings, TUniquePtr<FVoxelProcMeshBuffers> Buffers, EVoxelProcMeshSectionUpdate Update);
	void ClearSections(EVoxelProcMeshSectionUpdate Update);
	void FinishSectionsUpdates();

	template<typename F>
	inline void IterateSectionsSettings(F Lambda)
	{
		for (auto& Section : ProcMeshSections)
		{
			Lambda(Section.Settings);
		}
	}
	template<typename F>
	inline void IterateSections(F Lambda) const
	{
		for (auto& Section : ProcMeshSections)
		{
			Lambda(Section.Settings, static_cast<const FVoxelProcMeshBuffers&>(*Section.Buffers));
		}
	}
	
public:
	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override final;
	virtual UBodySetup* GetBodySetup() override final;
	virtual UMaterialInterface* GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const override final;
	virtual int32 GetNumMaterials() const override final;
	virtual UMaterialInterface* GetMaterial(int32 ElementIndex) const override final;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const override;
	virtual bool DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const override final;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override final;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	//~ End UPrimitiveComponent Interface.

	FMaterialRelevance GetMaterialRelevance_VoxelProcMesh(ERHIFeatureLevel::Type InFeatureLevel) const;
	
private:
	void UpdatePhysicalMaterials();
	void UpdateLocalBounds();
	void UpdateNavigation();
	void UpdateCollision();
	void FinishCollisionUpdate();
	void UpdateConvexMeshes(
		const FBox& ConvexBounds,
		TArray<FKConvexElem>&& ConvexElements,
		TArray<physx::PxConvexMesh*>&& ConvexMeshes,
		bool bCanFail = false);

private:
	void PhysicsCookerCallback(uint64 CookerId);

	friend class FVoxelAsyncPhysicsCooker;
	friend class IVoxelProceduralMeshComponent_PhysicsCallbackHandler;
	
private:
	UPROPERTY(Transient)
	UBodySetup* BodySetup;
	UPROPERTY(Transient)
	UBodySetup* BodySetupBeingCooked;
	
	FVoxelAsyncPhysicsCooker* AsyncCooker = nullptr;

	struct FVoxelProcMeshSection
	{
		FVoxelProcMeshSectionSettings Settings;
		TVoxelSharedPtr<const FVoxelProcMeshBuffers> Buffers;
	};
	TArray<FVoxelProcMeshSection> ProcMeshSections;
	TVoxelSharedPtr<const FDistanceFieldVolumeData> DistanceFieldData;

	// Used to skip rebuilding collisions & navmesh
	// GUID to detect geometry change
	// Map to detect settings changes
	TArray<FGuid> ProcMeshSectionsSortedGuids;
	TMap<FGuid, FVoxelProcMeshSectionSettings> ProcMeshSectionsGuidToSettings;
	
	FBoxSphereBounds LocalBounds;

	double LastFinishSectionsUpdatesTime = 0;

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
#if ENGINE_MINOR_VERSION >= 23
		uint32 InAlignment,
#endif
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
#if ENGINE_MINOR_VERSION >= 23
			InAlignment,
#endif
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
