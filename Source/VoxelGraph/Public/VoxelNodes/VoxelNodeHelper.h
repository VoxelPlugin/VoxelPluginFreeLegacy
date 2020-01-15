// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNode.h"
#include "VoxelNodeHelper.generated.h"

struct FVoxelHelperPin
{
	FName Name;
	EVoxelPinCategory Category;
	FString DefaultValue;
	FVoxelPinDefaultValueBounds DefaultValueBounds;
	FString ToolTip;

	FVoxelHelperPin() = default;
	FVoxelHelperPin(EVoxelPinCategory Category)
		: Category(Category)
	{
	}
	FVoxelHelperPin(const FName& Name, EVoxelPinCategory Category, const FString& ToolTip, const FString& DefaultValue = "", const FVoxelPinDefaultValueBounds& DefaultValueBounds = {})
		: Name(Name)
		, Category(Category)
		, DefaultValue(DefaultValue)
		, DefaultValueBounds(DefaultValueBounds)
		, ToolTip(ToolTip)
	{
	}
};

struct VOXELGRAPH_API FVoxelPinsHelper
{
	TArray<FVoxelHelperPin> InputPins;
	TArray<FVoxelHelperPin> OutputPins;

	inline FVoxelHelperPin GetInputPin(int32 PinIndex) const
	{
		PinIndex = FMath::Clamp(PinIndex, 0, InputPins.Num() - 1);
		if (InputPins.IsValidIndex(PinIndex))
		{
			return InputPins[PinIndex];
		}
		else
		{
			return {};
		}
	}
	inline FVoxelHelperPin GetOutputPin(int32 PinIndex) const
	{
		PinIndex = FMath::Clamp(PinIndex, 0, OutputPins.Num() - 1);
		if (OutputPins.IsValidIndex(PinIndex))
		{
			return OutputPins[PinIndex];
		}
		else
		{
			return {};
		}
	}
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNodeHelper : public UVoxelNode
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelNode Interface
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;
	virtual FName GetInputPinName(int32 PinIndex) const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;
	virtual FString GetInputPinToolTip(int32 PinIndex) const override;
	virtual FString GetOutputPinToolTip(int32 PinIndex) const override;
	virtual int32 GetMinInputPins() const override;
	virtual int32 GetMaxInputPins() const override;
	virtual int32 GetInputPinsIncrement() const override;
	virtual int32 GetOutputPinsCount() const override;
	virtual FLinearColor GetColor() const override;
	virtual FVoxelPinDefaultValueBounds GetInputPinDefaultValueBounds(int32 PinIndex) const override;
	virtual FString GetInputPinDefaultValue(int32 PinIndex) const override;
	//~ End UVoxelNode Interface

protected:
	// To allow using initializer lists
	void SetInputs(FVoxelHelperPin Pin)
	{
		Pins.InputPins.Add(Pin);
	}
	void SetInputs(
		FVoxelHelperPin Pin0, 
		FVoxelHelperPin Pin1)
	{
		Pins.InputPins.Add(Pin0);
		Pins.InputPins.Add(Pin1);
	}
	void SetInputs(
		FVoxelHelperPin Pin0, 
		FVoxelHelperPin Pin1, 
		FVoxelHelperPin Pin2)
	{
		Pins.InputPins.Add(Pin0);
		Pins.InputPins.Add(Pin1);
		Pins.InputPins.Add(Pin2);
	}
	void SetInputs(
		FVoxelHelperPin Pin0,
		FVoxelHelperPin Pin1,
		FVoxelHelperPin Pin2,
		FVoxelHelperPin Pin3)
	{
		Pins.InputPins.Add(Pin0);
		Pins.InputPins.Add(Pin1);
		Pins.InputPins.Add(Pin2);
		Pins.InputPins.Add(Pin3);
	}
	void SetInputs(
		FVoxelHelperPin Pin0,
		FVoxelHelperPin Pin1,
		FVoxelHelperPin Pin2, 
		FVoxelHelperPin Pin3, 
		FVoxelHelperPin Pin4)
	{
		Pins.InputPins.Add(Pin0);
		Pins.InputPins.Add(Pin1);
		Pins.InputPins.Add(Pin2);
		Pins.InputPins.Add(Pin3);
		Pins.InputPins.Add(Pin4);
	}
	void SetInputs(
		FVoxelHelperPin Pin0,
		FVoxelHelperPin Pin1, 
		FVoxelHelperPin Pin2, 
		FVoxelHelperPin Pin3,
		FVoxelHelperPin Pin4, 
		FVoxelHelperPin Pin5)
	{
		Pins.InputPins.Add(Pin0);
		Pins.InputPins.Add(Pin1);
		Pins.InputPins.Add(Pin2);
		Pins.InputPins.Add(Pin3);
		Pins.InputPins.Add(Pin4);
		Pins.InputPins.Add(Pin5);
	}
	void SetInputs(
		FVoxelHelperPin Pin0,
		FVoxelHelperPin Pin1, 
		FVoxelHelperPin Pin2, 
		FVoxelHelperPin Pin3,
		FVoxelHelperPin Pin4, 
		FVoxelHelperPin Pin5,
		FVoxelHelperPin Pin6)
	{
		Pins.InputPins.Add(Pin0);
		Pins.InputPins.Add(Pin1);
		Pins.InputPins.Add(Pin2);
		Pins.InputPins.Add(Pin3);
		Pins.InputPins.Add(Pin4);
		Pins.InputPins.Add(Pin5);
		Pins.InputPins.Add(Pin6);
	}
	void SetInputs(
		FVoxelHelperPin Pin0,
		FVoxelHelperPin Pin1, 
		FVoxelHelperPin Pin2, 
		FVoxelHelperPin Pin3,
		FVoxelHelperPin Pin4, 
		FVoxelHelperPin Pin5,
		FVoxelHelperPin Pin6,
		FVoxelHelperPin Pin7)
	{
		Pins.InputPins.Add(Pin0);
		Pins.InputPins.Add(Pin1);
		Pins.InputPins.Add(Pin2);
		Pins.InputPins.Add(Pin3);
		Pins.InputPins.Add(Pin4);
		Pins.InputPins.Add(Pin5);
		Pins.InputPins.Add(Pin6);
		Pins.InputPins.Add(Pin7);
	}
	
