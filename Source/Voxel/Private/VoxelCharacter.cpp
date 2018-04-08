// Copyright 2018 Phyronnaz

#include "VoxelCharacter.h"
#include "VoxelProceduralMeshComponent.h"

void AVoxelCharacter::SetBase(UPrimitiveComponent* NewBase, const FName BoneName /*= NAME_None*/, bool bNotifyActor /*= true*/)
{
	if (!Cast<UVoxelProceduralMeshComponent>(NewBase))
	{
		Super::SetBase(NewBase, BoneName, bNotifyActor);
	}
}
