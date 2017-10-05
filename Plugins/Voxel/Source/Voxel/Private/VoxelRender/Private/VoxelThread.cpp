// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelThread.h"
#include "VoxelData.h"
#include "VoxelChunkComponent.h"
#include "VoxelPolygonizer.h"
#include "InstancedStaticMesh.h"
#include "Kismet/KismetMathLibrary.h"
#include "GenericPlatformProcess.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"


FAsyncFoliageTask::FAsyncFoliageTask(FProcMeshSection Section, FVoxelGrassVariety GrassVariety, uint8 Material, AVoxelWorld* World, FIntVector ChunkPosition, UVoxelChunkComponent* Chunk)
	: Section(Section)
	, GrassVariety(GrassVariety)
	, Material(Material)
	, VoxelSize(World->GetVoxelSize())
	, ChunkPosition(ChunkPosition)
	, Seed(World->GetSeed())
	, Generator(World->GetWorldGenerator())
	, Chunk(Chunk)
	, World(World)
{

}

void FAsyncFoliageTask::DoWork()
{
	std::forward_list<FMatrix> InstanceTransformsList;
	uint32 InstanceTransformsCount = 0;

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

			const FVector A = Section.ProcVertexBuffer[IndexA].Position;
			const FVector B = Section.ProcVertexBuffer[IndexB].Position;
			const FVector C = Section.ProcVertexBuffer[IndexC].Position;

			const FVector Normal = (Section.ProcVertexBuffer[IndexA].Normal + Section.ProcVertexBuffer[IndexB].Normal + Section.ProcVertexBuffer[IndexC].Normal) / 3;

			FVector X = B - A;
			FVector Y = C - A;

			const float SizeX = X.Size();
			const float SizeY = Y.Size();

			X.Normalize();
			Y.Normalize();

			// Not exact, but all we want is a random generator that depends only on position
			const FVector ExactCenter = (A + B + C) / 3 * 1000000;
			const FIntVector IntCenter = ChunkPosition + FIntVector(FMath::RoundToInt(ExactCenter.X), FMath::RoundToInt(ExactCenter.Y), FMath::RoundToInt(ExactCenter.Z));

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

					const FVector CurrentRelativePosition = A + X * CoordX + Y * CoordY;


					// Compute scale

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


					// Compute rotation

					FRotator Rotation;
					if (GrassVariety.AlignToSurface)
					{
						if (GrassVariety.RandomRotation)
						{
							Rotation = UKismetMathLibrary::MakeRotFromZX(Normal, Stream.GetFraction() * X + Stream.GetFraction() * Y);
						}
						else
						{
							Rotation = UKismetMathLibrary::MakeRotFromZX(Normal, X + Y);
						}
					}
					else
					{
						Rotation = FRotator::ZeroRotator;
					}

					const FVector GrassUp = Rotation.RotateVector(FVector::UpVector).GetSafeNormal();
					const FIntVector P = ChunkPosition + FIntVector(FMath::RoundToInt(CurrentRelativePosition.X), FMath::RoundToInt(CurrentRelativePosition.Y), FMath::RoundToInt(CurrentRelativePosition.Z));
					const FVector WorldUp = Generator->GetUpVector(P.X, P.Y, P.Z).GetSafeNormal();

					const float Angle = FMath::Acos(FVector::DotProduct(GrassUp, WorldUp));

					if (Angle < GrassVariety.MaxAngleWithWorldUp)
					{
						InstanceTransformsList.push_front(FTransform(Rotation, VoxelSize * CurrentRelativePosition, Scale).ToMatrixWithScale());
						InstanceTransformsCount++;
					}
				}
			}
		}
	}


	if (InstanceTransformsCount)
	{
		TArray<FMatrix> InstanceTransforms;
		InstanceTransforms.SetNumUninitialized(InstanceTransformsCount);

		InstanceBuffer.AllocateInstances(InstanceTransformsCount, true);
		int32 InstanceIndex = 0;
		for (auto InstanceTransform : InstanceTransformsList)
		{
			InstanceBuffer.SetInstance(InstanceIndex, InstanceTransform, 0);

			InstanceTransforms[InstanceIndex] = InstanceTransform;
			InstanceIndex++;
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

	Chunk->OnFoliageComplete();
}




FAsyncPolygonizerTask::FAsyncPolygonizerTask(FVoxelPolygonizer* InBuilder, UVoxelChunkComponent* Chunk)
	: Builder(InBuilder)
	, Chunk(Chunk)
{
}

void FAsyncPolygonizerTask::DoWork()
{
	FProcMeshSection Section = FProcMeshSection();
	Builder->CreateSection(Section);

	Chunk->OnMeshComplete(Section);
}