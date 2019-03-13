// Copyright 2019 Phyronnaz

#include "VoxelGraphEditorModule.h"

#include "Interfaces/IPluginManager.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "Voxel"

const FVector2D Icon14x14(14.0f, 14.0f);
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);
const FVector2D Icon512x512(512.0f, 512.0f);

/**
 * Implements the VoxelEditor module.
 */
class FVoxelGraphEditorModule : public IVoxelGraphEditorModule
{
public:
};

IMPLEMENT_MODULE(FVoxelGraphEditorModule, VoxelGraphEditor);

#undef LOCTEXT_NAMESPACE
#undef IMAGE_PLUGIN_BRUSH
