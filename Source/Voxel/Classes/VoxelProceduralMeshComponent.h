// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "Components/MeshComponent.h"
#include "PhysicsEngine/ConvexElem.h"
#include "VoxelVertexFactory.h"
#include "PrimitiveSceneProxy.h"
#include "Runtime/Launch/Resources/Version.h"
#include "VoxelProceduralMeshComponent.generated.h"

class FPrimitiveSceneProxy;

/**
*	Struct used to specify a tangent vector for a vertex
*	The Y tangent is computed from the cross product of the vertex normal (Tangent Z) and the TangentX member.
*/
struct FVoxelProcMeshTangent
{
	/** Direction of X tangent for this vertex */
	FVector TangentX;

	/** Bool that indicates whether we should flip the Y tangent when we compute it using cross product */
	bool bFlipTangentY;

	FVoxelProcMeshTangent()
		: TangentX(1.f, 0.f, 0.f)
		, bFlipTangentY(false)
	{
	}

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
};

/** One vertex for the procedural mesh, used for storing data internally */
struct FVoxelProcMeshVertex
{
	/** Vertex position */
	FVector Position;

	/** Vertex normal */
	FVector Normal;

	/** Vertex tangent */
	FVoxelProcMeshTangent Tangent;

	/** Vertex color */
	FColor Color;

	FVector2D TextureCoordinate;


	FVoxelProcMeshVertex()
		: Position(0.f, 0.f, 0.f)
		, Normal(0.f, 0.f, 1.f)
		, Tangent(FVector(1.f, 0.f, 0.f), false)
		, Color(255, 255, 255)
		, TextureCoordinate(FVector2D::ZeroVector)
	{
	}
};

/** One section of the procedural mesh. Each material has its own section. */
struct FVoxelProcMeshSection
{
	/** Vertex buffer for this section */
	TArray<FVoxelProcMeshVertex> ProcVertexBuffer;

	/** Index buffer for this section */
	TArray<int32> ProcIndexBuffer;

	/** Local bounding box of section */
	FBox SectionLocalBox;

	/** Should we build collision data for triangles in this section */
	bool bEnableCollision;

	/** Should we display this section */
	bool bSectionVisible;

	FVoxelProcMeshSection()
		: SectionLocalBox(ForceInit)
		, bEnableCollision(false)
		, bSectionVisible(true)
	{
	}

	/** Reset this section, clear all mesh info. */
	void Reset()
	{
		ProcVertexBuffer.Empty();
		ProcIndexBuffer.Empty();
		SectionLocalBox.Init();
		bEnableCollision = false;
		bSectionVisible = true;
	}
};

/** Class representing a single section of the proc mesh */
class FVoxelProcMeshProxySection
{
public:
	/** Material applied to this section */
	UMaterialInterface * Material;

	/** Vertex buffer for this section */
	FVoxelProcMeshVertexBuffer VertexBuffer;

	/** Index buffer for this section */
	FVoxelProcMeshIndexBuffer IndexBuffer;

	/** Vertex factory for this section */
	FVoxelVertexFactory VertexFactory;

	/** Whether this section is currently visible */
	bool bSectionVisible;

	/** Buffer for tessellation */
	FVoxelProcMeshIndexBuffer AdjacencyIndexBuffer;

	bool bRequiresAdjacencyInformation;

#if ENGINE_MINOR_VERSION < 19
	FVoxelProcMeshProxySection()
		: Material(NULL)
		, bSectionVisible(true)
		, bRequiresAdjacencyInformation(false)
	{
	}
#else
	FVoxelProcMeshProxySection(ERHIFeatureLevel::Type InFeatureLevel)
		: Material(NULL)
		, bSectionVisible(true)
		, bRequiresAdjacencyInformation(false)
		, VertexFactory(InFeatureLevel)
	{
	}
#endif
};


/** Procedural mesh scene proxy */
class FVoxelProceduralMeshSceneProxy : public FPrimitiveSceneProxy
{
public:

	FVoxelProceduralMeshSceneProxy(class UVoxelProceduralMeshComponent* Component);

	virtual ~FVoxelProceduralMeshSceneProxy();

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const;

	virtual bool CanBeOccluded() const override;

	virtual uint32 GetMemoryFootprint(void) const;

	uint32 GetAllocatedSize(void) const;

#if ENGINE_MINOR_VERSION >= 19	
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}
#endif

private:
	/** Array of sections */
	TArray<FVoxelProcMeshProxySection*> Sections;

	UBodySetup* BodySetup;

	FMaterialRelevance MaterialRelevance;
};


UCLASS()
class VOXEL_API UVoxelProceduralMeshComponent : public UMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_UCLASS_BODY()
public:
	/** Returns number of sections currently created for this component */
	int32 GetNumSections() const;

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

	/**
	 *	Controls whether the complex (Per poly) geometry should be treated as 'simple' collision.
	 *	Should be set to false if this component is going to be given simple collision and simulated.
	 */
	bool bUseComplexAsSimpleCollision;

	/**
	 *	Controls whether the physics cooking should be done off the game thread. This should be used when collision geometry doesn't have to be immediately up to date (For example streaming in far away objects)
	 */
	bool bUseAsyncCooking;

	/** Collision data */
	UPROPERTY()
	class UBodySetup* ProcMeshBodySetup;

	/**
	 *	Get pointer to internal data for one section of this procedural mesh component.
	 *	Note that pointer will becomes invalid if sections are added or removed.
	 */
	FVoxelProcMeshSection* GetProcMeshSection(int32 SectionIndex);

	/** Replace a section with new section geometry */
	void SetProcMeshSection(int32 SectionIndex, const FVoxelProcMeshSection& Section);

	//~ Begin UPrimitiveComponent Interface.
	FPrimitiveSceneProxy* CreateSceneProxy() override;
	class UBodySetup* GetBodySetup() override;
	UMaterialInterface* GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

	//~ Begin UObject Interface
	void PostLoad() override;
	//~ End UObject Interface.

private:
	//~ Begin USceneComponent Interface.
	FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent Interface.


	/** Mark collision data as dirty, and re-create on instance if necessary */
	void UpdateCollision();
	/** Update LocalBounds member from the local box of each section */
	void UpdateLocalBounds();
	/** Ensure ProcMeshBodySetup is allocated and configured */
	void CreateProcMeshBodySetup();
	/** Once async physics cook is done, create needed state */
	void FinishPhysicsAsyncCook(UBodySetup* FinishedBodySetup);

	/** Helper to create new body setup objects */
	UBodySetup* CreateBodySetupHelper();

	/** Array of sections of mesh */
	TArray<FVoxelProcMeshSection> ProcMeshSections;

	/** Convex shapes used for simple collision */
	TArray<FKConvexElem> CollisionConvexElems;

	/** Local space bounds of mesh */
	FBoxSphereBounds LocalBounds;

	/** Queue for async body setups that are being cooked */
	UPROPERTY()
	TArray<UBodySetup*> AsyncBodySetupQueue;

	friend class FVoxelProceduralMeshSceneProxy;
};


