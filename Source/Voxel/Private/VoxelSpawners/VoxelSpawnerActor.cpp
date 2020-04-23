// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelSpawnerActor.h"
#include "VoxelComponents/VoxelPhysicsRelevancyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "RenderingThread.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Private/Materials/MaterialInstanceSupport.h"

AVoxelMeshSpawnerActor::AVoxelMeshSpawnerActor()
{
	RootComponent = StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Static Mesh Component");
}

void AVoxelMeshSpawnerActor::SetStaticMesh_Implementation(UStaticMesh* Mesh, const TMap<int32, UMaterialInterface*>& SectionsMaterials, const FBodyInstance& CollisionPresets)
{
	StaticMeshComponent->BodyInstance = CollisionPresets;
	StaticMeshComponent->BodyInstance.bSimulatePhysics = true;
	StaticMeshComponent->SetStaticMesh(Mesh);

	for (auto& It : SectionsMaterials)
	{
		StaticMeshComponent->SetMaterial(It.Key, It.Value);
	}
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

void AVoxelMeshSpawnerActor::SetInstanceRandom_Implementation(float Value)
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

AVoxelMeshWithPhysicsRelevancySpawnerActor::AVoxelMeshWithPhysicsRelevancySpawnerActor()
{
	PhysicsRelevancyComponent = CreateDefaultSubobject<UVoxelPhysicsRelevancyComponent>("Voxel Physics Relevancy Component");
}