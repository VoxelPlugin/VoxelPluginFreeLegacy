// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelThread.h"
#include "VoxelData.h"
#include "VoxelChunk.h"
#include "VoxelPolygonizer.h"
#include "InstancedStaticMesh.h"
#include "Kismet/KismetMathLibrary.h"


MeshBuilderAsyncTask::MeshBuilderAsyncTask(uint8 Depth, VoxelData* Data, FIntVector Position, TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes, bool bComputeTransitions, AVoxelChunk* Chunk)
	: bComputeTransitions(bComputeTransitions)
	, Chunk(Chunk)
{
	Worker = new VoxelPolygonizer(Depth, Data, Position, ChunkHasHigherRes);
}

MeshBuilderAsyncTask::~MeshBuilderAsyncTask()
{
	delete Worker;
}

void MeshBuilderAsyncTask::DoThreadedWork()
{
	Worker->CreateSection(Section, bComputeTransitions);

	FFunctionGraphTask::CreateAndDispatchWhenReady([this]() { Chunk->OnMeshComplete(); }, TStatId(), NULL, ENamedThreads::GameThread);
}

void MeshBuilderAsyncTask::Abandon()
{
	delete this;
}

FProcMeshSection& MeshBuilderAsyncTask::GetSection()
{
	return Section;
}




FoliageBuilderAsyncTask::FoliageBuilderAsyncTask(FProcMeshSection Section, FGrassVariety GrassVariety, uint8 Material, FTransform ChunkTransform, float VoxelSize, FIntVector ChunkPosition, int Seed, AVoxelChunk* Chunk)
	: Section(Section)
	, GrassVariety(GrassVariety)
	, Material(Material)
	, ChunkTransform(ChunkTransform)
	, VoxelSize(VoxelSize)
	, ChunkPosition(ChunkPosition)
	, Seed(Seed)
	, Chunk(Chunk)
{

}

