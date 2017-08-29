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
		TArray<FColor> Colors;
		Values.SetNum(TotalSize * TotalSize);
		Colors.SetNum(TotalSize * TotalSize);

		for (auto Component : Landscape->GetLandscapeActor()->LandscapeComponents)
		{
			FLandscapeComponentDataInterface DataInterface(Component, MipLevel);
			int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;

			TArray<uint8> Weightmap1;
			TArray<uint8> Weightmap2;
			TArray<uint8> Weightmap3;
			DataInterface.GetWeightmapTextureData(LayerInfo1, Weightmap1);
			DataInterface.GetWeightmapTextureData(LayerInfo2, Weightmap2);
			DataInterface.GetWeightmapTextureData(LayerInfo3, Weightmap3);

			int32 WeightmapSize = ((Component->SubsectionSizeQuads + 1) * Component->NumSubsections) >> MipLevel;

			for (int X = 0; X < Size; X++)
			{
				for (int Y = 0; Y < Size; Y++)
				{
					FVector Vertex = DataInterface.GetWorldVertex(X, Y);
					FVector LocalVertex = (Vertex - Landscape->GetActorLocation()) / Component->GetComponentTransform().GetScale3D();
					Values[LocalVertex.X + TotalSize * LocalVertex.Y] = Vertex.Z;

					uint8 Weight1 = 0;
					uint8 Weight2 = 0;
					uint8 Weight3 = 0;
					if (Weightmap1.Num())
					{
						Weight1 = Weightmap1[X + WeightmapSize * Y];
					}
					if (Weightmap2.Num())
					{
						Weight2 = Weightmap2[X + WeightmapSize * Y];
					}
					if (Weightmap3.Num())
					{
						Weight3 = Weightmap3[X + WeightmapSize * Y];
					}
					Colors[LocalVertex.X + TotalSize * LocalVertex.Y] = FColor(Weight1, Weight2, Weight3);
				}
			}
		}

		int Depth = FMath::CeilToInt(FMath::Log2(TotalSize)) - 4;
		Width = 16 << Depth;

		Heights.SetNum(Width * Width);
		Weights.SetNum(Width * Width);

		for (int X = 0; X < TotalSize; X++)
		{
			for (int Y = 0; Y < TotalSize; Y++)
			{
				Heights[X + Width * Y] = Values[X + TotalSize * Y];
				Weights[X + Width * Y] = Colors[X + TotalSize * Y];
			}
		}
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Invalid landscape"));
	}
}
