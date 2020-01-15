// Copyright 2020 Phyronnaz

#include "VoxelCharacter.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelWorld.h"

#include "Components/InstancedStaticMeshComponent.h"

AVoxelCharacter::AVoxelCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

inline UPrimitiveComponent* GetVoxelWorldBase(AVoxelWorld* World)
{
	auto* Root = Cast<UPrimitiveComponent>(World->GetRootComponent());
	ensure(Root != nullptr);
	return Root;
}

void AVoxelCharacter::SetBase(UPrimitiveComponent* NewBase, const FName BoneName, bool bNotifyActor)
{
	if (auto* Component = Cast<UVoxelProceduralMeshComponent>(NewBase))
	{
		if (auto* World = Cast<AVoxelWorld>(Component->GetOwner()))
		{
			Super::SetBase(GetVoxelWorldBase(World), BoneName, bNotifyActor);
		}
		else
		{
			UE_LOG(LogVoxel, Warning, TEXT("VoxelCharacter: Invalid Voxel Proc Mesh Component!"));
		}
	}
	else if (auto* InstancedComponent = Cast<UInstancedStaticMeshComponent>(NewBase))
	{
		if (auto* World = Cast<AVoxelWorld>(InstancedComponent->GetOwner()))
		{
			Super::SetBase(GetVoxelWorldBase(World), BoneName, bNotifyActor);
		}
		else
		{
			Super::SetBase(NewBase, BoneName, bNotifyActor);
		}
	}
	else
	{
		Super::SetBase(NewBase, BoneName, bNotifyActor);
	}
}
