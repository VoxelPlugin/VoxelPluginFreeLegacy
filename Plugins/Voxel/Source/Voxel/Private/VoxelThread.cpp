// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelThread.h"
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

void VoxelThread::DoThreadedWork()
{
	IsDoneCounter.Reset();
	Render->CreateSection(Section);
	IsDoneCounter.Increment();
}

void VoxelThread::Abandon()
{
	delete this;
}

bool VoxelThread::IsDone()
{
	return IsDoneCounter.GetValue();
}

FProcMeshSection& VoxelThread::GetSection()
{
	return Section;
}
