// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class UVoxelExposedNode;

class VOXELGRAPH_API FVoxelVariable
{
public:
	static FString SanitizeName(const FString& Name);

public:
	const FString Type;
	const FString Name;

	FVoxelVariable(const FString& Type, const FString& Name);

	inline FString GetDeclaration() const { return Type + TEXT(" ") + Name; }
	inline FString GetRefDeclaration() const { return Type + TEXT("& ") + Name; }
	inline FString GetConstDeclaration() const { return "const " + GetDeclaration(); }
	inline FString GetConstRefDeclaration() const { return "const " + GetRefDeclaration(); }
};

class VOXELGRAPH_API FVoxelExposedVariable : public FVoxelVariable
{
public:
	const UVoxelExposedNode* const Node;
	const FString DefaultValue;
	const FString ExposedType;
	const FString Category;
	const FString Tooltip;
	const int32 Priority;
	const TMap<FName, FString> CustomMetaData;

	/**
	 * Exposed variable
	 * @param	Type			The type of the variable
	 * @param	Node			The node exposing this variable
	 * @param	DefaultValue	The default value of the variable, can be empty
	 * @param	ExposedType		If the exposed type isn't the same as the instance type. 
								You'll be able to do custom stuff through GetLocalVariableFromExposedOne
	 */
	FVoxelExposedVariable(
		const UVoxelExposedNode& Node,
		const FString& Type,
		const FString& ExposedType,
		const FString& DefaultValue);

	virtual ~FVoxelExposedVariable() = default;

	// For example: Name.GetWorldGenerator()
	virtual FString GetLocalVariableFromExposedOne() const
	{
		return Name;
	}
	// Some exposed variables always have some metadata
	virtual TMap<FName, FString> GetExposedVariableDefaultMetadata() const
	{
		return {};
	}

	bool IsSameAs(const FVoxelExposedVariable& Other, bool bCheckNode) const;
	FString GetMetadataString() const;
};