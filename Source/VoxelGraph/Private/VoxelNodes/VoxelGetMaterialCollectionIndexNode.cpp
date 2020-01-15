// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelGetMaterialCollectionIndexNode.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelNodeFunctions.h"
#include "VoxelWorldGeneratorInit.h"
#include "VoxelRender/VoxelMaterialCollection.h"
#include "VoxelGraphGenerator.h"

#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#include "UObject/Package.h"
#include "AssetData.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialInstanceConstant.h"

UVoxelNode_GetMaterialCollectionIndex::UVoxelNode_GetMaterialCollectionIndex()
{
	SetOutputs(EVoxelPinCategory::Int);
}

FText UVoxelNode_GetMaterialCollectionIndex::GetTitle() const
{
	return FText::Format(NSLOCTEXT("Voxel", "GetMaterialCollectionIndexTitle", "Get Material Collection Index: {0}"), Super::GetTitle());
}

UObject* UVoxelNode_GetMaterialCollectionIndex::GetAsset() const
{
	return MaterialObject;
}

UClass* UVoxelNode_GetMaterialCollectionIndex::GetAssetClass() const
{
	return UObject::StaticClass();
}

void UVoxelNode_GetMaterialCollectionIndex::SetAsset(UObject* Object)
{
	MaterialObject = Object;
}

bool UVoxelNode_GetMaterialCollectionIndex::ShouldFilterAsset(const FAssetData& Asset) const
{
	UClass* Class = Asset.GetClass();
	return
		!Class->IsChildOf(UMaterialFunction::StaticClass()) &&
		!Class->IsChildOf(UMaterialInstanceConstant::StaticClass());
}

