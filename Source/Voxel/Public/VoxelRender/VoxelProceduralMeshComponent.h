// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "Components/MeshComponent.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PrimitiveSceneProxy.h"
#include "Runtime/Launch/Resources/Version.h"
#include "StaticMeshResources.h"
#include "VoxelProceduralMeshComponent.generated.h"

class FPrimitiveSceneProxy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Constructor isn't exported in the engine definition
 */
class FVoxelRawStaticIndexBuffer : public FIndexBuffer
{
public:	
	/**
	 * Initialization constructor.
	 * @param InNeedsCPUAccess	True if resource array data should be accessible by the CPU.
	 */
	FVoxelRawStaticIndexBuffer(bool InNeedsCPUAccess=false);

	/**
	 * Sets a single index value.  Consider using SetIndices() instead if you're setting a lot of indices.
	 * @param	At	The index of the index to set
	 * @param	NewIndexValue	The index value
	 */
	inline void SetIndex( const uint32 At, const uint32 NewIndexValue )
	{
		check( At >= 0 && At < (uint32)IndexStorage.Num() );

		if( b32Bit )
		{
			uint32* Indices32Bit = (uint32*)IndexStorage.GetData();
			Indices32Bit[ At ] = NewIndexValue;
		}
		else
		{
			uint16* Indices16Bit = (uint16*)IndexStorage.GetData();
			Indices16Bit[ At ] = (uint16)NewIndexValue;
		}
	}

	/**
	 * Set the indices stored within this buffer.
	 * @param InIndices		The new indices to copy in to the buffer.
	 * @param DesiredStride	The desired stride (16 or 32 bits).
	 */
	void SetIndices(const TArray<uint32>& InIndices, EIndexBufferStride::Type DesiredStride);

	/**
	 * Insert indices at the given position in the buffer
	 * @param	At					Index to insert at
	 * @param	IndicesToAppend		Pointer to the array of indices to insert
	 * @param	NumIndicesToAppend	How many indices are in the IndicesToAppend array
	 */
	void InsertIndices( const uint32 At, const uint32* IndicesToAppend, const uint32 NumIndicesToAppend );

	/**
	 * Append indices to the end of the buffer
	 * @param	IndicesToAppend		Pointer to the array of indices to add to the end
	 * @param	NumIndicesToAppend	How many indices are in the IndicesToAppend array
	 */
	void AppendIndices( const uint32* IndicesToAppend, const uint32 NumIndicesToAppend );

	/** @return Gets a specific index value */
	inline uint32 GetIndex( const uint32 At ) const
	{
		check( At >= 0 && At < (uint32)IndexStorage.Num() );
		uint32 IndexValue;
		if( b32Bit )
		{
			const uint32* SrcIndices32Bit = (const uint32*)IndexStorage.GetData();
			IndexValue = SrcIndices32Bit[ At ];
		}
		else
		{
			const uint16* SrcIndices16Bit = (const uint16*)IndexStorage.GetData();
			IndexValue = SrcIndices16Bit[ At ];
		}

		return IndexValue;
	}


	/**
	 * Removes indices from the buffer
	 *
	 * @param	At	The index of the first index to remove
	 * @param	NumIndicesToRemove	How many indices to remove
	 */
	void RemoveIndicesAt( const uint32 At, const uint32 NumIndicesToRemove );

	/**
	 * Retrieve a copy of the indices in this buffer. Only valid if created with
	 * NeedsCPUAccess set to true or the resource has not yet been initialized.
	 * @param OutIndices	Array in which to store the copy of the indices.
	 */
	void GetCopy(TArray<uint32>& OutIndices) const;

	/**
	 * Get the direct read access to index data 
	 * Only valid if NeedsCPUAccess = true and indices are 16 bit
	 */
	const uint16* AccessStream16() const;

