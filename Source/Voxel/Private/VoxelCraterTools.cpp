// Copyright 2018 Phyronnaz

#include "VoxelCraterTools.h"
#include "Misc/QueuedThreadPool.h"

#include "VoxelData.h"
#include "FastNoise.h"
#include "VoxelUtilities.h"
#include "VoxelWorld.h"
#include "VoxelPrivate.h"
#include "Async.h"

FAsyncAddCrater::FAsyncAddCrater(FVoxelData* Data, const FIntVector& LocalPosition, int IntRadius, float Radius, uint8 BlackMaterialIndex, uint8 AddedBlack, float HardnessMultiplier, AVoxelWorld* World)
	: Data(Data)
	, LocalPosition(LocalPosition)
	, IntRadius(IntRadius)
	, Radius(Radius)
	, BlackMaterialIndex(BlackMaterialIndex)
	, AddedBlack(AddedBlack)
	, HardnessMultiplier(HardnessMultiplier)
	, World(World)
{

}

void FAsyncAddCrater::DoThreadedWork()
{
	FValueOctree* LastOctree = nullptr;
	FastNoise Noise;

	const FIntBox Bounds = FIntBox(FIntVector(-IntRadius, -IntRadius, -IntRadius), FIntVector(IntRadius + 1, IntRadius + 1, IntRadius + 1)).TranslateBy(LocalPosition);

	auto Octrees = Data->BeginSet(Bounds);

	for (int X = -IntRadius; X <= IntRadius; X++)
	{
		for (int Y = -IntRadius; Y <= IntRadius; Y++)
		{
			for (int Z = -IntRadius; Z <= IntRadius; Z++)
			{
				const FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);

				float CurrentRadius = FVector(X, Y, Z).Size();
				float Distance = CurrentRadius;

				if (Radius - 2 < Distance && Distance <= Radius + 3)
				{
					float CurrentNoise = Noise.GetWhiteNoise(X / CurrentRadius, Y / CurrentRadius, Z / CurrentRadius);
					Distance -= CurrentNoise;
				}

				if (Distance <= Radius + 2)
				{
					// We want (Radius - Distance) != 0
					const float NoiseValue = (Radius - Distance == 0) ? 0.0001f : 0;
					float Value = FMath::Clamp(Radius - Distance + NoiseValue, -2.f, 2.f) / 2;

					Value *= HardnessMultiplier;

					float OldValue;
					FVoxelMaterial OldMaterial;
					Data->GetValueAndMaterial(CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z, OldValue, OldMaterial);

					bool bValid;
					if (Value > 0)
					{
						bValid = true;
					}
					else
					{
						bValid = FVoxelUtilities::HaveSameSign(OldValue, Value);
					}
					if (bValid)
					{
						if (LIKELY(Data->IsInWorld(CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z)))
						{
							if (OldMaterial.Index1 == BlackMaterialIndex)
							{
								OldMaterial.Alpha = FMath::Clamp<int>(OldMaterial.Alpha - AddedBlack, 0, 255);
							}
							else if (OldMaterial.Index2 == BlackMaterialIndex)
							{
								OldMaterial.Alpha = FMath::Clamp<int>(OldMaterial.Alpha + AddedBlack, 0, 255);
							}
							else if (OldMaterial.Alpha < 128)
							{
								// Index 1 biggest
								OldMaterial.Index2 = BlackMaterialIndex;
								OldMaterial.Alpha = FMath::Clamp<int>(AddedBlack, 0, 255);
							}
							else
							{
								// Index 2 biggest
								OldMaterial.Index1 = BlackMaterialIndex;
								OldMaterial.Alpha = FMath::Clamp<int>(255 - AddedBlack, 0, 255);
							}

							Data->SetValueAndMaterial(CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z, Value, OldMaterial, LastOctree);
						}
					}
				}
			}
		}
	}
	Data->EndSet(Octrees);

	AsyncTask(ENamedThreads::GameThread, [&]() { if (World->IsValidLowLevel()) { World->UpdateChunksOverlappingBox(Bounds); } delete this; });
}

