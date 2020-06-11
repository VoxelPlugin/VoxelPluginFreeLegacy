// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelWorldGeneratorTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.h"
#include "VoxelMessages.h"
#include "VoxelUtilities/VoxelWorldGeneratorUtilities.h"

template<typename T>
TVoxelTexture<T> UVoxelWorldGeneratorTools::CreateTextureFromWorldGeneratorImpl(
	const FVoxelWorldGeneratorInstance& WorldGenerator, 
	FName OutputName, 
	const FIntPoint& Start, 
	const FIntPoint& Size,
	float Scale)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Size.X * Size.Y);
	check(Size.X > 0 && Size.Y > 0);

	using TWorldGeneratorType = typename TChooseClass<TIsSame<T, float>::Value, v_flt, T>::Result;
	
	const auto FunctionPtr = WorldGenerator.GetOutputsPtrMap<TWorldGeneratorType>().FindRef(OutputName);
	if (!ensure(FunctionPtr)) return {};
	
	const auto Texture = MakeVoxelShared<typename TVoxelTexture<T>::FTextureData>();
	Texture->SetSize(Size.X, Size.Y);
	for (int32 X = 0; X < Size.X; X++)
	{
		for (int32 Y = 0; Y < Size.Y; Y++)
		{
			const auto Value = (WorldGenerator.*FunctionPtr)(Scale * (Start.X + X), Scale * (Start.Y + Y), 0, 0, FVoxelItemStack::Empty);
			Texture->SetValue(X, Y, T(Value));
		}
	}
	return TVoxelTexture<T>(Texture);
}

template VOXEL_API TVoxelTexture<float> UVoxelWorldGeneratorTools::CreateTextureFromWorldGeneratorImpl<float>(
	const FVoxelWorldGeneratorInstance& WorldGenerator,
	FName OutputName,
	const FIntPoint& Start,
	const FIntPoint& Size,
	float Scale);

template VOXEL_API TVoxelTexture<FColor> UVoxelWorldGeneratorTools::CreateTextureFromWorldGeneratorImpl<FColor>(
	const FVoxelWorldGeneratorInstance& WorldGenerator,
	FName OutputName,
	const FIntPoint& Start,
	const FIntPoint& Size,
	float Scale);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
TVoxelSharedPtr<FVoxelWorldGeneratorInstance> SetupWorldGenerator(
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

	using TWorldGeneratorType = typename TChooseClass<TIsSame<T, float>::Value, v_flt, T>::Result;

	auto Instance = WorldGenerator->GetInstance();
	if (!Instance->GetOutputsPtrMap<TWorldGeneratorType>().Contains(OutputName))
	{
		FVoxelMessages::Error(FUNCTION_ERROR_IMPL(FunctionName, FVoxelUtilities::GetMissingWorldGeneratorOutputErrorString<TWorldGeneratorType>(OutputName, *Instance)));
		return {};
	}

	const FVoxelWorldGeneratorInit Init(Seeds, VoxelSize, FMath::Max(SizeX, SizeY), EVoxelMaterialConfig::RGB, nullptr, nullptr);
	Instance->Init(Init);

	return Instance;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelWorldGeneratorTools::CreateFloatTextureFromWorldGenerator(
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

	const auto Instance = SetupWorldGenerator<float>(__FUNCTION__, WorldGenerator, Seeds, OutputName, VoxelSize, SizeX, SizeY);
	if (!Instance) return;

	OutTexture.Texture = CreateTextureFromWorldGeneratorImpl<float>(*Instance, OutputName, FIntPoint(StartX, StartY), FIntPoint(SizeX, SizeY), Scale);
}

void UVoxelWorldGeneratorTools::CreateFloatTextureFromWorldGeneratorAsync(
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

	const auto Instance = SetupWorldGenerator<float>(__FUNCTION__, WorldGenerator, Seeds, OutputName, VoxelSize, SizeX, SizeY);
	if (!Instance) return;

	FVoxelToolHelpers::StartAsyncLatentAction_WithoutWorld_WithValue(
		WorldContextObject,
		LatentInfo,
		__FUNCTION__,
		bHideLatentWarnings,
		OutTexture,
		[=](FVoxelFloatTexture& Texture)
		{
			Texture.Texture = CreateTextureFromWorldGeneratorImpl<float>(*Instance, OutputName, FIntPoint(StartX, StartY), FIntPoint(SizeX, SizeY), Scale);
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelWorldGeneratorTools::CreateColorTextureFromWorldGenerator(
	FVoxelColorTexture& OutTexture, 
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

	const auto Instance = SetupWorldGenerator<FColor>(__FUNCTION__, WorldGenerator, Seeds, OutputName, VoxelSize, SizeX, SizeY);
	if (!Instance) return;

	OutTexture.Texture = CreateTextureFromWorldGeneratorImpl<FColor>(*Instance, OutputName, FIntPoint(StartX, StartY), FIntPoint(SizeX, SizeY), Scale);
}

void UVoxelWorldGeneratorTools::CreateColorTextureFromWorldGeneratorAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	FVoxelColorTexture& OutTexture,
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

	const auto Instance = SetupWorldGenerator<FColor>(__FUNCTION__, WorldGenerator, Seeds, OutputName, VoxelSize, SizeX, SizeY);
	if (!Instance) return;

	FVoxelToolHelpers::StartAsyncLatentAction_WithoutWorld_WithValue(
		WorldContextObject,
		LatentInfo,
		__FUNCTION__,
		bHideLatentWarnings,
		OutTexture,
		[=](FVoxelColorTexture& Texture)
		{
			Texture.Texture = CreateTextureFromWorldGeneratorImpl<FColor>(*Instance, OutputName, FIntPoint(StartX, StartY), FIntPoint(SizeX, SizeY), Scale);
		});
}