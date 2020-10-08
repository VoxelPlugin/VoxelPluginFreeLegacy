// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelAssetTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.inl"
#include "VoxelGenerators/VoxelGeneratorInstanceWrapper.h"
#include "VoxelWorld.h"

inline TVoxelSharedPtr<FVoxelTransformableGeneratorInstance> ImportAssetHelper(
	const FString& FunctionName,
	AVoxelWorld* World,
	UVoxelTransformableGeneratorInstanceWrapper* Asset,
	FTransform& Transform,
	FVoxelIntBox& Bounds,
	bool bConvertToVoxelSpace)
{
	Transform = FVoxelToolHelpers::GetRealTransform(World, Transform, bConvertToVoxelSpace);
	
	if (!Asset || !Asset->IsValid())
	{
		FVoxelMessages::Error(FunctionName + ": Invalid asset");
		return nullptr;
	}
	if (!Bounds.IsValid())
	{
		if (Asset->Instance->HasBounds())
		{
			Bounds = Asset->Instance->GetBounds().ApplyTransform(Transform);
		}
		else
		{
			FVoxelMessages::Error(FunctionName + ": Invalid Bounds, and cannot deduce them from Asset");
			return nullptr;
		}
	}

	return Asset->Instance;
}

void UVoxelAssetTools::ImportAssetAsReference(
	FVoxelAssetItemReference& Reference,
	AVoxelWorld* World,
	UVoxelTransformableGeneratorInstanceWrapper* Asset,
	FTransform Transform,
	FVoxelIntBox Bounds,
	int32 Priority,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	const auto AssetInstance = ImportAssetHelper(__FUNCTION__, World, Asset, Transform, Bounds, bConvertToVoxelSpace);
	if (!AssetInstance) return;

	auto& Data = World->GetData();

	{
		FVoxelWriteScopeLock Lock(Data, Bounds, FUNCTION_FNAME);
		Reference.Bounds = Bounds;
		Reference.Item = Data.AddItem<FVoxelAssetItem>(
			AssetInstance.ToSharedRef(),
			Bounds,
			Transform,
			Priority);
	}

	if (bUpdateRender)
	{
		FVoxelToolHelpers::UpdateWorld(World, Bounds);
	}
}

void UVoxelAssetTools::ImportAssetAsReferenceAsync(
	UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
	FVoxelAssetItemReference& Reference,
	AVoxelWorld* World,
	UVoxelTransformableGeneratorInstanceWrapper* Asset,
	FTransform Transform,
	FVoxelIntBox Bounds,
	int32 Priority,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	const auto AssetInstance = ImportAssetHelper(__FUNCTION__, World, Asset, Transform, Bounds, bConvertToVoxelSpace);
	if (!AssetInstance) return;

	FVoxelToolHelpers::StartAsyncLatentAction_WithWorld_WithValue(
		WorldContextObject,
		LatentInfo,
		World,
		FUNCTION_FNAME,
		bHideLatentWarnings,
		Reference,
		[Bounds, AssetInstance, Transform, Priority](FVoxelData& Data, FVoxelAssetItemReference& InReference)
		{
			FVoxelWriteScopeLock Lock(Data, Bounds, FUNCTION_FNAME);
			InReference.Bounds = Bounds;
			InReference.Item = Data.AddItem<FVoxelAssetItem>(
				AssetInstance.ToSharedRef(),
				Bounds,
				Transform,
				Priority);
		},
		bUpdateRender ? EVoxelUpdateRender::UpdateRender : EVoxelUpdateRender::DoNotUpdateRender,
		Bounds);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelAssetTools::ImportModifierAssetImpl(
	FVoxelData& Data, 
	const FVoxelIntBox& Bounds, 
	const FTransform& Transform, 
	const FVoxelTransformableGeneratorInstance& Instance,
	bool bModifyValues,
	bool bModifyMaterials)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	FVoxelOctreeUtilities::IterateTreeInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Tree) 
	{
		if (Tree.IsLeaf())
		{
			ensureThreadSafe(Tree.IsLockedForWrite());
			auto& Leaf = Tree.AsLeaf();
			FVoxelDataUtilities::AddAssetItemToLeafData(Data, Leaf, Instance, Bounds, Transform, bModifyValues, bModifyMaterials);
		}
		else
		{
			auto& Parent = Tree.AsParent();
			if (!Parent.HasChildren())
			{
				ensureThreadSafe(Parent.IsLockedForWrite());
				Parent.CreateChildren();
			}
		}
	});
}

