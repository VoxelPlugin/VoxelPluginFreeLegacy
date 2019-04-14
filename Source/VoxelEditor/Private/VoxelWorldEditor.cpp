// Copyright 2019 Phyronnaz

#include "VoxelWorldEditor.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelSave.h"

#include "Details/VoxelEditorDetailsUtils.h"
#include "Factories/VoxelWorldSaveObjectFactory.h"

#include "EditorSupportDelegates.h"
#include "LevelEditorViewport.h"
#include "Editor.h"

class FVoxelWorldEditor : public IVoxelWorldEditor
{
public:
	FVoxelWorldEditor() = default;

	virtual UVoxelWorldSaveObject* CreateSaveObject() override
	{
		return Cast<UVoxelWorldSaveObject>(FVoxelEditorDetailsUtils::CreateAssetWithDialog(UVoxelWorldSaveObject::StaticClass(), NewObject<UVoxelWorldSaveObjectFactory>()));
	}

	virtual void BindEditorDelegates(UObject* Object) override
	{
		if (auto* World = Cast<AVoxelWorld>(Object))
		{
			if (!FEditorDelegates::PreSaveWorld.IsBoundToObject(World))
			{
				FEditorDelegates::PreSaveWorld.AddUObject(World, &AVoxelWorld::OnPreSaveWorld);
			}
			if (!FEditorDelegates::PreBeginPIE.IsBoundToObject(World))
			{
				FEditorDelegates::PreBeginPIE.AddUObject(World, &AVoxelWorld::OnPreBeginPIE);
			}
			if (!FEditorDelegates::EndPIE.IsBoundToObject(World))
			{
				FEditorDelegates::EndPIE.AddUObject(World, &AVoxelWorld::OnEndPIE);
			}
			if (!FEditorSupportDelegates::PrepareToCleanseEditorObject.IsBoundToObject(World))
			{
				FEditorSupportDelegates::PrepareToCleanseEditorObject.AddUObject(World, &AVoxelWorld::OnPrepareToCleanseEditorObject);
			}
			if (!FCoreDelegates::OnPreExit.IsBoundToObject(World))
			{
				FCoreDelegates::OnPreExit.AddUObject(World, &AVoxelWorld::OnPreExit);
			}
		}
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

	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		auto* Viewport = GEditor->GetActiveViewport();
		if (Viewport)
		{
			auto* Client = static_cast<FLevelEditorViewportClient*>(Viewport->GetClient());
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
	else
	{
		Destroy();
	}
}


void UVoxelInvokerEditorComponent::OnRegister()
{
	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		Super::OnRegister();
	}
	else
	{
		USceneComponent::OnRegister();
	}
}