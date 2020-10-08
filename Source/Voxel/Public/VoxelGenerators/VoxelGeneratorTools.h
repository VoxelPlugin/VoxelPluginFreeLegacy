// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelTexture.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGeneratorTools.generated.h"

class AVoxelWorld;
class UTexture2D;
class UVoxelGeneratorInstanceWrapper;
class UVoxelTransformableGeneratorInstanceWrapper;

UCLASS()
class VOXEL_API UVoxelGeneratorTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * Creates a new generator instance. Consider using VoxelWorld->GetGeneratorCache()->MakeGeneratorInstance instead
	 * @param	GeneratorPicker		The picker
	 * @param	GeneratorInit		The generator init. Use VoxelWorld->GetGeneratorInit to get it
	 * @return	The generator instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Generators", meta=(Keywords="construct build"))
	static UVoxelGeneratorInstanceWrapper* MakeGeneratorInstance(FVoxelGeneratorPicker GeneratorPicker, FVoxelGeneratorInit GeneratorInit);
	
	/**
	 * Creates a new transformable generator instance. Consider using VoxelWorld->GetGeneratorCache()->MakeTransformableGeneratorInstance instead
	 * @param	GeneratorPicker		The picker
	 * @param	GeneratorInit		The generator init. Use VoxelWorld->GetGeneratorInit to get it
	 * @return	The generator instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Generators", meta=(Keywords="construct build"))
	static UVoxelTransformableGeneratorInstanceWrapper* MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker GeneratorPicker, FVoxelGeneratorInit GeneratorInit);

public:
	/**
	 * @see MakeGeneratorPickerFromClass, MakeTransformableGeneratorPickerFromObject
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Generators", meta=(Keywords="construct build", NativeMakeFunc))
	static FVoxelGeneratorPicker MakeGeneratorPickerFromObject(UVoxelGenerator* Generator) { return Generator; }
	/**
	 * @see MakeTransformableGeneratorPickerFromClass, MakeGeneratorPickerFromObject
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Generators", meta=(Keywords="construct build", NativeMakeFunc))
	static FVoxelTransformableGeneratorPicker MakeTransformableGeneratorPickerFromObject(UVoxelTransformableGenerator* Generator) { return Generator; }
	
	/**
	 * @see MakeGeneratorPickerFromObject
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Generators", meta=(Keywords="construct build", NativeMakeFunc))
	static FVoxelGeneratorPicker MakeGeneratorPickerFromClass(TSubclassOf<UVoxelGenerator> GeneratorClass) { return GeneratorClass; }
	/**
	 * @see MakeTransformableGeneratorPickerFromObject, MakeGeneratorPickerFromClass
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Generators", meta=(Keywords="construct build", NativeMakeFunc))
	static FVoxelTransformableGeneratorPicker MakeTransformableGeneratorPickerFromClass(TSubclassOf<UVoxelTransformableGenerator> GeneratorClass) { return GeneratorClass; }

	UFUNCTION(BlueprintPure, Category = "Voxel|Generators", DisplayName = "Is Valid")
	static bool IsValid_GeneratorPicker(FVoxelGeneratorPicker GeneratorPicker) { return GeneratorPicker.IsValid(); }
	UFUNCTION(BlueprintPure, Category = "Voxel|Generators", DisplayName = "Is Valid")
	static bool IsValid_TransformableGeneratorPicker(FVoxelTransformableGeneratorPicker GeneratorPicker) { return GeneratorPicker.IsValid(); }

public:
	static bool SetGeneratorParameterImpl(
		TVoxelGeneratorPicker<UVoxelGenerator>& Picker,
		FName Name,
		FProperty& Property,
		void* Data,
		const FString& FunctionName);
	static bool CheckIsValidParameterName(
		TVoxelGeneratorPicker<UVoxelGenerator> GeneratorPicker,
		FName Name,
		FProperty& Property,
		const FString& FunctionName);

	/**
	 * Set a voxel generator parameter
	 * @param	Picker		The generator picker, by ref
	 * @param	UniqueName	The name of the parameter. Note that this is not the display name, but the parameter unique name.
	 *						You can get that unique name by checking the tooltip of the parameter in the picker details
	 * @param	Value		The value
	 * @return	Success
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Voxel|Generators", meta = (CustomStructureParam = "Value"))
	static bool SetGeneratorParameter(const FVoxelGeneratorPicker& Picker, FName UniqueName, int32 Value);

	/**
	 * Set a voxel generator parameter
	 * @param	Picker		The generator picker, by ref
	 * @param	UniqueName	The name of the parameter. Note that this is not the display name, but the parameter unique name.
	 *						You can get that unique name by checking the tooltip of the parameter in the picker details
	 * @param	Value		The value
	 * @return	Success
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Voxel|Generators", meta = (CustomStructureParam = "Value"))
	static bool SetTransformableGeneratorParameter(const FVoxelTransformableGeneratorPicker& Picker, FName UniqueName, int32 Value);

private:
	DECLARE_FUNCTION(execSetGeneratorParameter)
	{
		execSetGeneratorParameterImpl(Context, Stack, RESULT_PARAM);
	}
	
	DECLARE_FUNCTION(execSetTransformableGeneratorParameter)
	{
		execSetGeneratorParameterImpl(Context, Stack, RESULT_PARAM);
	}
	
	DECLARE_FUNCTION(execSetGeneratorParameterImpl)
	{
        P_GET_STRUCT_REF(TVoxelGeneratorPicker<UVoxelGenerator>, Picker);
        P_GET_STRUCT(FName, Name);
        
        Stack.StepCompiledIn<FStructProperty>(nullptr);

		P_FINISH;
		bool bSuccess = false;
		
		if (Stack.MostRecentProperty)
		{
			bSuccess = SetGeneratorParameterImpl(Picker, Name, *Stack.MostRecentProperty, Stack.MostRecentPropertyAddress, "SetGeneratorParameter");
		}
		else
		{
			const FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AccessViolation, VOXEL_LOCTEXT("Failed to resolve the Value parameter for SetGeneratorParameter."));
			FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		}
		*static_cast<bool*>(RESULT_PARAM) = bSuccess;
	}

public:
	// Scale is applied to (Start + Position)
	template<typename T>
	static TVoxelTexture<T> CreateTextureFromGeneratorImpl(
		const FVoxelGeneratorInstance& Generator,
		FName OutputName,
		const FIntPoint& Start,
		const FIntPoint& Size,
		float Scale);
	
	/**
	 * Creates a float texture by reading a float output from a generator
	 * 
	 * @param	OutTexture			The result
	 * @param	Generator			The generator to use
	 * @param	OutputName			The output name to query. Must be a float output.
	 * @param	SizeX				The Size of the resulting texture on the X axis
	 * @param	SizeY				The Size of the resulting texture on the Y axis
	 * @param	Scale				Scale that can be used to scale the inputs: the generator will be queried as (Start + Position) * Scale
	 * @param	StartX				Where the texture starts
	 * @param	StartY				Where the texture starts
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Generators", meta = (AdvancedDisplay = "StartX, StartY, VoxelSize"))
	static void CreateFloatTextureFromGenerator(
		FVoxelFloatTexture& OutTexture,
		UVoxelGeneratorInstanceWrapper* Generator,
		FName OutputName = "Value",
		int32 SizeX = 512,
		int32 SizeY = 512,
		float Scale = 1,
		int32 StartX = 0,
		int32 StartY = 0);
	
	/**
	 * Creates a float texture by reading a float output from a generator, asynchronously
	 * 
	 * @param	OutTexture			The result
	 * @param	Generator			The generator to use
	 * @param	OutputName			The output name to query. Must be a float output.
	 * @param	SizeX				The Size of the resulting texture on the X axis
	 * @param	SizeY				The Size of the resulting texture on the Y axis
	 * @param	Scale				Scale that can be used to scale the inputs: the generator will be queried as (Start + Position) * Scale
	 * @param	StartX				Where the texture starts
	 * @param	StartY				Where the texture starts
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Generators", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "StartX, StartY, VoxelSize, bHideLatentWarnings"))
	static void CreateFloatTextureFromGeneratorAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelFloatTexture& OutTexture,
		UVoxelGeneratorInstanceWrapper* Generator,
		FName OutputName = "Value",
		int32 SizeX = 512,
		int32 SizeY = 512,
		float Scale = 1,
		int32 StartX = 0,
		int32 StartY = 0,
		bool bHideLatentWarnings = false);

public:
	/**
	 * Creates a color texture by reading a color output from a generator
	 * 
	 * @param	OutTexture			The result
	 * @param	Generator			The generator to use
	 * @param	OutputName			The output name to query. Must be a color output.
	 * @param	SizeX				The Size of the resulting texture on the X axis
	 * @param	SizeY				The Size of the resulting texture on the Y axis
	 * @param	Scale				Scale that can be used to scale the inputs: the generator will be queried as (Start + Position) * Scale
	 * @param	StartX				Where the texture starts
	 * @param	StartY				Where the texture starts
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Generators", meta = (AdvancedDisplay = "StartX, StartY, VoxelSize"))
	static void CreateColorTextureFromGenerator(
		FVoxelColorTexture& OutTexture,
		UVoxelGeneratorInstanceWrapper* Generator,
		FName OutputName = "MyColor",
		int32 SizeX = 512,
		int32 SizeY = 512,
		float Scale = 1,
		int32 StartX = 0,
		int32 StartY = 0);
	
	/**
	 * Creates a color texture by reading a color output from a generator, asynchronously
	 * 
	 * @param	OutTexture			The result
	 * @param	Generator			The generator to use
	 * @param	OutputName			The output name to query. Must be a color output.
	 * @param	SizeX				The Size of the resulting texture on the X axis
	 * @param	SizeY				The Size of the resulting texture on the Y axis
	 * @param	Scale				Scale that can be used to scale the inputs: the generator will be queried as (Start + Position) * Scale
	 * @param	StartX				Where the texture starts
	 * @param	StartY				Where the texture starts
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Generators", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "StartX, StartY, VoxelSize, bHideLatentWarnings"))
	static void CreateColorTextureFromGeneratorAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelColorTexture& OutTexture,
		UVoxelGeneratorInstanceWrapper* Generator,
		FName OutputName = "MyColor",
		int32 SizeX = 512,
		int32 SizeY = 512,
		float Scale = 1,
		int32 StartX = 0,
		int32 StartY = 0,
		bool bHideLatentWarnings = false);
};