void UVoxelAssetTools::ImportModifierAsset(
	AVoxelWorld* World,
	UVoxelTransformableGeneratorInstanceWrapper* Asset,
	FTransform Transform,
	FVoxelIntBox Bounds,
	bool bModifyValues,
	bool bModifyMaterials,
	bool bLockEntireWorld,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	const auto AssetInstance = ImportAssetHelper(__FUNCTION__, World, Asset, Transform, Bounds, bConvertToVoxelSpace);
	if (!AssetInstance) return;

	CHECK_BOUNDS_ARE_VALID_VOID();
	CHECK_BOUNDS_ARE_32BITS_VOID();

	auto& Data = World->GetData();
	{
		FVoxelWriteScopeLock Lock(Data, bLockEntireWorld ? FVoxelIntBox::Infinite : Bounds, FUNCTION_FNAME);
		ImportModifierAssetImpl(Data, Bounds, Transform, *AssetInstance, bModifyValues, bModifyMaterials);
	}
	FVoxelToolHelpers::UpdateWorld(World, Bounds);
}

void UVoxelAssetTools::ImportModifierAssetAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	UVoxelTransformableGeneratorInstanceWrapper* Asset,
	FTransform Transform,
	FVoxelIntBox Bounds,
	bool bModifyValues,
	bool bModifyMaterials,
	bool bLockEntireWorld,
	bool bConvertToVoxelSpace,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	const auto AssetInstance = ImportAssetHelper(__FUNCTION__, World, Asset, Transform, Bounds, bConvertToVoxelSpace);
	if (!AssetInstance) return;

	CHECK_BOUNDS_ARE_VALID_VOID();
	CHECK_BOUNDS_ARE_32BITS_VOID();

	FVoxelToolHelpers::StartAsyncLatentAction_WithWorld(
		WorldContextObject,
		LatentInfo,
		World,
		FUNCTION_FNAME,
		bHideLatentWarnings,
		[=](FVoxelData& Data)
		{
			FVoxelWriteScopeLock Lock(Data, bLockEntireWorld ? FVoxelIntBox::Infinite : Bounds, FUNCTION_FNAME);
			ImportModifierAssetImpl(Data, Bounds, Transform, *AssetInstance, bModifyValues, bModifyMaterials);
		},
		EVoxelUpdateRender::UpdateRender, Bounds);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename TA, typename TB>