void FoliageBuilderAsyncTask::DoThreadedWork()
{
	// TODO: set num
	TArray<FMatrix> InstanceTransforms;

	const float VoxelTriangleArea = (VoxelSize * VoxelSize) / 2;
	const float MeanGrassPerTrig = GrassVariety.GrassDensity * VoxelTriangleArea / 100000 /* 10m² in cm² */;

	for (int Index = 0; Index < Section.ProcIndexBuffer.Num(); Index += 3)
	{
		int IndexA = Section.ProcIndexBuffer[Index];
		int IndexB = Section.ProcIndexBuffer[Index + 1];
		int IndexC = Section.ProcIndexBuffer[Index + 2];

		FVoxelMaterial MatA = FVoxelMaterial(Section.ProcVertexBuffer[IndexA].Color);
		FVoxelMaterial MatB = FVoxelMaterial(Section.ProcVertexBuffer[IndexB].Color);
		FVoxelMaterial MatC = FVoxelMaterial(Section.ProcVertexBuffer[IndexC].Color);

		const float ToleranceZone = 0.25f;
		if ((Material == MatA.Index1 && MatA.Alpha < 1 - ToleranceZone) || (Material == MatA.Index2 && ((MatA.Alpha > ToleranceZone) || (MatA.Index1 == MatA.Index2))) ||
			(Material == MatB.Index1 && MatB.Alpha < 1 - ToleranceZone) || (Material == MatB.Index2 && ((MatB.Alpha > ToleranceZone) || (MatB.Index1 == MatB.Index2))) ||
			(Material == MatC.Index1 && MatC.Alpha < 1 - ToleranceZone) || (Material == MatC.Index2 && ((MatC.Alpha > ToleranceZone) || (MatC.Index1 == MatC.Index2))))
		{

			FVector A = Section.ProcVertexBuffer[IndexA].Position;
			FVector B = Section.ProcVertexBuffer[IndexB].Position;
			FVector C = Section.ProcVertexBuffer[IndexC].Position;

			FVector N = (Section.ProcVertexBuffer[IndexA].Normal +
				Section.ProcVertexBuffer[IndexB].Normal +
				Section.ProcVertexBuffer[IndexC].Normal) / 3;

			FVector X = B - A;
			FVector Y = C - A;

			const float SizeX = X.Size();
			const float SizeY = Y.Size();

			X.Normalize();
			Y.Normalize();

			FVector ExactCenter = (A + B + C) / 3 * 1000000;
			FIntVector IntCenter = ChunkPosition + FIntVector(FMath::RoundToInt(ExactCenter.X), FMath::RoundToInt(ExactCenter.Y), FMath::RoundToInt(ExactCenter.Z));

			FRandomStream Stream(Seed * (IntCenter.X + Seed) * (IntCenter.Y + Seed * Seed) * (IntCenter.Z + Seed * Seed * Seed));

			int Count = 2 * MeanGrassPerTrig;
			if (Stream.GetFraction() < 2 * MeanGrassPerTrig - Count)
			{
				Count++;
			}

			for (float i = 0.5; i < Count; i++)
			{
				if (Stream.GetFraction() > 0.5f)
				{
					float CoordX = Stream.GetFraction() * SizeY;
					float CoordY = Stream.GetFraction() * SizeX;

					if (SizeY - CoordX * SizeY / SizeX < CoordY)
					{
						CoordX = SizeX - CoordX;
						CoordY = SizeY - CoordY;
					}

					FVector P = A + X * CoordX + Y * CoordY;

					FVector Scale(1.0f);

					switch (GrassVariety.Scaling)
					{
					case EGrassScaling::Uniform:
						Scale.X = GrassVariety.ScaleX.Interpolate(Stream.GetFraction());
						Scale.Y = Scale.X;
						Scale.Z = Scale.X;
						break;
					case EGrassScaling::Free:
						Scale.X = GrassVariety.ScaleX.Interpolate(Stream.GetFraction());
						Scale.Y = GrassVariety.ScaleY.Interpolate(Stream.GetFraction());
						Scale.Z = GrassVariety.ScaleZ.Interpolate(Stream.GetFraction());
						break;
					case EGrassScaling::LockXY:
						Scale.X = GrassVariety.ScaleX.Interpolate(Stream.GetFraction());
						Scale.Y = Scale.X;
						Scale.Z = GrassVariety.ScaleZ.Interpolate(FMath::RandRange(0.f, 1.f));
						break;
					default:
						check(0);
					}

					FRotator Rotation = GrassVariety.AlignToSurface ? UKismetMathLibrary::MakeRotFromZX(N, Stream.GetFraction() * X + Stream.GetFraction() * Y) : FRotator::ZeroRotator;

					InstanceTransforms.Add(FTransform(Rotation, ChunkTransform.GetScale3D() * P, Scale).ToMatrixWithScale());
				}
			}
		}
	}


	if (InstanceTransforms.Num())
	{
		InstanceBuffer.AllocateInstances(InstanceTransforms.Num(), true);
		for (int32 InstanceIndex = 0; InstanceIndex < InstanceTransforms.Num(); InstanceIndex++)
		{
			InstanceBuffer.SetInstance(InstanceIndex, InstanceTransforms[InstanceIndex], 0);
		}

		TArray<int32> SortedInstances;
		TArray<int32> InstanceReorderTable;
		UHierarchicalInstancedStaticMeshComponent::BuildTreeAnyThread(InstanceTransforms, GrassVariety.GrassMesh->GetBounds().GetBox(), ClusterTree, SortedInstances, InstanceReorderTable, OutOcclusionLayerNum, /*DesiredInstancesPerLeaf*/1);

		//SORT
		// in-place sort the instances
		const uint32 InstanceStreamSize = InstanceBuffer.GetStride();
		FInstanceStream32 SwapBuffer;
		check(sizeof(SwapBuffer) >= InstanceStreamSize);

		for (int32 FirstUnfixedIndex = 0; FirstUnfixedIndex < InstanceTransforms.Num(); FirstUnfixedIndex++)
		{
			int32 LoadFrom = SortedInstances[FirstUnfixedIndex];
			if (LoadFrom != FirstUnfixedIndex)
			{
				check(LoadFrom > FirstUnfixedIndex);
				FMemory::Memcpy(&SwapBuffer, InstanceBuffer.GetInstanceWriteAddress(FirstUnfixedIndex), InstanceStreamSize);
				FMemory::Memcpy(InstanceBuffer.GetInstanceWriteAddress(FirstUnfixedIndex), InstanceBuffer.GetInstanceWriteAddress(LoadFrom), InstanceStreamSize);
				FMemory::Memcpy(InstanceBuffer.GetInstanceWriteAddress(LoadFrom), &SwapBuffer, InstanceStreamSize);

				int32 SwapGoesTo = InstanceReorderTable[FirstUnfixedIndex];
				check(SwapGoesTo > FirstUnfixedIndex);
				check(SortedInstances[SwapGoesTo] == FirstUnfixedIndex);
				SortedInstances[SwapGoesTo] = LoadFrom;
				InstanceReorderTable[LoadFrom] = SwapGoesTo;

				InstanceReorderTable[FirstUnfixedIndex] = FirstUnfixedIndex;
				SortedInstances[FirstUnfixedIndex] = FirstUnfixedIndex;
			}
		}
	}

	FFunctionGraphTask::CreateAndDispatchWhenReady([this]() { Chunk->OnFoliageComplete(); }, TStatId(), NULL, ENamedThreads::GameThread);
}

void FoliageBuilderAsyncTask::Abandon()
{
	delete this;
}