// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelWorldGeneratorTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelWorldGenerator.h"
#include "VoxelWorldGeneratorInstance.h"
#include "VoxelMessages.h"
#include "VoxelWorldGeneratorUtilities.h"

TVoxelTexture<float> UVoxelWorldGeneratorTools::CreateTextureFromWorldGeneratorImpl(
	const FVoxelWorldGeneratorInstance& WorldGenerator, 
	FName OutputName, 
	const FIntPoint& Start, 
	const FIntPoint& Size,
	float Scale)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Size.X * Size.Y);
	check(Size.X > 0 && Size.Y > 0);
	
	const auto FunctionPtr = WorldGenerator.GetOutputsPtrMap<v_flt>().FindRef(OutputName);
	if (!ensure(FunctionPtr)) return {};
	
	auto Texture = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
	Texture->SetSize(Size.X, Size.Y);
	for (int32 X = 0; X < Size.X; X++)
	{
		for (int32 Y = 0; Y < Size.Y; Y++)
		{
			const float Value = (WorldGenerator.*FunctionPtr)(Scale * (Start.X + X), Scale * (Start.Y + Y), 0, 0, FVoxelItemStack::Empty);
			Texture->SetValue(X, Y, Value);
		}
	}
	return TVoxelTexture<float>(Texture);
}

inline TVoxelSharedPtr<FVoxelWorldGeneratorInstance> SetupWorldGenerator(
	const FString& FunctionName,
	UVoxelWorldGenerator* WorldGenerator,
	const TMap<FName, int32>& Seeds,
	FName OutputName,
	float VoxelSize,
	int32 SizeX,
	int32 SizeY)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!WorldGenerator)
	{
		FVoxelMessages::Error( FUNCTION_ERROR_IMPL(FunctionName, "Invalid WorldGenerator!"));
		return {};
	}
	if (SizeX <= 0 || SizeY <= 0)
	{
		FVoxelMessages::Error( FUNCTION_ERROR_IMPL(FunctionName, "Invalid Size!"));
		return {};
	}

	auto Instance = WorldGenerator->GetInstance();
	if (!Instance->GetOutputsPtrMap<v_flt>().Contains(OutputName))
	{
		FVoxelMessages::Error(FUNCTION_ERROR_IMPL(FunctionName, FVoxelUtilities::GetMissingWorldGeneratorOutputErrorString<v_flt>(OutputName, *Instance)));
		return {};
	}

	const FVoxelWorldGeneratorInit Init(Seeds, VoxelSize, FMath::Max(SizeX, SizeY), EVoxelMaterialConfig::RGB, nullptr, nullptr);
	Instance->Init(Init);

	return Instance;
}

void UVoxelWorldGeneratorTools::CreateTextureFromWorldGenerator(
	FVoxelFloatTexture& OutTexture, 
	UVoxelWorldGenerator* WorldGenerator, 
	const TMap<FName, int32>& Seeds, 
	FName OutputName,
	int32 SizeX, 
	int32 SizeY,
	float Scale,
	int32 StartX,
	int32 StartY,
	float VoxelSize)
{
	VOXEL_FUNCTION_COUNTER();

	const auto Instance = SetupWorldGenerator(__FUNCTION__, WorldGenerator, Seeds, OutputName, VoxelSize, SizeX, SizeY);
	if (!Instance) return;

	OutTexture.Texture = CreateTextureFromWorldGeneratorImpl(*Instance, OutputName, FIntPoint(StartX, StartY), FIntPoint(SizeX, SizeY), Scale);
}

void UVoxelWorldGeneratorTools::CreateTextureFromWorldGeneratorAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	FVoxelFloatTexture& OutTexture,
	UVoxelWorldGenerator* WorldGenerator,
	const TMap<FName, int32>& Seeds,
	FName OutputName,
	int32 SizeX,
	int32 SizeY,
	float Scale,
	int32 StartX,
	int32 StartY,
	float VoxelSize,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();

	const auto Instance = SetupWorldGenerator(__FUNCTION__, WorldGenerator, Seeds, OutputName, VoxelSize, SizeX, SizeY);
	if (!Instance) return;

	FVoxelToolHelpers::StartAsyncLatentAction_WithoutWorld_WithValue(
		WorldContextObject,
		LatentInfo,
		__FUNCTION__,
		bHideLatentWarnings,
		OutTexture,
		[=](FVoxelFloatTexture& Texture)
		{
			Texture.Texture = CreateTextureFromWorldGeneratorImpl(*Instance, OutputName, FIntPoint(StartX, StartY), FIntPoint(SizeX, SizeY), Scale);
		});
}