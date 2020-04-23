// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelBiomeMapNode.h"
#include "VoxelNodes/VoxelTextureSamplerNode.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelGraphGenerator.h"
#include "VoxelNodeFunctions.h"

#include "Engine/Texture2D.h"

UVoxelNode_BiomeMapSampler::UVoxelNode_BiomeMapSampler()
{
	SetInputs(
		{ "U", EC::Float, "Coordinate between 0 and texture width" },
		{ "V", EC::Float, "Coordinate between 0 and texture height" });
}

int32 UVoxelNode_BiomeMapSampler::GetOutputPinsCount() const
{
	return Biomes.Num();
}

FName UVoxelNode_BiomeMapSampler::GetOutputPinName(int32 PinIndex) const
{
	return Biomes.IsValidIndex(PinIndex) ? *Biomes[PinIndex].Name : TEXT("Error");
}

EVoxelPinCategory UVoxelNode_BiomeMapSampler::GetOutputPinCategory(int32 PinIndex) const
{
	return EC::Float;
}

FText UVoxelNode_BiomeMapSampler::GetTitle() const
{
	return FText::Format(VOXEL_LOCTEXT("Biome Map: {0}"), Super::GetTitle());
}


TArray<FColor> UVoxelNode_BiomeMapSampler::GetColors() const
{
	TArray<FColor> Colors;
	for (auto& Biome : Biomes)
	{
		Colors.Add(Biome.Color);
	}
	return Colors;
}

void UVoxelNode_BiomeMapSampler::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	
	FString Error;
	if (!FVoxelTextureUtilities::CanCreateFromTexture(Texture, Error))
	{
		ErrorReporter.AddMessageToNode(this, Error, EVoxelGraphNodeMessageType::FatalError);
	}
}

#if WITH_EDITOR
bool UVoxelNode_BiomeMapSampler::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	return TryImportObject(Property, Object, Texture);
}

void UVoxelNode_BiomeMapSampler::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (Graph && GraphNode && PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		if (Biomes.Num() >= 256)
		{
			Biomes.SetNum(256);
		}
		for (int32 Index = 0; Index < Biomes.Num(); Index++)
		{
			auto& Biome = Biomes[Index];
			if (Biome.Name.IsEmpty())
			{
				Biome.Name = FString::Printf(TEXT("Biome %d"), Index);
			}
		}

		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}
}
#endif