void FAsyncAddCrater::Abandon()
{
	delete this;
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelCraterTools::AddCrater(AVoxelWorld* World, const FVector Position, const float WorldRadius, const uint8 BlackMaterialIndex, const uint8 AddedBlack)
{
	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("AddCrater: World is NULL"));
		return;
	}

	const float Radius = WorldRadius / World->GetVoxelSize();

	// Position in voxel space
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius) + 2;
	const FIntBox Bounds = FIntBox(FIntVector(-IntRadius, -IntRadius, -IntRadius), FIntVector(IntRadius + 1, IntRadius + 1, IntRadius + 1)).TranslateBy(LocalPosition);

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	FastNoise Noise;

	{
		auto Octrees = Data->BeginSet(Bounds);
		for (int X = -IntRadius; X <= IntRadius; X++)
		{
			for (int Y = -IntRadius; Y <= IntRadius; Y++)
			{
				for (int Z = -IntRadius; Z <= IntRadius; Z++)
				{
					const FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);

					float CurrentRadius = FVector(X, Y, Z).Size();
					float Distance = CurrentRadius;

					if (Radius - 2 < Distance && Distance <= Radius + 3)
					{
						float CurrentNoise = Noise.GetWhiteNoise(X / CurrentRadius, Y / CurrentRadius, Z / CurrentRadius);
						Distance -= CurrentNoise;
					}

					if (Distance <= Radius + 2)
					{
						// TODO: is this still valid?
						// We want (Radius - Distance) != 0
						const float NoiseValue = (Radius - Distance == 0) ? 0.0001f : 0;
						float Value = FMath::Clamp(Radius - Distance + NoiseValue, -2.f, 2.f) / 2;

						float OldValue;
						FVoxelMaterial OldMaterial;
						Data->GetValueAndMaterial(CurrentPosition, OldValue, OldMaterial);

						if (Value > 0 || FVoxelUtilities::HaveSameSign(OldValue, Value))
						{
							if (LIKELY(Data->IsInWorld(CurrentPosition)))
							{
								if (OldMaterial.Index1 == BlackMaterialIndex)
								{
									OldMaterial.Alpha = FMath::Clamp<int>(OldMaterial.Alpha - AddedBlack, 0, 255);
								}
								else if (OldMaterial.Index2 == BlackMaterialIndex)
								{
									OldMaterial.Alpha = FMath::Clamp<int>(OldMaterial.Alpha + AddedBlack, 0, 255);
								}
								else if (OldMaterial.Alpha < 128)
								{
									// Index 1 biggest
									OldMaterial.Index2 = BlackMaterialIndex;
									OldMaterial.Alpha = FMath::Clamp<int>(AddedBlack, 0, 255);
								}
								else
								{
									// Index 2 biggest
									OldMaterial.Index1 = BlackMaterialIndex;
									OldMaterial.Alpha = FMath::Clamp<int>(255 - AddedBlack, 0, 255);
								}

								Data->SetValueAndMaterial(CurrentPosition, Value, OldMaterial, LastOctree);
							}
						}
					}
				}
			}
		}
		Data->EndSet(Octrees);
	}
	World->UpdateChunksOverlappingBox(Bounds);
}

void UVoxelCraterTools::AddCraterMultithreaded(AVoxelWorld* World, const FVector Position, const float WorldRadius, const uint8 BlackMaterialIndex, const uint8 AddedBlack)
{
	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("AddCreater: World is NULL"));
		return;
	}

	const float Radius = WorldRadius / World->GetVoxelSize();

	// Position in voxel space
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius) + 2;

	FVoxelData* Data = World->GetData();

	auto Task = new FAsyncAddCrater(Data, LocalPosition, IntRadius, Radius, BlackMaterialIndex, AddedBlack, 1, World);
	World->GetAsyncTasksThreadPool()->AddQueuedWork(Task);
}