// Copyright 2020 Phyronnaz

#include "NiagaraDataInterfaceVoxelDataAsset.h"
#include "NiagaraShader.h"
#include "ShaderParameterUtils.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelGlobals.h"

bool FNDIVoxelDataAsset_InstanceData::Init(UNiagaraDataInterfaceVoxelDataAsset* Interface, FNiagaraSystemInstance* SystemInstance)
{
	if (Interface->Asset)
	{
		Data = Interface->Asset->GetData();

		auto& AssetData = *Data;
		for (int32 X = 0; X < AssetData.GetSize().X; X++)
		{
			for (int32 Y = 0; Y < AssetData.GetSize().Y; Y++)
			{
				for (int32 Z = 0; Z < AssetData.GetSize().Z; Z++)
				{
					if (!AssetData.GetValueUnsafe(X, Y, Z).IsEmpty())
					{
						Positions.Emplace(X, Y, Z);
					}
				}
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

const FName UNiagaraDataInterfaceVoxelDataAsset::GetAssetValueName(TEXT("GetVoxelDataAssetValue"));
const FName UNiagaraDataInterfaceVoxelDataAsset::GetAssetColorName(TEXT("GetVoxelDataAssetColor"));
const FName UNiagaraDataInterfaceVoxelDataAsset::GetPositionFromAssetName(TEXT("GetPositionFromVoxelDataAsset"));
const FName UNiagaraDataInterfaceVoxelDataAsset::GetNumVoxelsName(TEXT("GetNumVoxels"));

void UNiagaraDataInterfaceVoxelDataAsset::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), true, false, false);
	}
}

bool UNiagaraDataInterfaceVoxelDataAsset::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FNDIVoxelDataAsset_InstanceData* Inst = new (PerInstanceData) FNDIVoxelDataAsset_InstanceData();
	return Inst->Init(this, SystemInstance);
}

void UNiagaraDataInterfaceVoxelDataAsset::DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FNDIVoxelDataAsset_InstanceData* Inst = (FNDIVoxelDataAsset_InstanceData*)PerInstanceData;
	Inst->~FNDIVoxelDataAsset_InstanceData();
}

bool UNiagaraDataInterfaceVoxelDataAsset::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	if (!Super::CopyToInternal(Destination))
	{
		return false;
	}
	UNiagaraDataInterfaceVoxelDataAsset* DestinationTexture = CastChecked<UNiagaraDataInterfaceVoxelDataAsset>(Destination);
	DestinationTexture->Asset = Asset;

	return true;
}

bool UNiagaraDataInterfaceVoxelDataAsset::Equals(const UNiagaraDataInterface* Other) const
{
	if (!Super::Equals(Other))
	{
		return false;
	}
	const UNiagaraDataInterfaceVoxelDataAsset* OtherTexture = CastChecked<const UNiagaraDataInterfaceVoxelDataAsset>(Other);
	return OtherTexture->Asset == Asset;
}

void UNiagaraDataInterfaceVoxelDataAsset::GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)
{
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetAssetValueName;
		Sig.bMemberFunction = true;
		Sig.bRequiresContext = false;
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Asset")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Position")));
		Sig.SetDescription(VOXEL_LOCTEXT("Get a voxel data asset value"));
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Value")));

		OutFunctions.Add(Sig);
	}
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetAssetColorName;
		Sig.bMemberFunction = true;
		Sig.bRequiresContext = false;
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Asset")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Position")));
		Sig.SetDescription(VOXEL_LOCTEXT("Color"));

		OutFunctions.Add(Sig);
	}
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetPositionFromAssetName;
		Sig.bMemberFunction = true;
		Sig.bRequiresContext = false;
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Asset")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		Sig.SetDescription(VOXEL_LOCTEXT("Position"));

		OutFunctions.Add(Sig);
	}
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetNumVoxelsName;
		Sig.bMemberFunction = true;
		Sig.bRequiresContext = false;
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Asset")));
		Sig.SetDescription(VOXEL_LOCTEXT("Get a voxel position from a voxel data asset"));
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Num")));

		OutFunctions.Add(Sig);
	}
}