	void SetOutputs(FVoxelHelperPin Pin)
	{
		Pins.OutputPins.Add(Pin);
	}
	void SetOutputs(
		FVoxelHelperPin Pin0, 
		FVoxelHelperPin Pin1)
	{
		Pins.OutputPins.Add(Pin0);
		Pins.OutputPins.Add(Pin1);
	}
	void SetOutputs(
		FVoxelHelperPin Pin0, 
		FVoxelHelperPin Pin1, 
		FVoxelHelperPin Pin2)
	{
		Pins.OutputPins.Add(Pin0);
		Pins.OutputPins.Add(Pin1);
		Pins.OutputPins.Add(Pin2);
	}
	void SetOutputs(
		FVoxelHelperPin Pin0,
		FVoxelHelperPin Pin1,
		FVoxelHelperPin Pin2, 
		FVoxelHelperPin Pin3)
	{
		Pins.OutputPins.Add(Pin0);
		Pins.OutputPins.Add(Pin1);
		Pins.OutputPins.Add(Pin2);
		Pins.OutputPins.Add(Pin3);
	}
	void SetOutputs(
		FVoxelHelperPin Pin0, 
		FVoxelHelperPin Pin1, 
		FVoxelHelperPin Pin2, 
		FVoxelHelperPin Pin3, 
		FVoxelHelperPin Pin4)
	{
		Pins.OutputPins.Add(Pin0);
		Pins.OutputPins.Add(Pin1);
		Pins.OutputPins.Add(Pin2);
		Pins.OutputPins.Add(Pin3);
		Pins.OutputPins.Add(Pin4);
	}
	void SetOutputs(
		FVoxelHelperPin Pin0,
		FVoxelHelperPin Pin1, 
		FVoxelHelperPin Pin2, 
		FVoxelHelperPin Pin3, 
		FVoxelHelperPin Pin4, 
		FVoxelHelperPin Pin5)
	{
		Pins.OutputPins.Add(Pin0);
		Pins.OutputPins.Add(Pin1);
		Pins.OutputPins.Add(Pin2);
		Pins.OutputPins.Add(Pin3);
		Pins.OutputPins.Add(Pin4);
		Pins.OutputPins.Add(Pin5);
	}
	void SetOutputs(
		FVoxelHelperPin Pin0,
		FVoxelHelperPin Pin1, 
		FVoxelHelperPin Pin2, 
		FVoxelHelperPin Pin3, 
		FVoxelHelperPin Pin4, 
		FVoxelHelperPin Pin5,
		FVoxelHelperPin Pin6)
	{
		Pins.OutputPins.Add(Pin0);
		Pins.OutputPins.Add(Pin1);
		Pins.OutputPins.Add(Pin2);
		Pins.OutputPins.Add(Pin3);
		Pins.OutputPins.Add(Pin4);
		Pins.OutputPins.Add(Pin5);
		Pins.OutputPins.Add(Pin6);
	}
	void SetOutputs(
		FVoxelHelperPin Pin0,
		FVoxelHelperPin Pin1, 
		FVoxelHelperPin Pin2, 
		FVoxelHelperPin Pin3, 
		FVoxelHelperPin Pin4, 
		FVoxelHelperPin Pin5,
		FVoxelHelperPin Pin6,
		FVoxelHelperPin Pin7)
	{
		Pins.OutputPins.Add(Pin0);
		Pins.OutputPins.Add(Pin1);
		Pins.OutputPins.Add(Pin2);
		Pins.OutputPins.Add(Pin3);
		Pins.OutputPins.Add(Pin4);
		Pins.OutputPins.Add(Pin5);
		Pins.OutputPins.Add(Pin6);
		Pins.OutputPins.Add(Pin7);
	}
	
