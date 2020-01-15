// Copyright 2020 Phyronnaz

#pragma once

#include "GameFramework/Character.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelCharacter.generated.h"

UCLASS(config=Game, BlueprintType)
class VOXEL_API AVoxelCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVoxelCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Sets the component the Character is walking on, used by CharacterMovement walking movement to be able to follow dynamic objects. */
	virtual void SetBase(UPrimitiveComponent* NewBase, FName BoneName, bool bNotifyActor) override;
};
