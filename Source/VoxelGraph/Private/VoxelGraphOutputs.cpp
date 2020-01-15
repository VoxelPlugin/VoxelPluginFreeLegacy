// Copyright 2020 Phyronnaz

#include "VoxelGraphOutputs.h"
#include "VoxelGraphConstants.h"


const TArray<FVoxelGraphOutput> FVoxelGraphOutput::DefaultOutputs(
	{
	{"RangeAnalysis", EVoxelDataPinCategory::Float   , FGuid(0x956428b9, 0xff921b6c, 0xabb6b187, 0x7d47075b), FVoxelGraphOutputsIndices::RangeAnalysisIndex},
	{"Value"        , EVoxelDataPinCategory::Float   , FGuid(0x3fd2558e, 0x99b0d175, 0x14562d0a, 0xc140e1a6), FVoxelGraphOutputsIndices::ValueIndex},
	{"Material"     , EVoxelDataPinCategory::Material, FGuid(0xe639695e, 0xecc58da5, 0x0f7be8f7, 0x29d173f3), FVoxelGraphOutputsIndices::MaterialIndex},
	{"UpVectorX"    , EVoxelDataPinCategory::Float   , FGuid(0xc9b67a6b, 0x592eb713, 0x18cc0ed0, 0x128e47cb), FVoxelGraphOutputsIndices::UpVectorXIndex},
	{"UpVectorY"    , EVoxelDataPinCategory::Float   , FGuid(0x0bde596a, 0xc3e9fc9c, 0x72b27fc0, 0x1bc6112d), FVoxelGraphOutputsIndices::UpVectorYIndex},
	{"UpVectorZ"    , EVoxelDataPinCategory::Float   , FGuid(0x0f9e0ef6, 0x2076c764, 0xd7fb80b4, 0xd135530e), FVoxelGraphOutputsIndices::UpVectorZIndex},
	}
);

const TArray<FVoxelGraphPermutationArray> FVoxelGraphOutput::DefaultOutputsPermutations(
	{
		{
			FVoxelGraphOutputsIndices::ValueIndex
		},
		{ 
			FVoxelGraphOutputsIndices::MaterialIndex 
		},
		{
			FVoxelGraphOutputsIndices::UpVectorXIndex,
			FVoxelGraphOutputsIndices::UpVectorYIndex,
			FVoxelGraphOutputsIndices::UpVectorZIndex
		},
		{ 
			FVoxelGraphOutputsIndices::ValueIndex, 
			FVoxelGraphOutputsIndices::RangeAnalysisIndex
		}
	}
);

FString FVoxelGraphOutputsUtils::GetPermutationName(const FVoxelGraphPermutationArray& Permutation, const TMap<uint32, FVoxelGraphOutput>& Outputs)
{
	FString Name = "";
	for (uint32 I : Permutation)
	{
		Name += Outputs[I].Name.ToString();
	}
	return Name;
}

TMap<FName, uint32> FVoxelGraphOutputsUtils::GetSingleOutputsNamesMap(
		const TArray<FVoxelGraphPermutationArray>& Permutations,
		const TMap<uint32, FVoxelGraphOutput>& Outputs,
		EVoxelDataPinCategory CategoryFilter)
{
	TMap<FName, uint32> Result;
	for (auto& Permutation : Permutations)
	{
		if (Permutation.Num() == 1)
		{
			uint32 Index = Permutation[0];
			auto& Output = Outputs[Index];
			if (Output.Category == CategoryFilter)
			{
				Result.Add(Output.Name, Index);
			}
		}
	}
	return Result;
}

bool FVoxelGraphOutputsUtils::IsVoxelGraphOutputHidden(int32 Index)
{
	return Index == FVoxelGraphOutputsIndices::RangeAnalysisIndex;
}