	/**
	 * Retrieves an array view in to the index buffer. The array view allows code
	 * to retrieve indices as 32-bit regardless of how they are stored internally
	 * without a copy. The array view is valid only if:
	 *		The buffer was created with NeedsCPUAccess = true
	 *		  OR the resource has not yet been initialized
	 *		  AND SetIndices has not been called since.
	 */
	FIndexArrayView GetArrayView() const;

	/**
	 * Computes the number of indices stored in this buffer.
	 */
	FORCEINLINE int32 GetNumIndices() const
	{
		return NumIndices;
	}

	/**
	 * Computes the amount of memory allocated to store the indices.
	 */
	FORCEINLINE uint32 GetAllocatedSize() const
	{
		return IndexStorage.GetAllocatedSize();
	}

	/**
	 * Serialization.
	 * @param	Ar				Archive to serialize with
	 * @param	bNeedsCPUAccess	Whether the elements need to be accessed by the CPU
	 */
	void Serialize(FArchive& Ar, bool bNeedsCPUAccess);

    /**
     * Discard
     * discards the serialized data when it is not needed
     */
    void Discard();
    
	// FRenderResource interface.
	virtual void InitRHI() override;

	inline bool Is32Bit() const { return b32Bit; }

private:
	/** Storage for indices. */
	TResourceArray<uint8,INDEXBUFFER_ALIGNMENT> IndexStorage;
	/** 32bit or 16bit? */
	bool b32Bit;
	/** The cached number of indices. */
	uint32 NumIndices = 0;

	void UpdateCachedNumIndices()
	{
		NumIndices = b32Bit ? (IndexStorage.Num() / 4) : (IndexStorage.Num() / 2);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelProcMeshTangent
{
	FVector TangentX = FVector::RightVector;
	bool bFlipTangentY = false;

	FVoxelProcMeshTangent() = default;
	FVoxelProcMeshTangent(float X, float Y, float Z)
		: TangentX(X, Y, Z)
		, bFlipTangentY(false)
	{
	}
	FVoxelProcMeshTangent(FVector InTangentX, bool bInFlipTangentY)
		: TangentX(InTangentX)
		, bFlipTangentY(bInFlipTangentY)
	{
	}

	FVector GetY(const FVector& Normal) const
	{
		return (Normal ^ TangentX) * (bFlipTangentY ? -1 : 1);
	}
};

struct FVoxelProcMeshSection
{
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

	void Check()
	{
		for (int Index : Indices)
		{
			check(0 <= Index && Index < Positions.Num());
		}
		for (int Index : AdjacencyIndices)
		{
			check(0 <= Index && Index < Positions.Num());
		}

		check(Positions.Num() == Normals.Num() &&
			Positions.Num() == Tangents.Num() &&
			Positions.Num() == Colors.Num() &&
			Positions.Num() == TextureCoordinates.Num());
	}
};

///////////////////////////////////////////////////////////////////////////////

class FVoxelProcMeshProxySection
{
public:
	/** Material applied to this section */
	UMaterialInterface* Material;
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	/** Index buffer for this section */
	FVoxelRawStaticIndexBuffer IndexBuffer;
	/** Index buffer containing adjacency information required by tessellation. */
	FVoxelRawStaticIndexBuffer AdjacencyIndexBuffer;
	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;
	/** Whether this section is currently visible */
	bool bSectionVisible;
	/** For tessellation */
	bool bRequiresAdjacencyInformation;

	FVoxelProcMeshProxySection(ERHIFeatureLevel::Type InFeatureLevel)
	: Material(nullptr)
	, VertexFactory(InFeatureLevel, "FProcMeshProxySection")
	, bSectionVisible(true)
	, bRequiresAdjacencyInformation(false)
	{}

	void InitResources();
	void ReleaseResources();
};

/** Procedural mesh scene proxy */
class FVoxelProceduralMeshSceneProxy : public FPrimitiveSceneProxy
{
public:
	FVoxelProceduralMeshSceneProxy(class UVoxelProceduralMeshComponent* Component);
	~FVoxelProceduralMeshSceneProxy();

	void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	bool CanBeOccluded() const override;
	uint32 GetMemoryFootprint() const override;
	uint32 GetAllocatedSize() const;

	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

private:
	TArray<FVoxelProcMeshProxySection*> Sections;
	UBodySetup* const BodySetup;
	FMaterialRelevance const MaterialRelevance;
};

///////////////////////////////////////////////////////////////////////////////

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
	bool bUseAsyncCooking = true;

	/** Collision data */
	UPROPERTY()
	class UBodySetup* ProcMeshBodySetup;

public:
	UVoxelProceduralMeshComponent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Voxel")
	void InitChunk(uint8 LOD, FIntVector Position);
	
public:
	
	void SetCollisionsAreAlwaysDisabled(bool bValue);
	void SetNavmeshesAreAlwaysDisabled(bool bValue);

	/** Add simple collision convex to this component */
	void AddCollisionConvexMesh(TArray<FVector> ConvexVerts);
	/** Add simple collision convex to this component */
	void ClearCollisionConvexMeshes();
	/** Function to replace _all_ simple collision in one go */
	void SetCollisionConvexMeshes(const TArray< TArray<FVector> >& ConvexMeshes);
	
	//~ Begin Interface_CollisionDataProvider Interface
	bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) override;
	bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;
	bool WantsNegXTriMesh() override { return false; }
	//~ End Interface_CollisionDataProvider Interface
	
public:
	int32 GetNumSections() const;
	FVoxelProcMeshSection* GetProcMeshSection(int32 SectionIndex);
	void SetProcMeshSection(int32 SectionIndex, const FVoxelProcMeshSection& Section);
	void SetProcMeshSection(int32 SectionIndex, FVoxelProcMeshSection&& Section, EVoxelProcMeshSectionUpdate Update);
	void ClearSections(EVoxelProcMeshSectionUpdate Update = EVoxelProcMeshSectionUpdate::UpdateNow);
	void FinishSectionsUpdates();
	void UpdateMaterials();

public:
	//~ Begin UPrimitiveComponent Interface.
	FPrimitiveSceneProxy* CreateSceneProxy() override;
	class UBodySetup* GetBodySetup() override;
	UMaterialInterface* GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const override;
	void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const override;	
	UMaterialInterface* GetMaterial(int32 ElementIndex) const override;
	bool DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.
	
