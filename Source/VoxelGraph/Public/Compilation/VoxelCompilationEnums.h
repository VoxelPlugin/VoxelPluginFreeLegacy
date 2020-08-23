// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

enum class EVoxelPinDirection : uint8
{
	Input,
	Output
};

enum class EVoxelCompilationNodeType : uint8
{
	Default,
	Macro,
	MacroInputOutput,
	FlowMerge,
	Passthrough,
	LocalVariableDeclaration,
	LocalVariableUsage,
	FunctionSeparator,
	FunctionCall,
	FunctionInit,
	If,
	Setter,
	RangeAnalysisConstant,
	GetRangeAnalysis,
	BiomeMerge,
	CompileTimeConstant,
	Switch,
	SmartMinMax
};

enum class EVoxelPinIter : uint8
{
	Input,
	Output,
	All
};