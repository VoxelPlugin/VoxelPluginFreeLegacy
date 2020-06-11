// Copyright 2020 Phyronnaz

#include "VoxelStaticWorld.h"
#include "VoxelMinimal.h"
#include "Components/StaticMeshComponent.h"

AVoxelStaticWorld::AVoxelStaticWorld()
{
}

#if WITH_EDITOR
void AVoxelStaticWorld::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty &&
		PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_STATIC(AVoxelStaticWorld, BaseMesh) &&
		PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		for (auto* Mesh : Meshes)
		{
			auto* StaticMesh = Mesh->GetStaticMesh();
			auto RelativeTransform = Mesh->GetRelativeTransform();
			Mesh->ReinitializeProperties(BaseMesh);
			Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
			Mesh->SetStaticMesh(StaticMesh);
			Mesh->SetRelativeTransform(RelativeTransform);
			Mesh->RegisterComponent();
		}
	}
}
#endif