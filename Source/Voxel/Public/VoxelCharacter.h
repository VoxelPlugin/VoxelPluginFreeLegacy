// Copyright 2020 Phyronnaz

#pragma once

#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelCharacter.generated.h"

UCLASS(BlueprintType)
class VOXEL_API AVoxelCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// You can copy/paste this function to your own character class
	// You might have to add
	// #include "Components/PrimitiveComponent.h"
	// at the top of your character header
	// This function is required for base replication to work properly, as voxel world components are generated at runtime & not replicated
	virtual void SetBase(UPrimitiveComponent* NewBase, FName BoneName, bool bNotifyActor) override
	{
		if (NewBase)
		{
			AActor* BaseOwner = NewBase->GetOwner();
			// LoadClass to not depend on the voxel module
			static UClass* VoxelWorldClass = LoadClass<UObject>(nullptr, TEXT("/Script/Voxel.VoxelWorld"));
			if (ensure(VoxelWorldClass) && BaseOwner && BaseOwner->IsA(VoxelWorldClass))
			{
				NewBase = Cast<UPrimitiveComponent>(BaseOwner->GetRootComponent());
				ensure(NewBase);
			}
		}
		Super::SetBase(NewBase, BoneName, bNotifyActor);
	}
};