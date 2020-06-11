// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceVoxelDataAsset.generated.h"

class UVoxelDataAsset;
class UNiagaraDataInterfaceVoxelDataAsset;
struct FVoxelDataAssetData;

struct FNDIVoxelDataAsset_InstanceData
{
	TArray<FVector> Positions;
	TVoxelSharedPtr<const FVoxelDataAssetData> Data;

	bool Init(UNiagaraDataInterfaceVoxelDataAsset* Interface, FNiagaraSystemInstance* SystemInstance);
};

/** Data Interface allowing sampling of a voxel data asset */
UCLASS(EditInlineNew, Category = "Voxel", meta = (DisplayName = "Voxel Data Asset Sample"))
class VOXELNIAGARA_API UNiagaraDataInterfaceVoxelDataAsset : public UNiagaraDataInterface
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Voxel")
	UVoxelDataAsset* Asset;

	//UObject Interface
	virtual void PostInitProperties() override;
	//UObject Interface End

	//UNiagaraDataInterface Interface
	virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual void DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual int32 PerInstanceDataSize() const override { return sizeof(FNDIVoxelDataAsset_InstanceData); }

	virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions) override;
	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction &OutFunc) override;
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override { return Target == ENiagaraSimTarget::CPUSim; }
	//UNiagaraDataInterface Interface End

	static void GetAssetValue(FVectorVMContext& Context);
	static void GetAssetColor(FVectorVMContext& Context);
	static void GetPositionFromAsset(FVectorVMContext& Context);
	static void GetNumVoxels(FVectorVMContext& Context);

	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
protected:
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;

	static const FName GetAssetValueName;
	static const FName GetAssetColorName;
	static const FName GetPositionFromAssetName;
	static const FName GetNumVoxelsName;
};