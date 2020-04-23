// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelTextureSamplerNode.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelNodeFunctions.h"
#include "Engine/Texture2D.h"

UVoxelNode_TextureSampler::UVoxelNode_TextureSampler()
{
	SetInputs(
		{ "X", EC::Float, "Coordinate between 0 and texture width" },
		{ "Y", EC::Float, "Coordinate between 0 and texture height" });
	SetOutputs(
		{ "R", EC::Float, "Red between 0 and 1" },
		{ "G", EC::Float, "Green between 0 and 1" },
		{ "B", EC::Float, "Blue between 0 and 1" },
		{ "A", EC::Float, "Alpha between 0 and 1" });
}


EVoxelPinCategory UVoxelNode_TextureSampler::GetInputPinCategory(int32 PinIndex) const
{
	return bBilinearInterpolation ? EC::Float : EC::Int;
}

FText UVoxelNode_TextureSampler::GetTitle() const
{
	return FText::Format(VOXEL_LOCTEXT("Texture: {0}"), Super::GetTitle());
}

void UVoxelNode_TextureSampler::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	
	FString Error;
	if (!FVoxelTextureUtilities::CanCreateFromTexture(Texture, Error))
	{
		ErrorReporter.AddMessageToNode(this, Error, EVoxelGraphNodeMessageType::FatalError);
	}
}

#if WITH_EDITOR
bool UVoxelNode_TextureSampler::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	return TryImportObject(Property, Object, Texture);
}

void UVoxelNode_TextureSampler::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelNode_TextureSampler, bBilinearInterpolation) &&
		GraphNode &&
		Graph)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_VoxelTextureSampler::UVoxelNode_VoxelTextureSampler()
{
	SetInputs(
		{"X", EC::Float, "Coordinate between 0 and texture width"},
		{"Y", EC::Float, "Coordinate between 0 and texture height"});
	SetOutputs(EC::Float);
}


EVoxelPinCategory UVoxelNode_VoxelTextureSampler::GetInputPinCategory(int32 PinIndex) const
{
	return bBilinearInterpolation ? EC::Float : EC::Int;
}

FText UVoxelNode_VoxelTextureSampler::GetTitle() const
{
	return FText::Format(VOXEL_LOCTEXT("Voxel Texture: {0}"), Super::GetTitle());
}

#if WITH_EDITOR
bool UVoxelNode_VoxelTextureSampler::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	return false;
}

void UVoxelNode_VoxelTextureSampler::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelNode_TextureSampler, bBilinearInterpolation) &&
		GraphNode &&
		Graph)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}
}
#endif