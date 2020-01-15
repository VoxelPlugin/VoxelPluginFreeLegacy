// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelWorldGeneratorTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelWorldGenerator.h"
#include "VoxelWorldGeneratorInstance.h"
#include "VoxelMessages.h"

void UVoxelWorldGeneratorTools::CreateTextureFromWorldGenerator(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	FVoxelFloatTexture& OutTexture,
	UVoxelWorldGenerator* WorldGenerator,
	const TMap<FName, int32>& Seeds,
	FName OutputName,
	float VoxelSize,
	int32 StartX,
	int32 StartY,
	int32 SizeX,
	int32 SizeY,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_FUNCTION_COUNTER(SizeX * SizeY);
	
	if (!WorldGenerator)
	{
		FVoxelMessages::Error("CreateTextureFromWorldGenerator: Invalid WorldGenerator!");
		return;
	}
	if (SizeX <= 0 || SizeY <= 0)
	{
		FVoxelMessages::Error("CreateTextureFromWorldGenerator: Invalid size!");
		return;
	}

	auto Instance = WorldGenerator->GetInstance();
	auto FunctionPtr = Instance->FloatOutputsPtr.FindRef(OutputName);
	if (!FunctionPtr)
	{
		FString Names;
		for (auto& It : Instance->FloatOutputsPtr)
		{
			if (!Names.IsEmpty())
			{
				Names += ", ";
			}
			Names += It.Key.ToString();
		}
		FVoxelMessages::Error(FString::Printf(
			TEXT("CreateTextureFromWorldGenerator: No output named %s and with type float found! Valid names: %s"),
			*OutputName.ToString(),
			*Names));
		return;
	}

	const FVoxelWorldGeneratorInit Init(Seeds, VoxelSize, FMath::Max(SizeX, SizeY), EVoxelMaterialConfig::RGB, nullptr, nullptr);
	Instance->Init(Init);

	FVoxelToolHelpers::StartAsyncLatentAction_WithoutWorld_WithValue<FVoxelFloatTexture>(
		WorldContextObject,
		LatentInfo,
		FUNCTION_FNAME,
		bHideLatentWarnings,
		OutTexture,
		[SizeX, SizeY, FunctionPtr, Instance, StartX, StartY](FVoxelFloatTexture& InTexture)
		{
			auto Texture = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
			Texture->SizeX = SizeX;
			Texture->SizeY = SizeY;
			Texture->TextureData.SetNumUninitialized(SizeX * SizeY);
			for (int32 X = 0; X < SizeX; X++)
			{
				for (int32 Y = 0; Y < SizeY; Y++)
				{
					const float Value = (Instance.Get().*FunctionPtr)(StartX + X, StartY + Y, 0, 0, FVoxelItemStack::Empty);
					const int32 Index = X + SizeX * Y;
					Texture->TextureData[Index] = Value;

					if (Index == 0)
					{
						Texture->Min = Value;
						Texture->Max = Value;
					}
					else
					{
						Texture->Min = FMath::Min(Texture->Min, Value);
						Texture->Max = FMath::Max(Texture->Max, Value);
					}
				}
			}
			InTexture.Texture = TVoxelTexture<float>(Texture);
		});
}