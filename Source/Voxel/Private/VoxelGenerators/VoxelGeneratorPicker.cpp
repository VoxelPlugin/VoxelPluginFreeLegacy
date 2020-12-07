// Copyright 2020 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGenerators/VoxelEmptyGenerator.h"
#include "VoxelMessages.h"

#include "UObject/Package.h"

inline void CheckOutputs(const UVoxelGenerator& Generator, const FVoxelGeneratorInstance& Instance)
{
#if VOXEL_DEBUG
	VOXEL_FUNCTION_COUNTER();

	FVoxelGeneratorOutputs Outputs = Generator.GetGeneratorOutputs();
	const auto Order = [](FName A, FName B) {return A.FastLess(B); };

	TArray<FName> FloatOutputs;
	Instance.GetOutputsPtrMap<v_flt>().GenerateKeyArray(FloatOutputs);
	FloatOutputs.Sort(Order);
	Outputs.FloatOutputs.Sort(Order);
	ensure(FloatOutputs == Outputs.FloatOutputs); // Will fail if a graph failed to compile
#endif
}

TVoxelSharedRef<FVoxelGeneratorInstance> FVoxelGeneratorPicker::GetInstance() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	auto* Generator = GetGenerator();
	if (Generator)
	{
		const auto Instance = Generator->GetInstance(Parameters);
		CheckOutputs(*Generator, *Instance);
		return Instance;
	}
	else
	{
		return MakeVoxelShared<FVoxelEmptyGeneratorInstance>();
	}
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> FVoxelTransformableGeneratorPicker::GetInstance() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	
	auto* Generator = GetGenerator();
	if (Generator)
	{
		const auto Instance = Generator->GetTransformableInstance(Parameters);
		CheckOutputs(*Generator, *Instance);
		return Instance;
	}
	else
	{
		return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
	}
}