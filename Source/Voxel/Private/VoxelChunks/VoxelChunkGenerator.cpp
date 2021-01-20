// Copyright 2021 Phyronnaz

#include "VoxelChunks/VoxelChunkGenerator.h"

TVoxelSharedRef<FVoxelGeneratorInstance> UVoxelChunkGenerator::GetInstance()
{
	return MakeVoxelShared<FVoxelChunkGeneratorInstance>(*this);
}

void FVoxelChunkGeneratorInstance::Init(const FVoxelGeneratorInit& InitStruct)
{
	const TVoxelSharedPtr<FVoxelRuntime> Runtime = InitStruct.GetRuntime();
	if (Runtime)
	{
		Data = Runtime->GetSubsystemChecked<FVoxelChunksSubsystem>().Data;
	}
}

v_flt FVoxelChunkGeneratorInstance::GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	return GetImpl<FVoxelValue>(X, Y, Z, FVoxelValue::Full(), FVoxelValue::Empty()).ToFloat();
}

FVoxelMaterial FVoxelChunkGeneratorInstance::GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	return GetImpl<FVoxelMaterial>(X, Y, Z, FVoxelMaterial::Default(), FVoxelMaterial::Default());
}

TVoxelRange<v_flt> FVoxelChunkGeneratorInstance::GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
{
	// TODO
	return { -1, 1 };
}

template<typename T>
T FVoxelChunkGeneratorInstance::GetImpl(v_flt X, v_flt Y, v_flt Z, T BelowValue, T AboveValue) const
{
	if (!Data)
	{
		return AboveValue;
	}

	if (Z < -127)
	{
		return BelowValue;
	}
	if (Z > 128)
	{
		return AboveValue;
	}
	Z += 127;

	const FIntPoint ChunkPosition = FVoxelChunkConfig::GetChunkPosition({ X, Y });
	const TVoxelSharedPtr<FVoxelChunkData> Chunk = Data->FindChunk(ChunkPosition);

	if (!Chunk)
	{
		return AboveValue;
	}

	FScopeLock Lock(&Chunk->CriticalSection);

	const FVoxelChunkData::TData<T>* ChunkData = Chunk->GetData<T>();
	if (!ChunkData)
	{
		return AboveValue;
	}

	const FIntVector Position = FIntVector(X - ChunkPosition.X, Y - ChunkPosition.Y, Z);
	return FVoxelUtilities::Get3D(*ChunkData, FVoxelChunkConfig::GetChunkSize(), Position);
}