void UNiagaraDataInterfaceVoxelDataAsset::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction &OutFunc)
{
	FNDIVoxelDataAsset_InstanceData* InstData = (FNDIVoxelDataAsset_InstanceData*)InstanceData;
	if (!InstData || !InstData->Data.IsValid())
	{
		OutFunc = FVMExternalFunction();
	}

	if (BindingInfo.Name == GetAssetValueName)
	{
		check(BindingInfo.GetNumInputs() == 4 && BindingInfo.GetNumOutputs() == 1);
		OutFunc = FVMExternalFunction::CreateStatic(&UNiagaraDataInterfaceVoxelDataAsset::GetAssetValue);
	}
	else if (BindingInfo.Name == GetAssetColorName)
	{
		check(BindingInfo.GetNumInputs() == 4 && BindingInfo.GetNumOutputs() == 4);
		OutFunc = FVMExternalFunction::CreateStatic(&UNiagaraDataInterfaceVoxelDataAsset::GetAssetColor);
	}
	else if (BindingInfo.Name == GetPositionFromAssetName)
	{
		check(BindingInfo.GetNumInputs() == 2 && BindingInfo.GetNumOutputs() == 3);
		OutFunc = FVMExternalFunction::CreateStatic(&UNiagaraDataInterfaceVoxelDataAsset::GetPositionFromAsset);
	}
	else if (BindingInfo.Name == GetNumVoxelsName)
	{
		check(BindingInfo.GetNumInputs() == 1 && BindingInfo.GetNumOutputs() == 1);
		OutFunc = FVMExternalFunction::CreateStatic(&UNiagaraDataInterfaceVoxelDataAsset::GetNumVoxels);
	}
}

void UNiagaraDataInterfaceVoxelDataAsset::GetAssetValue(FVectorVMContext& Context)
{
	VectorVM::FExternalFuncInputHandler<float> XParam(Context);
	VectorVM::FExternalFuncInputHandler<float> YParam(Context);
	VectorVM::FExternalFuncInputHandler<float> ZParam(Context);
	VectorVM::FUserPtrHandler<FNDIVoxelDataAsset_InstanceData> InstData(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutValue(Context);

	auto& Data = *InstData->Data;
	for (int32 i = 0; i < Context.NumInstances; ++i)
	{
		const float X = XParam.GetAndAdvance();
		const float Y = YParam.GetAndAdvance();
		const float Z = ZParam.GetAndAdvance();
		*OutValue.GetDestAndAdvance() = Data.GetInterpolatedValue(X, Y, Z, FVoxelValue::Empty());
	}
}

void UNiagaraDataInterfaceVoxelDataAsset::GetAssetColor(FVectorVMContext& Context)
{
	VectorVM::FExternalFuncInputHandler<float> XParam(Context);
	VectorVM::FExternalFuncInputHandler<float> YParam(Context);
	VectorVM::FExternalFuncInputHandler<float> ZParam(Context);
	VectorVM::FUserPtrHandler<FNDIVoxelDataAsset_InstanceData> InstData(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutR(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutG(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutB(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutA(Context);

	auto& Data = *InstData->Data;
	for (int32 i = 0; i < Context.NumInstances; ++i)
	{
		const float X = XParam.GetAndAdvance();
		const float Y = YParam.GetAndAdvance();
		const float Z = ZParam.GetAndAdvance();
		const FLinearColor Color = Data.GetInterpolatedMaterial(X, Y, Z).GetLinearColor();
		*OutR.GetDestAndAdvance() = Color.R;
		*OutG.GetDestAndAdvance() = Color.G;
		*OutB.GetDestAndAdvance() = Color.B;
		*OutA.GetDestAndAdvance() = Color.A;
	}
}

void UNiagaraDataInterfaceVoxelDataAsset::GetPositionFromAsset(FVectorVMContext& Context)
{
	VectorVM::FExternalFuncInputHandler<int32> IndexParam(Context);
	VectorVM::FUserPtrHandler<FNDIVoxelDataAsset_InstanceData> InstData(Context);

	VectorVM::FExternalFuncRegisterHandler<float> OutX(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutY(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutZ(Context);

	auto& Positions = InstData->Positions;
	for (int32 i = 0; i < Context.NumInstances; ++i)
	{
		const int32 Index = IndexParam.GetAndAdvance();
		FVector Position(0, 0, 0);
		if (Positions.IsValidIndex(Index))
		{
			Position = Positions[Index];
		}
		*OutX.GetDestAndAdvance() = Position.X;
		*OutY.GetDestAndAdvance() = Position.Y;
		*OutZ.GetDestAndAdvance() = Position.Z;
	}
}

void UNiagaraDataInterfaceVoxelDataAsset::GetNumVoxels(FVectorVMContext& Context)
{
	VectorVM::FUserPtrHandler<FNDIVoxelDataAsset_InstanceData> InstData(Context);
	VectorVM::FExternalFuncRegisterHandler<int32> OutNum(Context);

	const int32 Num = InstData->Positions.Num();
	for (int32 i = 0; i < Context.NumInstances; ++i)
	{
		*OutNum.GetDestAndAdvance() = Num;
	}
}