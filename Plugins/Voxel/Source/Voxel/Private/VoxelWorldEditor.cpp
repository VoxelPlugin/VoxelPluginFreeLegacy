// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelWorldEditor.h"
#include "VoxelInvokerComponent.h"
#include "Components/CapsuleComponent.h"
#include "LevelEditorViewport.h"
#include "Editor.h"

AVoxelWorldEditor::AVoxelWorldEditor()
{
	PrimaryActorTick.bCanEverTick = true;

	Invoker = CreateDefaultSubobject<UVoxelInvokerComponent>(FName("Editor Invoker"));

	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;
}

TWeakObjectPtr<UVoxelInvokerComponent> AVoxelWorldEditor::GetInvoker()
{
	return TWeakObjectPtr<UVoxelInvokerComponent>(Invoker);
}

void AVoxelWorldEditor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!World.IsValid())
	{
		Destroy();
		return;
	}

	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		auto Client = static_cast<FLevelEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
		if (Client)
		{
			FVector CameraPosition = Client->GetViewLocation();
			SetActorLocation(CameraPosition);
		}
		else
		{
			UE_LOG(VoxelLog, Error, TEXT("Cannot find editor camera"));
		}
	}
}

#if WITH_EDITOR
bool AVoxelWorldEditor::ShouldTickIfViewportsOnly() const
{
	return true;
}

void AVoxelWorldEditor::Init(TWeakObjectPtr<AVoxelWorld> NewWorld)
{
	World = NewWorld;
}

#endif