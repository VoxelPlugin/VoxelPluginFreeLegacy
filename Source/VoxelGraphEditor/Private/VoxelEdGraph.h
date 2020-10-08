// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "VoxelEdGraph.generated.h"

class UVoxelGraphGenerator;

UCLASS()
class UVoxelEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UVoxelGraphGenerator* GetGenerator() const;
};