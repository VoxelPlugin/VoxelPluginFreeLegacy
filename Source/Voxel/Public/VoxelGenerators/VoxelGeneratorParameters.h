// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelGeneratorParameters.generated.h"

UENUM()
enum class EVoxelGeneratorParameterContainerType : uint8
{
	None,
	Array,
	Set,
	Map
};

UENUM()
enum class EVoxelGeneratorParameterPropertyType : uint8
{
	Float,
	Int,
	Bool,
	Object,
	Struct,
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelGeneratorParameterTerminalType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	EVoxelGeneratorParameterPropertyType PropertyType = EVoxelGeneratorParameterPropertyType::Float;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	FName PropertyClass;

	FString ToString_Terminal() const;
	bool CanBeAssignedFrom_Terminal(const FVoxelGeneratorParameterTerminalType& Other) const;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelGeneratorParameterType : public FVoxelGeneratorParameterTerminalType
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	EVoxelGeneratorParameterContainerType ContainerType = EVoxelGeneratorParameterContainerType::None;

	// For maps
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	FVoxelGeneratorParameterTerminalType ValueType;

	FVoxelGeneratorParameterType() = default;
	explicit FVoxelGeneratorParameterType(FProperty& Property);

public:
	bool operator==(const FVoxelGeneratorParameterType& Other) const
	{
		return
			PropertyType == Other.PropertyType &&
			PropertyClass == Other.PropertyClass;
	}
	bool operator!=(const FVoxelGeneratorParameterType& Other) const
	{
		return !(*this == Other);
	}

	FString ToString() const;
	bool CanBeAssignedFrom(const FVoxelGeneratorParameterType& Other) const;
};

USTRUCT(BlueprintType)
struct FVoxelGeneratorParameter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	FVoxelGeneratorParameterType Type;

	// Not consistent with vs without editor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	FString Category;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	FString ToolTip;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	int32 Priority = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	TMap<FName, FString> MetaData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter")
	FString DefaultValue;

	FVoxelGeneratorParameter() = default;
	FVoxelGeneratorParameter(
		const FName& Id,
		const FVoxelGeneratorParameterType& Type,
		const FString& Name,
		const FString& Category,
		const FString& ToolTip,
		int32 Priority,
		const TMap<FName, FString>& MetaData,
		const FString& DefaultValue)
		: Id(Id)
		, Type(Type)
		, Name(Name)
		, Category(Category)
		, ToolTip(ToolTip)
		, Priority(Priority)
		, MetaData(MetaData)
		, DefaultValue(DefaultValue)
	{
	}

	bool operator==(const FVoxelGeneratorParameter& Other) const
	{
		return
			Id == Other.Id &&
			Type == Other.Type &&
			Name == Other.Name &&
			Category == Other.Category &&
			ToolTip == Other.ToolTip &&
			Priority == Other.Priority &&
			MetaData.OrderIndependentCompareEqual(Other.MetaData) &&
			DefaultValue == Other.DefaultValue;
	}
	bool operator!=(const FVoxelGeneratorParameter& Other) const
	{
		return !(*this == Other);
	}
};