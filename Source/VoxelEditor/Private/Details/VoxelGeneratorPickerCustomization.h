// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "VoxelGenerators/VoxelGeneratorParameters.h"
#include "VoxelGeneratorPickerCustomization.generated.h"

class SWidget;
class IPropertyHandle;
struct FEdGraphPinType;
struct FEdGraphTerminalType;
struct FVoxelGeneratorPicker;
enum class EVoxelGeneratorPickerType : uint8;

UCLASS()
class UVoxelGeneratorPickerEditorData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UObject> GeneratorObject = nullptr;
	
	UPROPERTY()
	TArray<FVoxelGeneratorParameter> Parameters;

	UPROPERTY()
	TObjectPtr<UBlueprint> Blueprint = nullptr;

	UPROPERTY()
	TObjectPtr<UObject> BlueprintInstance = nullptr;
};

UCLASS()
class UVoxelGeneratorPickerBlueprintPool : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<TObjectPtr<UBlueprint>> Blueprints;
};

class FVoxelGeneratorPickerCustomization : public IPropertyTypeCustomization
{
public:
	FVoxelGeneratorPickerCustomization() = default;
	
	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface

private:
	EVoxelGeneratorPickerType PickerType{};
	TArray<TSharedPtr<EVoxelGeneratorPickerType>> ComboBoxArray;

private:
	static FVoxelGeneratorPicker& GetPicker(IPropertyHandle& Handle);
	static FEdGraphPinType GetParameterPinType(const FVoxelGeneratorParameterType& ParameterType);
	static FEdGraphTerminalType GetParameterTerminalPinType(const FVoxelGeneratorParameterTerminalType& ParameterType);
};