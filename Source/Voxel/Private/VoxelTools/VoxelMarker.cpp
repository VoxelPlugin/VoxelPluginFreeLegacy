// Copyright 2019 Phyronnaz

#include "VoxelTools/VoxelMarker.h"
#include "VoxelLogStatDefinitions.h"
#include "UObject/ConstructorHelpers.h"
#include "VoxelWorld.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Components/SpotLightComponent.h"
#include "Components/DecalComponent.h"

#define CHECK_WORLD_VOXELMARKERS(Name) \
if (!World) \
{ \
	UE_LOG(LogVoxel, Error, TEXT("%s: World is NULL"), TEXT(#Name)); \
	return; \
} \
 \
if (!World->IsCreated()) \
{ \
	UE_LOG(LogVoxel, Error, TEXT("%s: World is not created"), TEXT(#Name)); \
	return; \
}

AVoxelMarker::AVoxelMarker()
{

}

void AVoxelMarker::Tick(float DeltaTime)
{
	if (GetWorld()->WorldType != EWorldType::Editor && GetWorld()->WorldType != EWorldType::EditorPreview)
	{
		Destroy();
	}
}

void AVoxelMarker::SetAsEditorMarker()
{
	PrimaryActorTick.bCanEverTick = true;
}

///////////////////////////////////////////////////////////////////////////////

AVoxelProjectionMarker::AVoxelProjectionMarker()
{
	Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	SetRootComponent(Decal);

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder(TEXT("Material'/Voxel/Markers/M_Target.M_Target'"));
 
	if (MaterialFinder.Object)
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(MaterialFinder.Object, this, "Instance");
		Decal->SetDecalMaterial(MaterialInstance);
	}
}

void AVoxelProjectionMarker::UpdateProjectionMarker(AVoxelWorld* World, EVoxelMode Mode, FVector Position, FVector Normal, float Radius, float EditDistance, float HeightOffset)
{
	CHECK_WORLD_VOXELMARKERS(UpdateProjectionMarker);

	SetActorLocation(Position + Normal * HeightOffset);
	SetActorRotation(FRotationMatrix::MakeFromX(-Normal).Rotator());

	static FName NAME_Color("Color");
	static FName NAME_Normal("Normal");

	switch (Mode)
	{
	case EVoxelMode::Add:
		MaterialInstance->SetVectorParameterValue(NAME_Color, FLinearColor::Green);
		break;
	case EVoxelMode::Remove:
		MaterialInstance->SetVectorParameterValue(NAME_Color, FLinearColor::Red);
		break;
	case EVoxelMode::Flatten:
		MaterialInstance->SetVectorParameterValue(NAME_Color, FLinearColor::Blue);
		break;
	case EVoxelMode::Paint:
		MaterialInstance->SetVectorParameterValue(NAME_Color, FLinearColor::White);
		break;
	default:
		check(false);
		break;
	}

	MaterialInstance->SetVectorParameterValue(NAME_Normal, Normal);

	Decal->DecalSize.Y = Radius;
	Decal->DecalSize.Z = Radius;
	Decal->DecalSize.X = EditDistance;
	Decal->MarkRenderStateDirty();
}

///////////////////////////////////////////////////////////////////////////////

AVoxelSphereMarker::AVoxelSphereMarker()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);

	StaticMesh->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), this, TEXT("/Engine/BasicShapes/Sphere.Sphere"))));
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialParentFinder(TEXT("Material'/Voxel/Markers/M_VoxelMarker.M_VoxelMarker'"));
 
	if (MaterialParentFinder.Object)
	{
		MaterialParent = MaterialParentFinder.Object;
	}
}

void AVoxelSphereMarker::UpdateSphereMarker(AVoxelWorld* World, EVoxelMode Mode, FIntVector Position, float Radius)
{
	CHECK_WORLD_VOXELMARKERS(UpdateSphereMarker);

	if (!Material && MaterialParent)
	{
		Material = UMaterialInstanceDynamic::Create(MaterialParent, this);
		StaticMesh->SetMaterial(0, Material);
	}

	SetActorLocation(World->LocalToGlobal(Position));
	SetActorScale3D(FVector::OneVector * Radius * World->GetVoxelSize() / 50);

	switch (Mode)
	{
	case EVoxelMode::Add:
		Material->SetVectorParameterValue("Color", FColor::Green);
		break;
	case EVoxelMode::Remove:
		Material->SetVectorParameterValue("Color", FColor::Red);
		break;
	case EVoxelMode::Flatten:
		break;
	case EVoxelMode::Paint:
		Material->SetVectorParameterValue("Color", FColor::White);
		break;
	default:
		check(false);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////

AVoxelBoxMarker::AVoxelBoxMarker()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);

	StaticMesh->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), this, TEXT("/Voxel/Markers/Shape_Cube.Shape_Cube"))));
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialParentFinder(TEXT("Material'/Voxel/Markers/M_VoxelMarker.M_VoxelMarker'"));
 
	if (MaterialParentFinder.Object)
	{
		MaterialParent = MaterialParentFinder.Object;
	}
}

void AVoxelBoxMarker::UpdateBoxMarker(AVoxelWorld* World, EVoxelMode Mode, FIntBox Bounds)
{
	CHECK_WORLD_VOXELMARKERS(UpdateBoxMarker);

	if (!Material && MaterialParent)
	{
		Material = UMaterialInstanceDynamic::Create(MaterialParent, this);
		StaticMesh->SetMaterial(0, Material);
	}

	FIntVector Size = Bounds.Size();
	FIntVector Position = Bounds.Min;

	FVector Scale = (FVector)(Size - FIntVector(1, 1, 1)) * World->GetVoxelSize() / 100;

	FVector WorldPosition = World->LocalToGlobal(Position) + (FVector)Size * World->GetVoxelSize() / 2 - FVector(0, 0, 50) * Scale - FVector::OneVector * 30.75;

	SetActorLocation(WorldPosition);
	SetActorScale3D(FVector::OneVector * Scale);

	switch (Mode)
	{
	case EVoxelMode::Add:
		Material->SetVectorParameterValue("Color", FColor::Green);
		break;
	case EVoxelMode::Remove:
		Material->SetVectorParameterValue("Color", FColor::Red);
		break;
	case EVoxelMode::Flatten:
		break;
	case EVoxelMode::Paint:
		Material->SetVectorParameterValue("Color", FColor::White);
		break;
	default:
		check(false);
		break;
	}
}
