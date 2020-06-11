// Copyright 2020 Phyronnaz

#include "VoxelWorldEditorControls.h"

#include "LevelEditorViewport.h"
#include "Editor.h"

AVoxelWorldEditorControls::AVoxelWorldEditorControls()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = Invoker = CreateDefaultSubobject<UVoxelInvokerEditorComponent>(FName("Editor Invoker"));
}

void AVoxelWorldEditorControls::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->WorldType == EWorldType::Editor ||
		GetWorld()->WorldType == EWorldType::EditorPreview)
	{
		if (bOverrideLocation)
		{
			SetActorLocation(LocationOverride);
		}
		else
		{
			FViewport* Viewport = GEditor->GetActiveViewport();
			if (Viewport)
			{
				FViewportClient* Client = Viewport->GetClient();
				if (Client)
				{
					for (FEditorViewportClient* EditorViewportClient : GEditor->GetAllViewportClients())
					{
						if (EditorViewportClient == Client)
						{
							const FVector CameraPosition = EditorViewportClient->GetViewLocation();
							SetActorLocation(CameraPosition);
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		Destroy();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelInvokerEditorComponent::UVoxelInvokerEditorComponent()
{
	bUseForLOD = true;
	LODToSet = 0;
	LODRange = 10000;

	bUseForCollisions = false;
	bUseForNavmesh = false;
}

void UVoxelInvokerEditorComponent::OnRegister()
{
	Super::OnRegister();

	if (GetWorld()->WorldType != EWorldType::Editor &&
		GetWorld()->WorldType != EWorldType::EditorPreview)
	{
		DisableInvoker();
	}
}