void ImportAssetImplImpl(
	FVoxelData& Data,
	const FVoxelIntBox& Bounds,
	bool bSubtractive,
	EVoxelAssetMergeMode MergeMode,
	TA GetInstanceValue,
	TB GetInstanceMaterial,
	uint32 MaterialMask)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());

	const auto GetInstanceMaterialImpl = [&](int32 X, int32 Y, int32 Z, FVoxelMaterial Material)
	{
		const auto NewMaterial = GetInstanceMaterial(X, Y, Z);
		Material.CopyFrom(NewMaterial, MaterialMask);
		return Material;
	};
	
	switch (MergeMode)
	{
	case EVoxelAssetMergeMode::AllValues:
	{
		Data.Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
		{
			Value = GetInstanceValue(X, Y, Z);
		});
		break;
	}
	case EVoxelAssetMergeMode::AllMaterials:
	{
		Data.Set<FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelMaterial& Material)
		{
			Material = GetInstanceMaterialImpl(X, Y, Z, Material);
		});
		break;
	}
	case EVoxelAssetMergeMode::AllValuesAndAllMaterials:
	{
		Data.Set<FVoxelValue, FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value, FVoxelMaterial& Material)
		{
			Value = GetInstanceValue(X, Y, Z);
			Material = GetInstanceMaterialImpl(X, Y, Z, Material);
		});
		break;
	}
	case EVoxelAssetMergeMode::InnerValues:
	{
		Data.Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
		{
			const auto InstanceValue = GetInstanceValue(X, Y, Z);
			Value = FVoxelUtilities::MergeAsset(Value, InstanceValue, bSubtractive);
		});
		break;
	}
	case EVoxelAssetMergeMode::InnerMaterials:
	{
		Data.Set<const FVoxelValue, FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, const FVoxelValue& Value, FVoxelMaterial& Material)
		{
			const auto InstanceValue = GetInstanceValue(X, Y, Z);
			const auto NewValue = FVoxelUtilities::MergeAsset(Value, InstanceValue, bSubtractive);
			if (NewValue == InstanceValue)
			{
				Material = GetInstanceMaterialImpl(X, Y, Z, Material);
			}
		});
		break;
	}
	case EVoxelAssetMergeMode::InnerValuesAndInnerMaterials:
	{
		Data.Set<FVoxelValue, FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value, FVoxelMaterial& Material)
		{
			const auto InstanceValue = GetInstanceValue(X, Y, Z);
			const auto NewValue = FVoxelUtilities::MergeAsset(Value, InstanceValue, bSubtractive);
			Value = NewValue;
			if (NewValue == InstanceValue)
			{
				Material = GetInstanceMaterialImpl(X, Y, Z, Material);
			}
		});
		break;
	}
	default: ensure(false);
	}
}

void UVoxelAssetTools::ImportAssetImpl(
	FVoxelData& Data,
	const FVoxelIntBox& Bounds,
	const FTransform& Transform,
	const FVoxelTransformableGeneratorInstance& Instance,
	bool bSubtractive,
	EVoxelAssetMergeMode MergeMode,
	uint32 MaterialMask)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	bool bNeedValues;
	bool bNeedMaterials;
	switch (MergeMode)
	{
	case EVoxelAssetMergeMode::AllValues: 
	{
		bNeedValues = true;
		bNeedMaterials = false;
		break;
	}
	case EVoxelAssetMergeMode::AllMaterials: 
	{
		bNeedValues = false;
		bNeedMaterials = true;
		break;
	}
	case EVoxelAssetMergeMode::AllValuesAndAllMaterials: 
	{
		bNeedValues = true;
		bNeedMaterials = true;
		break;
	}
	case EVoxelAssetMergeMode::InnerValues: 
	{
		bNeedValues = true;
		bNeedMaterials = false;
		break;
	}
	case EVoxelAssetMergeMode::InnerMaterials:
	{
		bNeedValues = true;
		bNeedMaterials = true;
		break;
	}
	case EVoxelAssetMergeMode::InnerValuesAndInnerMaterials:
	{
		bNeedValues = true;
		bNeedMaterials = true;
		break;
	}
	default:
	{
		bNeedValues = true;
		bNeedMaterials = true;
		ensure(false);
	}
	}

	const auto Size = Bounds.Size();
	check(FVoxelUtilities::CountIs32Bits(Size));

	TArray<FVoxelValue> Values;
	if (bNeedValues)
	{
		Values.SetNumUninitialized(Size.X * Size.Y * Size.Z);
		TVoxelQueryZone<FVoxelValue> QueryZone(Bounds, Values);
		Instance.GetValues_Transform(Transform, QueryZone, 0, FVoxelItemStack::Empty);
	}
	
	TArray<FVoxelMaterial> Materials;
	if (bNeedMaterials)
	{
		Materials.SetNumUninitialized(Size.X * Size.Y * Size.Z);
		TVoxelQueryZone<FVoxelMaterial> QueryZone(Bounds, Materials);
		Instance.GetMaterials_Transform(Transform, QueryZone, 0, FVoxelItemStack::Empty);
	}

	const auto GetIndex = [&](int32 X, int32 Y, int32 Z)
	{
		checkVoxelSlow(Bounds.Contains(X, Y, Z));
		const int32 LX = X - Bounds.Min.X;
		const int32 LY = Y - Bounds.Min.Y;
		const int32 LZ = Z - Bounds.Min.Z;
		checkVoxelSlow(0 <= LX && LX < Size.X);
		checkVoxelSlow(0 <= LY && LY < Size.Y);
		checkVoxelSlow(0 <= LZ && LZ < Size.Z);
		return LX + LY * Size.X + LZ * Size.X * Size.Y;
	};

	const auto GetInstanceValue = [&](int32 X, int32 Y, int32 Z)
	{
		checkVoxelSlow(bNeedValues);
		const int32 Index = GetIndex(X, Y, Z);
		checkVoxelSlow(Values.IsValidIndex(Index));
		return Values.GetData()[Index];
	};
	const auto GetInstanceMaterial = [&](int32 X, int32 Y, int32 Z)
	{
		checkVoxelSlow(bNeedMaterials);
		const int32 Index = GetIndex(X, Y, Z);
		checkVoxelSlow(Materials.IsValidIndex(Index));
		return Materials.GetData()[Index];
	};
	
	ImportAssetImplImpl(
		Data,
		Bounds,
		bSubtractive,
		MergeMode,
		GetInstanceValue,
		GetInstanceMaterial,
		MaterialMask);
}

