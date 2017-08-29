#include "VoxelPrivatePCH.h"
#include "VoxelAsset.h"
#include "VoxelData.h"
#include "LandscapeDataAccess.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "DrawDebugHelpers.h"
#include "EmptyWorldGenerator.h"
#include "Engine/World.h"

void AVoxelAsset::CreateFromLandcape()
{
	if (Landscape)
	{
		int MipLevel = 0;
		int ComponentSize = 0;
		int Count = 0;

		for (auto Component : Landscape->GetLandscapeActor()->LandscapeComponents)
		{
			int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;
			Count++;
			if (ComponentSize == 0)
			{
				ComponentSize = Size;
			}
			else
			{
				check(ComponentSize == Size);
			}

		}

		check(FMath::RoundToInt(FMath::Sqrt(Count)) * FMath::RoundToInt(FMath::Sqrt(Count)) == Count);
		int TotalSize = FMath::RoundToInt(FMath::Sqrt(Count)) * ComponentSize;

		TArray<float> Values;
		Values.SetNum(TotalSize * TotalSize);

		for (auto Component : Landscape->GetLandscapeActor()->LandscapeComponents)
		{
			FLandscapeComponentDataInterface DataInterface(Component, MipLevel);
			int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;

			for (int X = 0; X < Size; X++)
			{
				for (int Y = 0; Y < Size; Y++)
				{
					FVector Vertex = DataInterface.GetWorldVertex(X, Y);
					FVector LocalVertex = (Vertex - Landscape->GetActorLocation()) / Component->GetComponentTransform().GetScale3D();
					Values[LocalVertex.X + TotalSize * LocalVertex.Y] = Vertex.Z;
				}
			}
		}

		int Depth = FMath::CeilToInt(FMath::Log2(TotalSize)) - 4;
		Width = 16 << Depth;
		Heights.SetNum(Width * Width);

		for (int X = 0; X < TotalSize; X++)
		{
			for (int Y = 0; Y < TotalSize; Y++)
			{
				Heights[X + Width * Y] = Values[X + TotalSize * Y];
			}
		}
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Invalid landscape"));
	}
}
