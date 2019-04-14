// Copyright 2019 Phyronnaz

#include "VoxelCharacter.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelWorld.h"

void AVoxelCharacter::SetBase(UPrimitiveComponent* NewBase, const FName BoneName, bool bNotifyActor)
{
	if (auto* Component = Cast<UVoxelProceduralMeshComponent>(NewBase))
	{
		if (auto* World = Cast<AVoxelWorld>(Component->GetOwner()))
		{
			ensure(World->GetPrimitiveComponent());
			Super::SetBase(World->GetPrimitiveComponent(), BoneName, bNotifyActor);
		}
		else
		{
			UE_LOG(LogVoxel, Warning, TEXT("VoxelCharacter: Invalid Voxel Proc Mesh Component!"));
		}
	}
	else
	{
		Super::SetBase(NewBase, BoneName, bNotifyActor);
	}
}