void UVoxelAssetTools::ImportDataAssetImpl(
	FVoxelData& Data,
	const FVoxelIntBox& Bounds,
	const FVoxelVector& Position,
	const FVoxelDataAssetData& AssetData,
	bool bSubtractive,
	EVoxelAssetMergeMode MergeMode,
	uint32 MaterialMask)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	ensure(Bounds == FVoxelIntBox(Position, Position + AssetData.GetSize()));

	const FVoxelValue DefaultValue = bSubtractive ? FVoxelValue::Full() : FVoxelValue::Empty();
	const auto GetInstanceValue = [&](int32 X, int32 Y, int32 Z)
	{
		const v_flt NewValueFlt = AssetData.GetInterpolatedValue(
			X - Position.X,
			Y - Position.Y,
			Z - Position.Z,
			DefaultValue);
		return FVoxelValue(NewValueFlt);
	};
	const auto GetInstanceMaterial = [&](int32 X, int32 Y, int32 Z)
	{
		return AssetData.HasMaterials() ? AssetData.GetInterpolatedMaterial(
			X - Position.X,
			Y - Position.Y,
			Z - Position.Z) : FVoxelMaterial::Default();
	};
	ImportAssetImplImpl(
		Data,
		Bounds,
		bSubtractive,
		MergeMode,
		GetInstanceValue,
		GetInstanceMaterial,
		MaterialMask);
}

void UVoxelAssetTools::ImportAsset(
	AVoxelWorld* World,
	UVoxelTransformableGeneratorInstanceWrapper* Asset,
	FTransform Transform,
	FVoxelIntBox Bounds,
	bool bSubtractive,
	EVoxelAssetMergeMode MergeMode,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	const auto AssetInstance = ImportAssetHelper(__FUNCTION__, World, Asset, Transform, Bounds, bConvertToVoxelSpace);
	if (!AssetInstance) return;

	CHECK_BOUNDS_ARE_VALID_VOID();
	CHECK_BOUNDS_ARE_32BITS_VOID();

	VOXEL_TOOL_HELPER_BODY(Write, UpdateRender,
		ImportAssetImpl(
			Data,
			Bounds,
			Transform,
			*AssetInstance,
			bSubtractive,
			MergeMode,
			EVoxelMaterialMask::All));
}

