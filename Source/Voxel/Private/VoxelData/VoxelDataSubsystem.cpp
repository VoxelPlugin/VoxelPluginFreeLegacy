// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelDataSubsystem.h"
#include "VoxelData/VoxelData.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"

DEFINE_VOXEL_SUBSYSTEM_PROXY(UVoxelDataSubsystemProxy);

void FVoxelDataSubsystem::Create()
{
	Super::Create();

	if (Settings.DataOverride)
	{
		Data = Settings.DataOverride;
	}
	else
	{
		FVoxelDataSettings DataSettings;
		DataSettings.Depth = FVoxelUtilities::ConvertDepth<RENDER_CHUNK_SIZE, DATA_CHUNK_SIZE>(Settings.RenderOctreeDepth);
		DataSettings.WorldBounds = Settings.GetWorldBounds();

		DataSettings.Generator = FVoxelGeneratorPicker(Settings.Generator).GetInstance();
		DataSettings.Generator->Init(Settings.GetGeneratorInit());

		DataSettings.bEnableMultiplayer = Settings.bEnableMultiplayer;
		DataSettings.bEnableUndoRedo = Settings.bEnableUndoRedo;

		check(!Data);
		Data = FVoxelData::Create(DataSettings, Settings.DataOctreeInitialSubdivisionDepth);
	}
}