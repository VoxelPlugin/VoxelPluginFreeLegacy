// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelAsyncPhysicsCooker.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelRender/IVoxelProceduralMeshComponent_PhysicsCallbackHandler.h"
#include "VoxelThreadingUtilities.h"
#include "VoxelPhysXHelpers.h"
#include "VoxelGlobals.h"

#if ENGINE_MINOR_VERSION < 23
#include "Physics/IPhysXCooking.h"
#include "Physics/IPhysXCookingModule.h"
#else
#include "IPhysXCooking.h"
#include "IPhysXCookingModule.h"
#endif

#include "Async/Async.h"
#include "PhysicsPublic.h"
#include "PhysicsEngine/PhysicsSettings.h"
//#include "ThirdParty/VHACD/public/VHACD.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static TAutoConsoleVariable<int32> CVarLogCollisionCookingTimes(
	TEXT("voxel.renderer.LogCollisionCookingTimes"),
	0,
	TEXT("If true, will log the time it took to cook the voxel meshes collisions"),
	ECVF_Default);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline IPhysXCooking* GetPhysXCooking()
{
	static IPhysXCookingModule* PhysXCookingModule = nullptr;
	if (!PhysXCookingModule)
	{
		PhysXCookingModule = GetPhysXCookingModule();
	}
	return PhysXCookingModule->GetPhysXCooking();
}

static const FName PhysXFormat = FPlatformProperties::GetPhysicsFormat();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelAsyncPhysicsCooker::FVoxelAsyncPhysicsCooker(UVoxelProceduralMeshComponent* Component)
	: FVoxelAsyncWork("AsyncPhysicsCooker", Component->PriorityDuration)
	, UniqueId(UNIQUE_ID())
	, Component(Component)
	, PhysicsCallbackHandler(Component->PhysicsCallbackHandler)
	, LOD(Component->LOD)
	, CollisionTraceFlag(
		Component->CollisionTraceFlag == ECollisionTraceFlag::CTF_UseDefault
		? ECollisionTraceFlag(UPhysicsSettings::Get()->DefaultShapeComplexity)
		: Component->CollisionTraceFlag)
	, PriorityHandler(Component->PriorityHandler)
	, bCleanCollisionMesh(Component->bCleanCollisionMesh)
	, NumConvexHullsPerAxis(Component->NumConvexHullsPerAxis)
	, Buffers([&]()
		{
			TArray<TVoxelSharedPtr<const FVoxelProcMeshBuffers>> TmpBuffers;
			TmpBuffers.Reserve(Component->ProcMeshSections.Num());
			for (auto& Section : Component->ProcMeshSections)
			{
				if (Section.Settings.bEnableCollisions)
				{
					TmpBuffers.Add(Section.Buffers);
				}
			}
			return TmpBuffers;
		}())
	, LocalToRoot(Component->GetRelativeTransform())
	, PhysXCooking(GetPhysXCooking())
{
	check(IsInGameThread());
	ensure(CollisionTraceFlag != ECollisionTraceFlag::CTF_UseDefault);
	ensure(Buffers.Num() > 0);
}

void FVoxelAsyncPhysicsCooker::DoWork()
{
	VOXEL_FUNCTION_COUNTER();

	const double CookStartTime = FPlatformTime::Seconds();
	
	if (CollisionTraceFlag != ECollisionTraceFlag::CTF_UseComplexAsSimple)
	{
		DecomposeMeshToHulls();
		CreateConvexMesh();
	}
	if (CollisionTraceFlag != ECollisionTraceFlag::CTF_UseSimpleAsComplex)
	{
		CreateTriMesh();
	}

	if (CVarLogCollisionCookingTimes.GetValueOnAnyThread() != 0)
	{
		LOG_VOXEL(Log, TEXT("Collisions cooking took %fms"), (FPlatformTime::Seconds() - CookStartTime) * 1000);
	}
}

void FVoxelAsyncPhysicsCooker::PostDoWork()
{
	auto Pinned = PhysicsCallbackHandler.Pin();
	if (Pinned.IsValid())
	{
		Pinned->CookerCallback(UniqueId, Component);
		FVoxelUtilities::DeleteOnGameThread_AnyThread(Pinned);
	}
}

