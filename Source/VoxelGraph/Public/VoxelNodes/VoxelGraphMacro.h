// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphGenerator.h"
#include "VoxelNode.h"
#include "VoxelGraphMacro.generated.h"

USTRUCT()
struct FVoxelGraphMacroPin
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FString Name;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelPinCategory Category;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FString ToolTip;

	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (EditCondition = bCustomDefaultValue))
	FString DefaultValue;

	UPROPERTY(EditAnywhere, Category = "Voxel",  meta = (InlineEditConditionToggle))
	bool bCustomDefaultValue = false;

	inline FString GetDefaultValueEqual() const
	{
		return (DefaultValue.IsEmpty() || !bCustomDefaultValue) ? "" : (" = " + DefaultValue);
	}

	inline FString GetDefaultValue() const
	{
		return bCustomDefaultValue ? DefaultValue : "";
	}
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelGraphMacroInputOutputNode : public UVoxelNode
{
	GENERATED_BODY()	
public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	TArray<FVoxelGraphMacroPin> Pins;

	UPROPERTY()
	class UVoxelGraphMacro* Macro;

	virtual FLinearColor GetColor() const override;
	virtual bool CanUserDeleteNode() const override;
	virtual bool CanDuplicateNode() const override;

	virtual int32 GetMaxInputPins() const override;
	virtual int32 GetMinInputPins() const override;
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;

	virtual int32 GetOutputPinsCount() const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;


#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void PostLoad() override;
#endif // WITH_EDITOR
};

UCLASS(DisplayName = "Input", NotPlaceable)
class VOXELGRAPH_API UVoxelGraphMacroInputNode : public UVoxelGraphMacroInputOutputNode
{
	GENERATED_BODY()

	FName GetInputPinName(int32 PinIndex) const override { return *(Pins[PinIndex].Name + Pins[PinIndex].GetDefaultValueEqual()); }
	FName GetOutputPinName(int32 PinIndex) const override { return *Pins[PinIndex].Name; }
};

UCLASS(DisplayName = "Output", NotPlaceable)
class VOXELGRAPH_API UVoxelGraphMacroOutputNode : public UVoxelGraphMacroInputOutputNode
{
	GENERATED_BODY()

	FName GetInputPinName(int32 PinIndex) const override { return *Pins[PinIndex].Name; }
	FName GetOutputPinName(int32 PinIndex) const override { return *Pins[PinIndex].Name; }
};

/**
 * A graph macro
 */
UCLASS(BlueprintType, HideCategories = (Object), HideDropdown)
class VOXELGRAPH_API UVoxelGraphMacro : public UVoxelGraphGenerator
{
	GENERATED_BODY()
public:
	// Shift+Enter for new line
	UPROPERTY(EditAnywhere, Category = "Macro Config", meta = (MultiLine = true, DisplayName = "Tooltip (Shift+Enter for new line)"))
	FString Tooltip;

	UPROPERTY(EditAnywhere, Category = "Macro Config")
	FString Keywords;

	// If empty the Macro Nodes category is used
	UPROPERTY(EditAnywhere, Category = "Macro Config")
	FString CustomCategory;

	// If empty the asset name is used
	UPROPERTY(EditAnywhere, Category = "Macro Config")
	FString CustomName;
	
	UPROPERTY(EditAnywhere, Category = "Macro Config")
	bool bShowInContextMenu = true;

	UPROPERTY()
	UVoxelGraphMacroInputNode* InputNode;

	UPROPERTY()
	UVoxelGraphMacroOutputNode* OutputNode;

	FText GetMacroName() const;
	FText GetMacroCategory() const;
};

UCLASS(NotPlaceable)
class VOXELGRAPH_API UVoxelGraphMacroNode : public UVoxelNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	UVoxelGraphMacro* Macro;


	virtual FText GetTitle() const override;
	virtual FText GetTooltip() const override;

	virtual int32 GetMaxInputPins() const override;
	virtual int32 GetMinInputPins() const override;
	virtual int32 GetOutputPinsCount() const override;

	virtual FName GetInputPinName(int32 PinIndex) const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;
	
	virtual FString GetInputPinToolTip(int32 PinIndex) const override;
	virtual FString GetOutputPinToolTip(int32 PinIndex) const override;
	
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;

	virtual FString GetInputPinDefaultValue(int32 PinIndex) const override;
	
	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};