void UVoxelAssetTools::ImportAssetAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	UVoxelTransformableGeneratorInstanceWrapper* Asset,
	FTransform Transform,
	FVoxelIntBox Bounds,
	bool bSubtractive,
	EVoxelAssetMergeMode MergeMode,
	bool bConvertToVoxelSpace,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	const auto AssetInstance = ImportAssetHelper(__FUNCTION__, World, Asset, Transform, Bounds, bConvertToVoxelSpace);
	if (!AssetInstance) return;

	CHECK_BOUNDS_ARE_VALID_VOID();
	CHECK_BOUNDS_ARE_32BITS_VOID();

	VOXEL_TOOL_LATENT_HELPER_BODY(Write, UpdateRender,
		ImportAssetImpl(
			Data,
			Bounds,
			Transform,
			*AssetInstance,
			bSubtractive,
			MergeMode,
			EVoxelMaterialMask::All));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelAssetTools::ImportDataAssetFast(
	AVoxelWorld* World, 
	UVoxelDataAsset* Asset, 
	FVector InPosition, 
	EVoxelAssetMergeMode MergeMode, 
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (!Asset)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid asset"));
		return;
	}

	FVoxelVector Position = FVoxelToolHelpers::GetRealPosition(World, InPosition, bConvertToVoxelSpace);
	Position += Asset->PositionOffset;
	
	const auto AssetData = Asset->GetData();
	const FVoxelIntBox Bounds(Position, Position + AssetData->GetSize());
	const bool bSubtractiveAsset = Asset->bSubtractiveAsset;

	CHECK_BOUNDS_ARE_VALID_VOID();
	CHECK_BOUNDS_ARE_32BITS_VOID();
	
	VOXEL_TOOL_HELPER_BODY(Write, UpdateRender,
		ImportDataAssetImpl(
			Data,
			Bounds,
			Position,
			*AssetData,
			bSubtractiveAsset,
			MergeMode,
			EVoxelMaterialMask::All));
}

void UVoxelAssetTools::ImportDataAssetFastAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World, 
	UVoxelDataAsset* Asset, 
	FVector InPosition, 
	EVoxelAssetMergeMode MergeMode, 
	bool bConvertToVoxelSpace,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (!Asset)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid asset"));
		return;
	}

	FVoxelVector Position = FVoxelToolHelpers::GetRealPosition(World, InPosition, bConvertToVoxelSpace);
	Position += Asset->PositionOffset;
	const auto AssetData = Asset->GetData();
	const FVoxelIntBox Bounds(Position, Position + AssetData->GetSize());
	const bool bSubtractiveAsset = Asset->bSubtractiveAsset;
	
	CHECK_BOUNDS_ARE_VALID_VOID();
	CHECK_BOUNDS_ARE_32BITS_VOID();
	
	VOXEL_TOOL_LATENT_HELPER_BODY(Write, UpdateRender,
		ImportDataAssetImpl(
			Data,
			Bounds,
			Position,
			*AssetData,
			bSubtractiveAsset,
			MergeMode,
			EVoxelMaterialMask::All));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelAssetTools::InvertDataAssetImpl(const FVoxelDataAssetData& AssetData, FVoxelDataAssetData& InvertedAssetData)
{
	VOXEL_TOOL_FUNCTION_COUNTER(AssetData.GetSize().X * AssetData.GetSize().Y * AssetData.GetSize().Z);

	InvertedAssetData.SetSize(AssetData.GetSize(), AssetData.HasMaterials());
	const int32 Num = AssetData.GetRawValues().Num();
#if VOXEL_DEBUG
	const auto& Src = AssetData.GetRawValues();
	auto& Dst = InvertedAssetData.GetRawValues();
#else
	const auto* RESTRICT Src = AssetData.GetRawValues().GetData();
	auto* RESTRICT Dst = InvertedAssetData.GetRawValues().GetData();
#endif
	for (int32 Index = 0; Index < Num; Index++)
	{
		Dst[Index] = Src[Index].GetInverse();
	}

	InvertedAssetData.GetRawMaterials() = AssetData.GetRawMaterials();
}

