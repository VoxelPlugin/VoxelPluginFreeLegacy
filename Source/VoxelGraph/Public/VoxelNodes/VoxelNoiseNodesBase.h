// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Containers/StaticArray.h"
#include "FastNoise.h"
#include "VoxelRange.h"
#include "VoxelNode.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelNoiseNodesEnums.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelNoiseNodesBase.generated.h"

class FVoxelCppConstructor;
struct FVoxelWorldGeneratorInit;

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_NoiseNode : public UVoxelNodeWithContext // For LOD for octaves
{
	GENERATED_BODY()

public:
	// Do not use here, exposed as pin now
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "Noise settings", meta = (DisplayName = "Old Frequency"))
	float Frequency = 0.02;

	UPROPERTY(EditAnywhere, Category = "Noise settings")
	EInterp Interpolation = EInterp::Quintic;

	// To find the output range, NumberOfSamples random samples are computed on start.
	// Increase this if the output range is too irregular, and if you start to see holes in your terrain
	// Increasing it will add a significant (async) cost in graphs
	// Free when compiled to C++
	UPROPERTY(EditAnywhere, Category = "Range analysis settings")
	uint32 NumberOfSamples = 100000;

	UPROPERTY(VisibleAnywhere, Category = "Range analysis settings")
	TArray<FVoxelRange> OutputRanges;

	//~ Begin UVoxelNode Interface
	virtual FName GetInputPinName(int32 PinIndex) const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;
	virtual FString GetInputPinToolTip(int32 PinIndex) const override;
	virtual FString GetOutputPinToolTip(int32 PinIndex) const override;
	virtual int32 GetMinInputPins() const override { return GetDimension() + 2; }
	virtual int32 GetMaxInputPins() const override { return GetDimension() + 2; }
	virtual int32 GetOutputPinsCount() const override { return IsDerivative() ? GetDimension() + 1 : 1; }
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override { return PinIndex == GetDimension() + 1 ? EVoxelPinCategory::Seed : EVoxelPinCategory::Float; }
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override { return EVoxelPinCategory::Float; }
	virtual FString GetInputPinDefaultValue(int32 PinIndex) const override;
	//~ End UVoxelNode Interface

	//~ Begin UVoxelNode_NoiseNode Interface
	virtual uint32 GetDimension() const { unimplemented(); return 0; }
	virtual bool IsDerivative() const { return false; }
	virtual bool IsFractal() const { return false; }
	virtual bool NeedRangeAnalysis() const { return true; }
	//~ End UVoxelNode_NoiseNode Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif
	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	//~ End UObject Interface
};


UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_NoiseNodeFractal : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Fractal Noise settings", meta = (UIMin = 1, ClampMin = 1))
	int32 FractalOctaves = 3;

	// A multiplier that determines how quickly the frequency increases for each successive octave
	// The frequency of each successive octave is equal to the product of the previous octave's frequency and the lacunarity value.
	UPROPERTY(EditAnywhere, Category = "Fractal Noise settings")
	float FractalLacunarity = 2;

	// A multiplier that determines how quickly the amplitudes diminish for each successive octave
	// The amplitude of each successive octave is equal to the product of the previous octave's amplitude and the gain value. Increasing the gain produces "rougher" Perlin noise.
	UPROPERTY(EditAnywhere, Category = "Fractal Noise settings")
	float FractalGain = 0.5;

	UPROPERTY(EditAnywhere, Category = "Fractal Noise settings")
	EFractalType FractalType = EFractalType::FBM;

	// To use lower quality noise for far LODs
	UPROPERTY(EditAnywhere, Category = "LOD settings", meta = (DisplayName = "LOD to Octaves map"))
	TMap<FString, uint8> LODToOctavesMap;

	//~ Begin UVoxelNode Interface
	virtual FLinearColor GetNodeBodyColor() const override;
	virtual FLinearColor GetColor() const override;
	//~ End UVoxelNode Interface

	//~ Begin UVoxelNode_NoiseNode Interface
	virtual bool IsFractal() const override final { return true; }
	//~ End UVoxelNode_NoiseNode Interface
	
protected:
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	//~ End UObject Interface
};


//////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_NoiseNodeWithDerivative : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Noise settings")
	bool bComputeDerivative = false;

	virtual bool IsDerivative() const override { return bComputeDerivative; }
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_NoiseNodeWithDerivativeFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Noise settings")
	bool bComputeDerivative = false;

	virtual bool IsDerivative() const override { return bComputeDerivative; }
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_IQNoiseBase : public UVoxelNode_NoiseNodeWithDerivativeFractal
{
	GENERATED_BODY()

public:
	UVoxelNode_IQNoiseBase();

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif
	//~ End UObject Interface
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_2DIQNoiseBase : public UVoxelNode_IQNoiseBase
{
	GENERATED_BODY()

public:
	// Rotation (in degrees) applied to the position between each octave
	UPROPERTY(EditAnywhere, Category = "IQ Noise settings")
	float Rotation = 40;
};


UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_3DIQNoiseBase : public UVoxelNode_IQNoiseBase
{
	GENERATED_BODY()

public:
	// Rotation (in degrees) applied to the position between each octave
	UPROPERTY(EditAnywhere, Category = "IQ Noise settings")
	FRotator Rotation = { 40, 45, 50 };
};

