// Copyright 2020 Phyronnaz

#include "VoxelWorldEditor.h"
#include "VoxelWorld.h"
#include "VoxelPlaceableItems/VoxelAssetActor.h"
#include "VoxelData/VoxelSave.h"

#include "VoxelEditorDetailsUtilities.h"
#include "Factories/VoxelWorldSaveObjectFactory.h"

#include "LevelEditorViewport.h"
#include "Editor.h"

class FVoxelWorldEditor : public IVoxelWorldEditor
{
public:
	FVoxelWorldEditor() = default;

	virtual UVoxelWorldSaveObject* CreateSaveObject() override
	{
		return Cast<UVoxelWorldSaveObject>(FVoxelEditorUtilities::CreateAssetWithDialog(UVoxelWorldSaveObject::StaticClass(), NewObject<UVoxelWorldSaveObjectFactory>()));
	}

	virtual UClass* GetVoxelWorldEditorClass() override
	{
		return AVoxelWorldEditor::StaticClass();
	}

};

AVoxelWorldEditor::AVoxelWorldEditor()
{
	PrimaryActorTick.bCanEverTick = true;

	Invoker = CreateDefaultSubobject<UVoxelInvokerEditorComponent>(FName("Editor Invoker"));
	Invoker->bUseForCollisions = false;
	Invoker->bUseForNavmesh = false;
	RootComponent = Invoker;

	if (!IVoxelWorldEditor::GetVoxelWorldEditor())
	{
		IVoxelWorldEditor::SetVoxelWorldEditor(MakeShared<FVoxelWorldEditor>());
	}
}

void AVoxelWorldEditor::Tick(float DeltaTime)
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


void UVoxelInvokerEditorComponent::OnRegister()
{
	if (GetWorld()->WorldType == EWorldType::Editor ||
		GetWorld()->WorldType == EWorldType::EditorPreview)
	{
		Super::OnRegister();
	}
	else
	{
		USceneComponent::OnRegister();
	}
}