	//~ Begin UObject Interface
	void PostLoad() override;
	//~ End UObject Interface.

	//~ Begin USceneComponent Interface.
	FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent Interface.

	
private:
	void UpdateNavigation();
	/** Mark collision data as dirty, and re-create on instance if necessary */
	void UpdateCollision();
	/** Update LocalBounds member from the local box of each section */
	void UpdateLocalBounds();
	/** Ensure ProcMeshBodySetup is allocated and configured */
	void CreateProcMeshBodySetup();
	/** Once async physics cook is done, create needed state */
	void FinishPhysicsAsyncCook(bool bSuccess, UBodySetup* FinishedBodySetup);	
	/** Helper to create new body setup objects */
	UBodySetup* CreateBodySetupHelper();
	
private:
	/** Array of sections of mesh */
	TArray<FVoxelProcMeshSection> ProcMeshSections;

	/** Convex shapes used for simple collision */
	TArray<FKConvexElem> CollisionConvexElems;

	/** Local space bounds of mesh */
	FBoxSphereBounds LocalBounds;

	/** Queue for async body setups that are being cooked */
	UPROPERTY()
	TArray<UBodySetup*> AsyncBodySetupQueue;

	UPROPERTY()
	TArray<UMaterialInterface*> SectionMaterials;

	bool bCollisionsAreAlwaysDisabled = false;
	bool bNavmeshesAreAlwaysDisabled = false;

	friend class FVoxelProceduralMeshSceneProxy;
};