uint32 FVoxelAsyncPhysicsCooker::GetPriority() const
{
	return PriorityHandler.GetPriority();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelAsyncPhysicsCooker::CreateTriMesh()
{
	VOXEL_FUNCTION_COUNTER();

	TArray<FVector> Vertices;
	TArray<FTriIndices> Indices;
	TArray<uint16> MaterialIndices;

	// Copy data from buffers
	{
		VOXEL_SCOPE_COUNTER("Copy data from buffers");

		{
			int32 NumIndices = 0;
			int32 NumVertices = 0;
			for (auto& Buffer : Buffers)
			{
				NumIndices += Buffer->GetNumIndices();
				NumVertices += Buffer->GetNumVertices();
			}
			VOXEL_SCOPE_COUNTER("Reserve");
			Vertices.Reserve(NumVertices);
			Indices.Reserve(NumIndices);
			MaterialIndices.Reserve(NumIndices);
		}

		int32 VertexOffset = 0;
		for (int32 SectionIndex = 0; SectionIndex < Buffers.Num(); SectionIndex++)
		{
			auto& Buffer = *Buffers[SectionIndex];
			const auto Get = [](auto& Array, int32 Index) -> auto&
			{
#if VOXEL_DEBUG
				return Array[Index];
#else
				return Array.GetData()[Index];
#endif
			};

			// Copy vertices
			{
				auto& PositionBuffer = Buffer.VertexBuffers.PositionVertexBuffer;

				const int32 Offset = Vertices.Num();
				check(PositionBuffer.GetNumVertices() <= uint32(Vertices.GetSlack()));
				Vertices.AddUninitialized(PositionBuffer.GetNumVertices());

				VOXEL_SCOPE_COUNTER("Copy vertices");
				for (uint32 Index = 0; Index < PositionBuffer.GetNumVertices(); Index++)
				{
					Get(Vertices, Offset + Index) = PositionBuffer.VertexPosition(Index);
				}
			}

			// Copy triangle data
			{
				auto& IndexBuffer = Buffer.IndexBuffer;

				ensure(Indices.Num() == MaterialIndices.Num());
				const int32 Offset = Indices.Num();
				ensure(IndexBuffer.GetNumIndices() % 3 == 0);
				const int32 NumTriangles = IndexBuffer.GetNumIndices() / 3;

				check(NumTriangles <= Indices.GetSlack());
				check(NumTriangles <= MaterialIndices.GetSlack());
				Indices.AddUninitialized(NumTriangles);
				MaterialIndices.AddUninitialized(NumTriangles);

				{
					VOXEL_SCOPE_COUNTER("Copy triangles");
					const auto Lambda = [&](const auto* RESTRICT Data)
					{
						for (int32 Index = 0; Index < NumTriangles; Index++)
						{
							// Need to add base offset for indices
							FTriIndices TriIndices;
							TriIndices.v0 = Data[3 * Index + 0] + VertexOffset;
							TriIndices.v1 = Data[3 * Index + 1] + VertexOffset;
							TriIndices.v2 = Data[3 * Index + 2] + VertexOffset;
							checkVoxelSlow(3 * Index + 2 < IndexBuffer.GetNumIndices());
							Get(Indices, Offset + Index) = TriIndices;
						}
					};
					if (IndexBuffer.Is32Bit())
					{
						Lambda(IndexBuffer.GetData_32());
					}
					else
					{
						Lambda(IndexBuffer.GetData_16());
					}
				}
				// Also store material info
				{
					VOXEL_SCOPE_COUNTER("Copy material info");
					for (int32 Index = 0; Index < NumTriangles; Index++)
					{
						Get(MaterialIndices, Offset + Index) = SectionIndex;
					}
				}
			}

			VertexOffset = Vertices.Num();
		}
	}

	if (Indices.Num() < 3)
	{
		// If less than 3 triangles the cooking is likely to fail
		return;
	}

	physx::PxTriangleMesh* TriangleMesh = nullptr;

	constexpr bool bFlipNormals = true; // Always true due to the order of the vertices (clock wise vs not)
	const bool bSuccess = PhysXCooking->CreateTriMesh(
		PhysXFormat,
		GetCookFlags(),
		Vertices,
		Indices,
		MaterialIndices,
		bFlipNormals,
		TriangleMesh);
	
	CookResult.TriangleMeshes.Add(TriangleMesh);

	if (TriangleMesh)
	{
		CookResult.TriangleMeshesMemoryUsage += FVoxelPhysXHelpers::GetAllocatedSize(*TriangleMesh);
	}
	
	if (!bSuccess)
	{
		// Happens sometimes
		LOG_VOXEL(Warning, TEXT("Failed to cook TriMesh. Num vertices: %d; Num triangles: %d"), Vertices.Num(), Indices.Num());
		ErrorCounter.Increment();
	}
}

void FVoxelAsyncPhysicsCooker::CreateConvexMesh()
{
	VOXEL_FUNCTION_COUNTER();

	for (auto& Element : CookResult.ConvexElems)
	{
		CookResult.ConvexMeshes.AddZeroed();
		const EPhysXCookingResult Result = PhysXCooking->CreateConvex(PhysXFormat, GetCookFlags(), Element.VertexData, CookResult.ConvexMeshes.Last());
		switch (Result)
		{
		case EPhysXCookingResult::Failed:
		{
			LOG_VOXEL(Warning, TEXT("Failed to cook convex"));
			ErrorCounter.Increment();
			break;
		}
		case EPhysXCookingResult::SucceededWithInflation:
		{
			LOG_VOXEL(Warning, TEXT("Cook convex failed but succeeded with inflation"));
			break;
		}
		case EPhysXCookingResult::Succeeded: break;
		default: ensure(false);
		}
	}
}

void FVoxelAsyncPhysicsCooker::DecomposeMeshToHulls()
{
	VOXEL_FUNCTION_COUNTER();
	
#if 0 // This is way too slow :(
	TArray<FVector> Vertices;
	TArray<uint32> Indices;
	
	// Copy data from buffers
	{
		VOXEL_SCOPE_COUNTER("Copy data from buffers");

		{
			int32 NumIndices = 0;
			int32 NumVertices = 0;
			for (auto& Buffer : Buffers)
			{
				NumIndices += Buffer->GetNumIndices();
				NumVertices += Buffer->GetNumVertices();
			}
			VOXEL_SCOPE_COUNTER("Reserve");
			Vertices.Reserve(NumVertices);
			Indices.Reserve(NumIndices);
		}

		int32 VertexOffset = 0;
		for (int32 SectionIndex = 0; SectionIndex < Buffers.Num(); SectionIndex++)
		{
			auto& Buffer = *Buffers[SectionIndex];
			const auto Get = [](auto& Array, int32 Index) -> auto&
			{
#if VOXEL_DEBUG
				return Array[Index];
#else
				return Array.GetData()[Index];
#endif
			};

			// Copy vertices
			{
				auto& PositionBuffer = Buffer.VertexBuffers.PositionVertexBuffer;

				const int32 Offset = Vertices.Num();
				Vertices.AddUninitialized(PositionBuffer.GetNumVertices());

				VOXEL_SCOPE_COUNTER("Copy vertices");
				for (uint32 Index = 0; Index < PositionBuffer.GetNumVertices(); Index++)
				{
					Get(Vertices, Offset + Index) = PositionBuffer.VertexPosition(Index);
				}
			}

			// Copy triangle data
			{
				auto& IndexBuffer = Buffer.IndexBuffer;

				const int32 Offset = Indices.Num();
				Indices.AddUninitialized(IndexBuffer.GetNumIndices());

				{
					VOXEL_SCOPE_COUNTER("Copy triangles");
					const auto Lambda = [&](const auto* RESTRICT Data)
					{
						for (int32 Index = 0; Index < IndexBuffer.GetNumIndices(); Index++)
						{
							checkVoxelSlow(Index < IndexBuffer.GetNumIndices());
							Get(Indices, Offset + Index) = Data[Index] + VertexOffset;
						}
					};
					if (IndexBuffer.Is32Bit())
					{
						Lambda(IndexBuffer.GetData_32());
					}
					else
					{
						Lambda(IndexBuffer.GetData_16());
					}
				}
			}

			VertexOffset = Vertices.Num();
		}
	}

	// Validate input by checking bounding box
	FBox Box(ForceInit);
	for (auto& Vertex : Vertices)
	{
		Box += Vertex;
	}

	// If box is invalid, or the largest dimension is less than 1 unit, or smallest is less than 0.1, skip trying to generate collision (V-HACD often crashes...)
	if (Box.IsValid == 0 || Box.GetSize().GetMax() < 1.f || Box.GetSize().GetMin() < 0.1f)
	{
		LOG_VOXEL(Warning, TEXT("Convex decomposition failed: mesh too small. Bounds: %s"), *Box.ToString());
		ErrorCounter.Increment();
		return;
	}

	VHACD::IVHACD::Parameters VHACD_Params;
	VHACD_Params.m_resolution = 1000; // Maximum number of voxels generated during the voxelization stage (default=100,000, range=10,000-16,000,000)
	VHACD_Params.m_maxNumVerticesPerCH = 16; // Controls the maximum number of triangles per convex-hull (default=64, range=4-1024)
	VHACD_Params.m_concavity = 0.001; // NOT TRUE: Concavity is set to zero so that we consider any concave shape as a potential hull. The number of output hulls is better controlled by recursion depth and the max convex hulls parameter
	VHACD_Params.m_maxConvexHulls = 2; // The number of convex hulls requested by the artists/designer
	VHACD_Params.m_oclAcceleration = false;
	VHACD_Params.m_minVolumePerCH = 1.f; // this should be around 1 / (3 * m_resolution ^ (1/3))
	VHACD_Params.m_projectHullVertices = false; // Project the approximate hull vertices onto the original source mesh and use highest precision results possible.
	VHACD_Params.m_callback = nullptr; // callback interface for message/status updates

#if 1
	class VHACDLogger : public VHACD::IVHACD::IUserLogger
	{
	public:
		virtual ~VHACDLogger() = default;
		virtual void Log(const char* const msg) override
		{
			LOG_VOXEL(Log, TEXT("VHACD: %s"), ANSI_TO_TCHAR(msg));
		}
	};
	VHACDLogger logger;
	VHACD_Params.m_logger = &logger;
#endif

	VHACD::IVHACD* InterfaceVHACD = VHACD::CreateVHACD();
	
	const float* const Verts = reinterpret_cast<float*>(Vertices.GetData());
	const unsigned int NumVerts = Vertices.Num();
	const uint32_t* const Tris = static_cast<uint32_t*>(Indices.GetData());
	const unsigned int NumTris = Indices.Num() / 3;

	bool bSuccess;
	{
		VOXEL_SCOPE_COUNTER("Compute");
		bSuccess = InterfaceVHACD->Compute(Verts, NumVerts, Tris, NumTris, VHACD_Params);
	}

	if (!bSuccess)
	{
		LOG_VOXEL(Warning, TEXT("Convex decomposition failed: VHACD failed"));
		ErrorCounter.Increment();
		return;
	}

	{
		VOXEL_SCOPE_COUNTER("Copy hulls");
		
		const int32 NumHulls = InterfaceVHACD->GetNConvexHulls();
		ensure(CookResult.ConvexElems.Num() == 0);
		CookResult.ConvexElems.SetNum(NumHulls);
		for (int32 HullIndex = 0; HullIndex < NumHulls; HullIndex++)
		{
			VHACD::IVHACD::ConvexHull Hull;
			InterfaceVHACD->GetConvexHull(HullIndex, Hull);

			FKConvexElem& ConvexElem = CookResult.ConvexElems[HullIndex];
			ConvexElem.VertexData.SetNumUninitialized(Hull.m_nPoints);
			for (uint32 Index = 0; Index < Hull.m_nPoints; Index++)
			{
				FVector V;
				V.X = float(Hull.m_points[(Index * 3) + 0]);
				V.Y = float(Hull.m_points[(Index * 3) + 1]);
				V.Z = float(Hull.m_points[(Index * 3) + 2]);
				ConvexElem.VertexData[Index] = V;
			}
			ConvexElem.UpdateElemBox();
		}
	}

	InterfaceVHACD->Clean();
	InterfaceVHACD->Release();
#else

	if (Buffers.Num() == 1 && Buffers[0]->GetNumVertices() < 4) return;

	auto& ConvexElems = CookResult.ConvexElems;
	
	FBox Box(ForceInit);
	for(auto& Buffer : Buffers)
	{
		auto& PositionBuffer = Buffer->VertexBuffers.PositionVertexBuffer;
		for (uint32 Index = 0; Index < PositionBuffer.GetNumVertices(); Index++)
		{
			Box += PositionBuffer.VertexPosition(Index);
		}
	}

	const int32 ChunkSize = RENDER_CHUNK_SIZE << LOD;
	const FIntVector Size =
		FVoxelUtilities::ComponentMax(
			FIntVector(1),
			FVoxelUtilities::CeilToInt(Box.GetSize() / ChunkSize * NumConvexHullsPerAxis));
	
	if (!ensure(Size.GetMax() <= 64)) return;

	ConvexElems.SetNum(Size.X * Size.Y * Size.Z);

	for (auto& Buffer : Buffers)
	{
		auto& PositionBuffer = Buffer->VertexBuffers.PositionVertexBuffer;
		for (uint32 Index = 0; Index < PositionBuffer.GetNumVertices(); Index++)
		{
			const FVector Vertex = PositionBuffer.VertexPosition(Index);
			
			FIntVector MainPosition;
			const auto Lambda = [&](int32 OffsetX, int32 OffsetY, int32 OffsetZ)
			{
				const FVector Offset = FVector(OffsetX, OffsetY, OffsetZ) * (1 << LOD); // 1 << LOD: should be max distance between the vertices
				FIntVector Position = FVoxelUtilities::FloorToInt((Vertex + Offset - Box.Min) / ChunkSize * NumConvexHullsPerAxis);
				Position = FVoxelUtilities::Clamp(Position, FIntVector(0), Size - 1);

				// Avoid adding too many duplicates by checking we're not in the center
				if (OffsetX == 0 && OffsetY == 0 && OffsetZ == 0)
				{
					MainPosition = Position;
				}
				else
				{
					if (Position == MainPosition)
					{
						return;
					}
				}
				ConvexElems[Position.X + Size.X * Position.Y + Size.X * Size.Y * Position.Z].VertexData.Add(Vertex);
			};
			Lambda(0, 0, 0);
			// Iterate possible neighbors to avoid holes between hulls
			Lambda(+1, 0, 0);
			Lambda(-1, 0, 0);
			Lambda(0, +1, 0);
			Lambda(0, -1, 0);
			Lambda(0, 0, +1);
			Lambda(0, 0, -1);
		}
	}

	constexpr int32 Threshold = 8;

	// Merge the hulls until they are big enough
	// This moves the vertices to the end
	for (int32 Index = 0; Index < ConvexElems.Num() - 1; Index++)
	{
		auto& Element = ConvexElems[Index];
		if (Element.VertexData.Num() < Threshold)
		{
			ConvexElems[Index + 1].VertexData.Append(Element.VertexData);
			Element.VertexData.Reset();
		}
	}

	// Remove all empty hulls
	ConvexElems.RemoveAll([](auto& Element) { return Element.VertexData.Num() == 0; });
	if(!ensure(ConvexElems.Num() > 0)) return;

	// Then merge backwards while the last hull isn't big enough
	while (ConvexElems.Last().VertexData.Num() < Threshold && ConvexElems.Num() > 1)
	{
		ConvexElems[ConvexElems.Num() - 2].VertexData.Append(ConvexElems.Last().VertexData);
		ConvexElems.Pop();
	}

	CookResult.ConvexBounds = FBox(ForceInit);
	for (auto& Element : ConvexElems)
	{
		ensure(Element.VertexData.Num() >= 4);
		for (auto& Vertex : Element.VertexData)
		{
			// Transform from component space to root component space, as the root is going to hold the convex meshes
			Vertex = LocalToRoot.TransformPosition(Vertex);
		}
		Element.UpdateElemBox();
		CookResult.ConvexBounds += Element.ElemBox;
	}
#endif
}

EPhysXMeshCookFlags FVoxelAsyncPhysicsCooker::GetCookFlags() const
{
	EPhysXMeshCookFlags CookFlags = EPhysXMeshCookFlags::Default;
	if (!bCleanCollisionMesh)
	{
		CookFlags |= EPhysXMeshCookFlags::DeformableMesh;
	}
	// TODO try and bench CookFlags |= EPhysXMeshCookFlags::DisableActiveEdgePrecompute;
	// TODO: option/check perf
	CookFlags |= EPhysXMeshCookFlags::FastCook;
	return CookFlags;
}