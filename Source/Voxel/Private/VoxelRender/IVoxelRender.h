// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"

class UVoxelInvokerComponent;
class UVoxelChunkComponent;
class FVoxelChunkOctree;
class AVoxelWorld;
class UTexture;

class VOXEL_API IVoxelRender
{
public:
	AVoxelWorld* const World;

	IVoxelRender(AVoxelWorld* World)
		: World(World)
	{

	}
	virtual ~IVoxelRender() = default;

	virtual void Tick(float DeltaTime) = 0;
	
	void UpdateBox(const FIntBox& Box, bool bRemoveHoles, TFunction<void()> CallbackWhenUpdated = nullptr) { UpdateBoxInternal(FIntBox(Box.Min - FIntVector(2, 2, 2), Box.Max + FIntVector(2, 2, 2)), bRemoveHoles, CallbackWhenUpdated); }	
	virtual void UpdateBoxInternal(const FIntBox& Box, bool bRemoveHoles, TFunction<void()> CallbackWhenUpdated) = 0;

	virtual uint8 GetLODAtPosition(const FIntVector& Position) const = 0;
	virtual int GetTaskCount() const = 0;

	virtual void RecreateMaterials() = 0;
	virtual void RecomputeMeshPositions() = 0;

	virtual void SetScalarParameterValue(FName ParameterName, float Value) = 0;
	virtual void SetTextureParameterValue(FName ParameterName, UTexture* Value) = 0;
	virtual void SetVectorParameterValue(FName ParameterName, FLinearColor Value) = 0;
};