// Copyright 2020 Phyronnaz

#include "VoxelWorldEditorControls.h"
#include "VoxelUtilities/VoxelConfigUtilities.h"

#include "LevelEditorViewport.h"
#include "Editor.h"

AVoxelWorldEditorControls::AVoxelWorldEditorControls()
{
	PrimaryActorTick.bCanEverTick = true;

	Invoker = CreateDefaultSubobject<UVoxelInvokerEditorComponent>(FName("Editor Invoker"));
	RootComponent = Invoker;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		FVoxelConfigUtilities::LoadConfig(Invoker, "VoxelWorldEditorControls");
	}
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

#if WITH_EDITOR
void AVoxelWorldEditorControls::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		FVoxelConfigUtilities::SaveConfig(Invoker, "VoxelWorldEditorControls");
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelInvokerEditorComponent::UVoxelInvokerEditorComponent()
{
	bEditorOnlyInvoker = true;
	
	bUseForLOD = true;
	LODToSet = 0;
	LODRange = 10000;

	bUseForCollisions = false;
	bUseForNavmesh = false;
}