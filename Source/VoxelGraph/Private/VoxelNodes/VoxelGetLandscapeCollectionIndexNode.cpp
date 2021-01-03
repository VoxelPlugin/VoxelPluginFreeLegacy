// Copyright 2021 Phyronnaz

#include "VoxelNodes/VoxelGetLandscapeCollectionIndexNode.h"

#include "VoxelGraphGenerator.h"
#include "VoxelGraphErrorReporter.h"


#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"

#include "Misc/UObjectToken.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"

UVoxelNode_GetLandscapeCollectionIndex::UVoxelNode_GetLandscapeCollectionIndex()
{
	SetOutputs(EVoxelPinCategory::Int);
}

FText UVoxelNode_GetLandscapeCollectionIndex::GetTitle() const
{
	return FText::Format(VOXEL_LOCTEXT("Get Landscape Collection Index: {0}"), Super::GetTitle());
}

