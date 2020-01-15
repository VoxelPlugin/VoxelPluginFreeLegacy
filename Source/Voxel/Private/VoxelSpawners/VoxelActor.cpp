// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelActor.h"
#include "VoxelComponents/VoxelAutoDisableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "RenderingThread.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Private/Materials/MaterialInstanceSupport.h"

AVoxelSpawnerActorWithStaticMesh::AVoxelSpawnerActorWithStaticMesh()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Static Mesh Component");
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->bUseDefaultCollision = true;
	SetRootComponent(StaticMeshComponent);
}

void AVoxelSpawnerActorWithStaticMesh::SetStaticMesh_Implementation(UStaticMesh* Mesh)
{
	StaticMeshComponent->SetSimulatePhysics(false);
	StaticMeshComponent->UnregisterComponent();
	StaticMeshComponent->SetStaticMesh(Mesh);
	StaticMeshComponent->RegisterComponent();
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

template <typename ParameterType>
void GameThread_UpdateMIParameter(const UMaterialInstance* Instance, const ParameterType& Parameter)
{
	FMaterialInstanceResource* Resource = Instance->Resource;
	const FMaterialParameterInfo& ParameterInfo = Parameter.ParameterInfo;
	typename ParameterType::ValueType Value = ParameterType::GetValue(Parameter);
	ENQUEUE_RENDER_COMMAND(SetMIParameterValue)(
		[Resource, ParameterInfo, Value](FRHICommandListImmediate& RHICmdList)
		{
			Resource->RenderThread_UpdateParameter(ParameterInfo, Value);
		});
}

void AVoxelSpawnerActorWithStaticMesh::SetInstanceRandom_Implementation(float Value)
{
	for (int32 Index = 0; Index < StaticMeshComponent->GetNumMaterials(); Index++)
	{
		if (auto* Material = StaticMeshComponent->GetMaterial(Index))
		{
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, StaticMeshComponent);
			if (DynamicMaterial)
			{
				// Do it manually, as for some values it's not updated
				DynamicMaterial->SetScalarParameterValue("PerInstanceRandom", PI);
				FScalarParameterValue* ParameterValue = GameThread_FindParameterByName(DynamicMaterial->ScalarParameterValues, FMaterialParameterInfo("PerInstanceRandom"));
				if (ensure(ParameterValue))
				{
					ParameterValue->ParameterValue = Value;
					// Update the material instance data in the rendering thread.
					GameThread_UpdateMIParameter(DynamicMaterial, *ParameterValue);
					DynamicMaterial->RecacheUniformExpressions(false);
				}
				StaticMeshComponent->SetMaterial(Index, DynamicMaterial);
			}
		}
	}
}

AVoxelSpawnerActorWithStaticMeshAndAutoDisable::AVoxelSpawnerActorWithStaticMeshAndAutoDisable()
{
	AutoDisableComponent = CreateDefaultSubobject<UVoxelAutoDisableComponent>("Voxel Auto Disable Component");
}