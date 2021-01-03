// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"

struct FVoxelGeneratorInit;

class VOXELGRAPH_API FVoxelGeneratorVariableHelper
{
public:
	FVoxelGeneratorVariableHelper(const FVoxelWeakGeneratorPicker& Picker)
		: Picker(Picker)
	{
	}
	
	void Init(const FVoxelGeneratorInit& InitStruct) const;

	const FVoxelGeneratorInstance* operator->() const
	{
		return Generator.Get();
	}
	const FVoxelGeneratorInstance& operator*() const
	{
		return *Generator;
	}

private:
	const FVoxelWeakGeneratorPicker Picker;
	mutable TVoxelSharedPtr<FVoxelGeneratorInstance> Generator;
};