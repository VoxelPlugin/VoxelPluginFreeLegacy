// Copyright 2020 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorTools.h"
#include "VoxelGenerators/VoxelGenerator.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"
#include "VoxelGenerators/VoxelGeneratorParameters.h"
#include "VoxelGenerators/VoxelGeneratorInstanceWrapper.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelMessages.h"
#include "VoxelUtilities/VoxelGeneratorUtilities.h"

#include "UObject/PropertyPortFlags.h"

UVoxelGeneratorInstanceWrapper* UVoxelGeneratorTools::MakeGeneratorInstance(FVoxelGeneratorPicker GeneratorPicker, FVoxelGeneratorInit GeneratorInit)
{
	if (!GeneratorPicker.IsValid())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid generator"));
		return nullptr;
	}

	auto* Instance = NewObject<UVoxelGeneratorInstanceWrapper>();
	Instance->Instance = GeneratorPicker.GetInstance(true);
	Instance->Instance->Init(GeneratorInit);
	return Instance;
}

UVoxelTransformableGeneratorInstanceWrapper* UVoxelGeneratorTools::MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker GeneratorPicker, FVoxelGeneratorInit GeneratorInit)
{
	if (!GeneratorPicker.IsValid())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid generator"));
		return nullptr;
	}

	auto* Instance = NewObject<UVoxelTransformableGeneratorInstanceWrapper>();
	Instance->Instance = GeneratorPicker.GetInstance(true);
	Instance->Instance->Init(GeneratorInit);
	return Instance;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelGeneratorTools::SetGeneratorParameterImpl(TVoxelGeneratorPicker<UVoxelGenerator>& Picker, FName Name, FProperty& Property, void* Data, const FString& FunctionName)
{
	if (!CheckIsValidParameterName(Picker, Name, Property, FunctionName))
	{
		return false;
	}

	FString Result;
	Property.ExportTextItem(Result, Data, nullptr, nullptr, PPF_None);
	Picker.Parameters.Add(Name, Result);
	
	return true;
}

bool UVoxelGeneratorTools::CheckIsValidParameterName(
	TVoxelGeneratorPicker<UVoxelGenerator> GeneratorPicker,
	FName Name,
	FProperty& Property,
	const FString& FunctionName)
{
	if (!GeneratorPicker.IsValid())
	{
		FVoxelMessages::Error(FUNCTION_ERROR_IMPL(FunctionName, "Invalid generator"));
		return false;
	}

	TArray<FVoxelGeneratorParameter> Parameters;
	GeneratorPicker.GetGenerator()->GetParameters(Parameters);

	const FVoxelGeneratorParameterType Type(Property);
	for (auto& It : Parameters)
	{
		if (It.Id == Name)
		{
			if (It.Type.CanBeAssignedFrom(Type))
			{
				return true;
			}
			else
			{
				FVoxelMessages::Error(FUNCTION_ERROR_IMPL(FunctionName, FString::Printf(
					TEXT("Incompatible parameter type: cannot cast from %s to %s"),
					*Type.ToString(),
					*It.Type.ToString())));
				return false;
			}
		}
	}

	FVoxelMessages::Error(FUNCTION_ERROR_IMPL(FunctionName, FString::Printf(TEXT(
		"Parameter name not found: %s. Make sure you use the parameter unique name and not its display name. "
		"You can find the unique name in the parameter tooltip in the Generator details."), *Name.ToString())));
	return false;
}

bool UVoxelGeneratorTools::SetGeneratorParameter(const FVoxelGeneratorPicker& Picker, FName UniqueName, int32 Value)
{
	checkf(false, TEXT("SetGeneratorParameter can only be called from blueprints"));
	return false;
}

bool UVoxelGeneratorTools::SetTransformableGeneratorParameter(const FVoxelTransformableGeneratorPicker& Picker, FName UniqueName, int32 Value)
{
	checkf(false, TEXT("SetTransformableGeneratorParameter can only be called from blueprints"));
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
TVoxelTexture<T> UVoxelGeneratorTools::CreateTextureFromGeneratorImpl(
	const FVoxelGeneratorInstance& Generator, 
	FName OutputName, 
	const FIntPoint& Start, 
	const FIntPoint& Size,
	float Scale)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Size.X * Size.Y);
	check(Size.X > 0 && Size.Y > 0);

	using TGeneratorType = typename TChooseClass<TIsSame<T, float>::Value, v_flt, T>::Result;
	
	const auto FunctionPtr = Generator.GetOutputsPtrMap<TGeneratorType>().FindRef(OutputName);
	if (!ensure(FunctionPtr)) return {};
	
	const auto Texture = MakeVoxelShared<typename TVoxelTexture<T>::FTextureData>();
	Texture->SetSize(Size.X, Size.Y);
	for (int32 X = 0; X < Size.X; X++)
	{
		for (int32 Y = 0; Y < Size.Y; Y++)
		{
			const auto Value = (Generator.*FunctionPtr)(Scale * (Start.X + X), Scale * (Start.Y + Y), 0, 0, FVoxelItemStack::Empty);
			Texture->SetValue(X, Y, T(Value));
		}
	}
	return TVoxelTexture<T>(Texture);
}

