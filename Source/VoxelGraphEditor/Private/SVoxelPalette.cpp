// Copyright 2020 Phyronnaz

#include "SVoxelPalette.h"
#include "VoxelGraphSchema.h"

void SVoxelPalette::Construct(const FArguments& InArgs)
{
	SGraphPalette::Construct(SGraphPalette::FArguments().AutoExpandActionMenu(false));
}

void SVoxelPalette::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	const UVoxelGraphSchema* Schema = GetDefault<UVoxelGraphSchema>();

	FGraphActionMenuBuilder ActionMenuBuilder;

	Schema->GetPaletteActions(ActionMenuBuilder);
	OutAllActions.Append(ActionMenuBuilder);
}