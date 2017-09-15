// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelThread.h"
#include <forward_list>
#include "VoxelChunk.h"
#include "VoxelData.h"

VoxelThread::VoxelThread(AVoxelChunk* VoxelChunk)
{
	Render = new VoxelRender(VoxelChunk->Depth, VoxelChunk->World->GetData(), VoxelChunk->Position, VoxelChunk->ChunkHasHigherRes);
}

VoxelThread::~VoxelThread()
{
	delete Render;
}

void VoxelThread::DoWork()
{
	Render->CreateSection(Section);
}