template VOXEL_API TVoxelTexture<float> UVoxelGeneratorTools::CreateTextureFromGeneratorImpl<float>(
	const FVoxelGeneratorInstance& Generator,
	FName OutputName,
	const FIntPoint& Start,
	const FIntPoint& Size,
	float Scale);

template VOXEL_API TVoxelTexture<FColor> UVoxelGeneratorTools::CreateTextureFromGeneratorImpl<FColor>(
	const FVoxelGeneratorInstance& Generator,
	FName OutputName,
	const FIntPoint& Start,
	const FIntPoint& Size,
	float Scale);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
TVoxelSharedPtr<FVoxelGeneratorInstance> SetupGenerator(
	const FString& FunctionName,
	UVoxelGeneratorInstanceWrapper* Generator,
	FName OutputName,
	int32 SizeX,
	int32 SizeY)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!Generator || !Generator->IsValid())
	{
		FVoxelMessages::Error( FUNCTION_ERROR_IMPL(FunctionName, "Invalid Generator!"));
		return {};
	}
	if (SizeX <= 0 || SizeY <= 0)
	{
		FVoxelMessages::Error( FUNCTION_ERROR_IMPL(FunctionName, "Invalid Size!"));
		return {};
	}

	using TGeneratorType = typename TChooseClass<TIsSame<T, float>::Value, v_flt, T>::Result;

	if (!Generator->Instance->GetOutputsPtrMap<TGeneratorType>().Contains(OutputName))
	{
		FVoxelMessages::Error(FUNCTION_ERROR_IMPL(FunctionName, FVoxelUtilities::GetMissingGeneratorOutputErrorString<TGeneratorType>(OutputName, *Generator->Instance)));
		return {};
	}

	return Generator->Instance;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGeneratorTools::CreateFloatTextureFromGenerator(
	FVoxelFloatTexture& OutTexture, 
	UVoxelGeneratorInstanceWrapper* Generator, 
	FName OutputName,
	int32 SizeX, 
	int32 SizeY,
	float Scale,
	int32 StartX,
	int32 StartY)
{
	VOXEL_FUNCTION_COUNTER();

	const auto Instance = SetupGenerator<float>(__FUNCTION__, Generator, OutputName, SizeX, SizeY);
	if (!Instance) return;

	OutTexture.Texture = CreateTextureFromGeneratorImpl<float>(*Instance, OutputName, FIntPoint(StartX, StartY), FIntPoint(SizeX, SizeY), Scale);
}

void UVoxelGeneratorTools::CreateFloatTextureFromGeneratorAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	FVoxelFloatTexture& OutTexture,
	UVoxelGeneratorInstanceWrapper* Generator,
	FName OutputName,
	int32 SizeX,
	int32 SizeY,
	float Scale,
	int32 StartX,
	int32 StartY,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();

	const auto Instance = SetupGenerator<float>(__FUNCTION__, Generator, OutputName, SizeX, SizeY);
	if (!Instance) return;

	FVoxelToolHelpers::StartAsyncLatentAction_WithoutWorld_WithValue(
		WorldContextObject,
		LatentInfo,
		__FUNCTION__,
		bHideLatentWarnings,
		OutTexture,
		[=](FVoxelFloatTexture& Texture)
		{
			Texture.Texture = CreateTextureFromGeneratorImpl<float>(*Instance, OutputName, FIntPoint(StartX, StartY), FIntPoint(SizeX, SizeY), Scale);
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGeneratorTools::CreateColorTextureFromGenerator(
	FVoxelColorTexture& OutTexture, 
	UVoxelGeneratorInstanceWrapper* Generator, 
	FName OutputName,
	int32 SizeX, 
	int32 SizeY,
	float Scale,
	int32 StartX,
	int32 StartY)
{
	VOXEL_FUNCTION_COUNTER();

	const auto Instance = SetupGenerator<FColor>(__FUNCTION__, Generator, OutputName, SizeX, SizeY);
	if (!Instance) return;

	OutTexture.Texture = CreateTextureFromGeneratorImpl<FColor>(*Instance, OutputName, FIntPoint(StartX, StartY), FIntPoint(SizeX, SizeY), Scale);
}

void UVoxelGeneratorTools::CreateColorTextureFromGeneratorAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	FVoxelColorTexture& OutTexture,
	UVoxelGeneratorInstanceWrapper* Generator,
	FName OutputName,
	int32 SizeX,
	int32 SizeY,
	float Scale,
	int32 StartX,
	int32 StartY,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();

	const auto Instance = SetupGenerator<FColor>(__FUNCTION__, Generator, OutputName, SizeX, SizeY);
	if (!Instance) return;

	FVoxelToolHelpers::StartAsyncLatentAction_WithoutWorld_WithValue(
		WorldContextObject,
		LatentInfo,
		__FUNCTION__,
		bHideLatentWarnings,
		OutTexture,
		[=](FVoxelColorTexture& Texture)
		{
			Texture.Texture = CreateTextureFromGeneratorImpl<FColor>(*Instance, OutputName, FIntPoint(StartX, StartY), FIntPoint(SizeX, SizeY), Scale);
		});
}