	template<typename... TArgs>
	void SetInputs(FVoxelHelperPin Pin, TArgs... Args)
	{
		Pins.InputPins.Add(Pin);
		SetInputs(Args...);
	}
	template<typename... TArgs>
	void SetOutputs(FVoxelHelperPin Pin, TArgs... Args)
	{
		Pins.OutputPins.Add(Pin);
		SetOutputs(Args...);
	}
	
	void SetInputs(const TArray<FVoxelHelperPin>& InPins)
	{
		Pins.InputPins.Append(InPins);
	}
	void SetOutputs(const TArray<FVoxelHelperPin>& InPins)
	{
		Pins.OutputPins.Append(InPins);
	}

	void SetColor(const FLinearColor& InColor)
	{
		Color = InColor;
	}

	void SetInputsCount(int32 Min, int32 Max)
	{
		bCustomInputsCount = true;
		CustomMin = Min;
		CustomMax = Max;
	}
	void SetInputIncrement(int32 InIncrement)
	{
		ensure(InIncrement > 0);
		Increment = InIncrement;
	}

private:
	FVoxelPinsHelper Pins;
	FLinearColor Color = FLinearColor::Black;
	bool bCustomInputsCount = false;
	int32 CustomMin = 0;
	int32 CustomMax = 0;
	int32 Increment = 1;
};

UCLASS(Abstract, Category = "Setter nodes")
class VOXELGRAPH_API UVoxelSetterNode : public UVoxelNodeHelper
{
	GENERATED_BODY()

public:
	UVoxelSetterNode();


	//~ Begin UVoxelSetterNode Interface
	virtual int32 GetOutputIndex() const { unimplemented(); return -1; };
	//~ End UVoxelSetterNode Interface
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelPureNode : public UVoxelNodeHelper
{
	GENERATED_BODY()

public:
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNodeWithDependencies : public UVoxelNodeHelper
{
	GENERATED_BODY()

public:
	
	//~ Begin UVoxelNodeWithDependencies Interface
	virtual uint8 GetNodeDependencies() const { unimplemented(); return 0; };
	//~ End UVoxelNodeWithDependencies Interface
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNodeWithContext : public UVoxelNodeWithDependencies
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelNodeWithDependencies Interface
	virtual uint8 GetNodeDependencies() const override final;
	//~ End UVoxelNodeWithDependencies Interface
};