// Copyright 2018 Phyronnaz

#include "VoxelWorldEditor.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelWorld.h"
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
		auto Viewport = GEditor->GetActiveViewport();
		if (Viewport)
		{
			auto Client = static_cast<FLevelEditorViewportClient*>(Viewport->GetClient());
			if (Client)
			{
				FVector CameraPosition = Client->GetViewLocation();
				SetActorLocation(CameraPosition);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Cannot find editor camera"));
			}
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
