// Copyright 2020 Phyronnaz

#include "VoxelVDBModule.h"
#include "Modules/ModuleManager.h"
#include "VoxelVDBInclude.h"

IMPLEMENT_MODULE(FVoxelVDB, VoxelVDB)

void FVoxelVDB::StartupModule()
{
	openvdb::initialize();
}