void UVoxelAssetTools::InvertDataAsset(UVoxelDataAsset* Asset, UVoxelDataAsset*& InvertedAsset)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Asset)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid asset"));
		return;
	}

	const auto InvertedAssetData = MakeVoxelShared<FVoxelDataAssetData>();
	InvertDataAssetImpl(*Asset->GetData(), *InvertedAssetData);
	
	InvertedAsset = NewObject<UVoxelDataAsset>(GetTransientPackage());
	InvertedAsset->SetData(InvertedAssetData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelAssetTools::SetDataAssetMaterialImpl(
	const FVoxelDataAssetData& AssetData,
	FVoxelDataAssetData& NewAssetData,
	FVoxelMaterial Material)
{
	VOXEL_TOOL_FUNCTION_COUNTER(AssetData.GetSize().X * AssetData.GetSize().Y * AssetData.GetSize().Z);

	NewAssetData.SetSize(AssetData.GetSize(), true);
	NewAssetData.GetRawValues() = AssetData.GetRawValues();
	const int32 Num = AssetData.GetRawValues().Num();
#if VOXEL_DEBUG
	auto& Ptr = NewAssetData.GetRawMaterials();
#else
	auto* RESTRICT Ptr = NewAssetData.GetRawMaterials().GetData();
#endif
	for (int32 Index = 0; Index < Num; Index++)
	{
		Ptr[Index] = Material;
	}
}

void UVoxelAssetTools::SetDataAssetMaterial(UVoxelDataAsset* Asset, UVoxelDataAsset*& NewAsset, FVoxelMaterial Material)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Asset)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid asset"));
		return;
	}

	const auto NewAssetData = MakeVoxelShared<FVoxelDataAssetData>();
	SetDataAssetMaterialImpl(*Asset->GetData(), *NewAssetData, Material);
	
	NewAsset = NewObject<UVoxelDataAsset>(GetTransientPackage());
	NewAsset->SetData(NewAssetData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelAssetTools::CreateDataAssetFromWorldSectionImpl(
	const FVoxelData& Data,
	const FVoxelIntBox& Bounds,
	const bool bCopyMaterials,
	FVoxelDataAssetData& AssetData)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	
	AssetData.SetSize(Bounds.Size(), bCopyMaterials);

	{
		TVoxelQueryZone<FVoxelValue> QueryZone(Bounds, AssetData.GetRawValues());
		Data.Get<FVoxelValue>(QueryZone, 0);
	}
	if (bCopyMaterials)
	{
		TVoxelQueryZone<FVoxelMaterial> QueryZone(Bounds, AssetData.GetRawMaterials());
		Data.Get<FVoxelMaterial>(QueryZone, 0);
	}
}

UVoxelDataAsset* UVoxelAssetTools::CreateDataAssetFromWorldSection(
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	bool bCopyMaterials)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	CHECK_BOUNDS_ARE_VALID();

	const auto AssetData = MakeVoxelShared<FVoxelDataAssetData>();

	VOXEL_TOOL_HELPER_BODY(Read, DoNotUpdateRender, CreateDataAssetFromWorldSectionImpl(Data, Bounds, bCopyMaterials, *AssetData));
	
	auto* Asset = NewObject<UVoxelDataAsset>(GetTransientPackage());
	Asset->SetData(AssetData);
	return Asset;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelAssetTools::AddDisableEditsBox(
	FVoxelDisableEditsBoxItemReference& Reference,
	AVoxelWorld* World, 
	FVoxelIntBox Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_BOUNDS_ARE_VALID_VOID();

	VOXEL_TOOL_HELPER_BODY(Write, DoNotUpdateRender,
	{
		Reference.Bounds = Bounds;
		Reference.Item = Data.AddItem<FVoxelDisableEditsBoxItem>(Bounds);
	});
}

void UVoxelAssetTools::AddDisableEditsBoxAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	FVoxelDisableEditsBoxItemReference& Reference,
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_BOUNDS_ARE_VALID_VOID();
	
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE_BODY(Reference, Write, DoNotUpdateRender,
	{
		InReference.Bounds = Bounds;
		InReference.Item = Data.AddItem<FVoxelDisableEditsBoxItem>(Bounds);
	});
}