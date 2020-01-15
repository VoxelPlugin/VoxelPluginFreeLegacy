// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "UObject/WeakObjectPtr.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;

class VOXEL_API IVoxelMaterialInterface
{
public:
	virtual ~IVoxelMaterialInterface();

	FORCEINLINE UMaterialInterface* GetMaterial() const
	{
		// Happens if the material is recompiled
		if (MaterialInterface.IsValid())
		{
			return MaterialInterface.Get();
		}
		else
		{
			return DefaultMaterial();
		}
	}

	static TVoxelSharedRef<IVoxelMaterialInterface> Default();

private:
	explicit IVoxelMaterialInterface(UMaterialInterface* MaterialInterface);

	static UMaterialInterface* DefaultMaterial();
	
	const TWeakObjectPtr<UMaterialInterface> MaterialInterface;
	const FName MaterialName_Debug;
	
	// Friend so that static functions can access the constructor
	friend class FVoxelMaterialInterface;
	friend class FVoxelMaterialInstance;
};

class VOXEL_API FVoxelMaterialInterface : public IVoxelMaterialInterface
{
public:
	static TVoxelSharedRef<IVoxelMaterialInterface> Create(UMaterialInterface* MaterialInterface);
	virtual ~FVoxelMaterialInterface();

private:
	explicit FVoxelMaterialInterface(UMaterialInterface* MaterialInterface);
	const bool bWasAlreadyRooted;
};

class VOXEL_API FVoxelMaterialInstance : public IVoxelMaterialInterface
{
public:
	static TVoxelSharedRef<IVoxelMaterialInterface> Create(UMaterialInterface* Parent);
	virtual ~FVoxelMaterialInstance();

	static void ClearPool();
	
private:
	explicit FVoxelMaterialInstance(UMaterialInstanceDynamic* MaterialInstance);

	static TArray<TWeakObjectPtr<UMaterialInstanceDynamic>> InstancePool;
};