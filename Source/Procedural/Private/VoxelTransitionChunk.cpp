// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelTransitionChunk.h"


AVoxelTransitionChunk::AVoxelTransitionChunk()
{

}

void AVoxelTransitionChunk::BeginPlay()
{
	Super::BeginPlay();

}

void AVoxelTransitionChunk::Update()
{

}

void AVoxelTransitionChunk::Init(FIntVector position, bool isAlongX, bool isAlongY, bool isAlongZ)
{
	Position = position;
	IsAlongX = isAlongX;
	IsAlongY = isAlongY;
	IsAlongZ = isAlongZ;
}

