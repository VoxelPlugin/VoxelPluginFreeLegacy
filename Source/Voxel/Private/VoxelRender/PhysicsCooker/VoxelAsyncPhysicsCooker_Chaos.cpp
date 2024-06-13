// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelRender/PhysicsCooker/VoxelAsyncPhysicsCooker_Chaos.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelUtilities/VoxelMathUtilities.h"

#include "PhysicsEngine/BodySetup.h"

#include "Chaos/ImplicitObject.h"
#include "Chaos/CollisionConvexMesh.h"
#include "Chaos/TriangleMeshImplicitObject.h"

FVoxelAsyncPhysicsCooker_Chaos::FVoxelAsyncPhysicsCooker_Chaos(UVoxelProceduralMeshComponent* Component)
	: IVoxelAsyncPhysicsCooker(Component)
{
}

bool FVoxelAsyncPhysicsCooker_Chaos::Finalize(UBodySetup& BodySetup, FVoxelProceduralMeshComponentMemoryUsage& OutMemoryUsage)
{
#if TRACK_CHAOS_GEOMETRY
	for (auto& TriMesh : TriMeshes)
	{
		TriMesh->Track(Chaos::MakeSerializable(TriMesh), "Voxel Mesh");
	}
#endif

	// Force trimesh collisions off
	for (auto& TriMesh : TriMeshes)
	{
		TriMesh->SetDoCollide(false);
	}

#if VOXEL_ENGINE_VERSION >= 504
	BodySetup.TriMeshGeometries = MoveTemp(TriMeshes);
#else
	BodySetup.ChaosTriMeshes = MoveTemp(TriMeshes);
#endif
	BodySetup.bCreatedPhysicsMeshes = true;

	return true;
}

void FVoxelAsyncPhysicsCooker_Chaos::CookMesh()
{
	if (CollisionTraceFlag != ECollisionTraceFlag::CTF_UseComplexAsSimple)
	{
		ensure(false);
	}
	if (CollisionTraceFlag != ECollisionTraceFlag::CTF_UseSimpleAsComplex)
	{
		CreateTriMesh();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelAsyncPhysicsCooker_Chaos::CreateTriMesh()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
				
	int32 NumIndices = 0;
	int32 NumVertices = 0;
	for (auto& Buffer : Buffers)
	{
		NumIndices += Buffer->GetNumIndices();
		NumVertices += Buffer->GetNumVertices();
	}

	const auto Process = [&](auto& Triangles)
	{
		Chaos::TParticles<Chaos::FRealSingle, 3> Particles;

		{
			VOXEL_ASYNC_SCOPE_COUNTER("Copy data from buffers");

			{
				VOXEL_ASYNC_SCOPE_COUNTER("Allocate");
				ensure(NumIndices % 3 == 0);
				Triangles.SetNumUninitialized(NumIndices / 3);
				Particles.AddParticles(NumVertices);
			}

			int32 IndexIndex = 0;
			int32 VertexIndex = 0;
			for (int32 SectionIndex = 0; SectionIndex < Buffers.Num(); SectionIndex++)
			{
				auto& Buffer = *Buffers[SectionIndex];

				const int32 VertexOffset = VertexIndex;

				{
					VOXEL_ASYNC_SCOPE_COUNTER("Copy vertices");
					
					auto& PositionBuffer = Buffer.VertexBuffers.PositionVertexBuffer;
					for (uint32 Index = 0; Index < PositionBuffer.GetNumVertices(); Index++)
					{
#if VOXEL_ENGINE_VERSION >= 504
						Particles.SetX(VertexIndex++, PositionBuffer.VertexPosition(Index));
#else
						Particles.X(VertexIndex++) = PositionBuffer.VertexPosition(Index);
#endif
					}
				}

				{
					VOXEL_ASYNC_SCOPE_COUNTER("Copy triangles");
					
					auto& IndexBuffer = Buffer.IndexBuffer;

					ensure(IndexBuffer.GetNumIndices() % 3 == 0);
					const int32 NumTriangles = IndexBuffer.GetNumIndices() / 3;

					const auto Lambda = [&](const auto* RESTRICT Data)
					{
						for (int32 Index = 0; Index < NumTriangles; Index++)
						{
							checkVoxelSlow(3 * Index + 2 < IndexBuffer.GetNumIndices());

							const Chaos::TVector<int32, 3> Triangle{
									int32(Data[3 * Index + 2]) + VertexOffset,
									int32(Data[3 * Index + 1]) + VertexOffset,
									int32(Data[3 * Index + 0]) + VertexOffset
							};

							FVoxelUtilities::Get(Triangles, IndexIndex++) = Triangle;

#if VOXEL_DEBUG
#if VOXEL_ENGINE_VERSION >= 504
							const auto A = Particles.GetX(Triangle.X);
							const auto B = Particles.GetX(Triangle.Y);
							const auto C = Particles.GetX(Triangle.Z);
#else

							const auto A = Particles.X(Triangle.X);
							const auto B = Particles.X(Triangle.Y);
							const auto C = Particles.X(Triangle.Z);
#endif
							ensure(Chaos::FConvexBuilder::IsValidTriangle(A, B, C));
#endif
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
			check(IndexIndex == Triangles.Num());
			check(VertexIndex == Particles.Size());
		}

		TArray<uint16> MaterialIndices;
		
		VOXEL_ASYNC_SCOPE_COUNTER("Build Tri Mesh");
		TriMeshes.Emplace(new Chaos::FTriangleMeshImplicitObject(MoveTemp(Particles), MoveTemp(Triangles), MoveTemp(MaterialIndices)));
	};
	
	if (NumVertices < TNumericLimits<uint16>::Max())
	{
		TArray<Chaos::TVector<uint16, 3>> TrianglesSmallIdx;
		Process(TrianglesSmallIdx);
	}
	else
	{
		TArray<Chaos::TVector<int32, 3>> TrianglesLargeIdx;
		Process(TrianglesLargeIdx);
	}
}