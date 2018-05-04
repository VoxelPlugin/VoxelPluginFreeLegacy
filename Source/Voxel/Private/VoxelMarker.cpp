// Copyright 2017 Phyronnaz

#include "VoxelMarker.h"
#include "ConstructorHelpers.h"
#include "VoxelWorld.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Components/SpotLightComponent.h"


AVoxelEditorMarker::AVoxelEditorMarker()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AVoxelEditorMarker::Tick(float DeltaTime)
{
	if (GetWorld()->WorldType != EWorldType::Editor && GetWorld()->WorldType != EWorldType::EditorPreview)
	{
		Destroy();
	}
}

///////////////////////////////////////////////////////////////////////////////

AVoxelBoxMarker::AVoxelBoxMarker()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));

	StaticMesh->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), this, TEXT("/Voxel/Markers/Shape_Cube.Shape_Cube"))));
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialParentFinder(TEXT("Material'/Voxel/Markers/M_VoxelMarker.M_VoxelMarker'"));
 
	if (MaterialParentFinder.Object)
	{
		MaterialParent = MaterialParentFinder.Object;
	}
}

void AVoxelBoxMarker::SetParametersCpp(AVoxelWorld* World, EVoxelMode Mode, FVector Position, FVector Normal, float SpotlightHeight, float ToolHeight, float Radius, bool bAdd)
{
	if (!World)
	{
		return;
	}

	if (!Material && MaterialParent)
	{
		Material = UMaterialInstanceDynamic::Create(MaterialParent, this);
		StaticMesh->SetMaterial(0, Material);
	}

	const float Scale = (FMath::CeilToInt(Radius / World->GetVoxelSize()) - 1) * World->GetVoxelSize() / 100;

	const FVector WorldPosition = World->LocalToGlobal(World->GlobalToLocal(Position - Radius / 2)) + Radius / 2 - FVector(0, 0, 50) * Scale - FVector::OneVector * 30.75;

	SetActorLocation(WorldPosition);
	SetActorScale3D(FVector::OneVector * Scale);

	switch (Mode)
	{
	case EVoxelMode::Edit:
		if (bAdd)
		{
			Material->SetVectorParameterValue("Color", FColor::Green);
		}
		else
		{
			Material->SetVectorParameterValue("Color", FColor::Red);
		}
		break;
	case EVoxelMode::Flatten:
		break;
	case EVoxelMode::Material:
		Material->SetVectorParameterValue("Color", FColor::White);
		break;
	default:
		check(false);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////

AVoxelSphereMarker::AVoxelSphereMarker()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));

	StaticMesh->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), this, TEXT("/Engine/BasicShapes/Sphere.Sphere"))));
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialParentFinder(TEXT("Material'/Voxel/Markers/M_VoxelMarker.M_VoxelMarker'"));
 
	if (MaterialParentFinder.Object)
	{
		MaterialParent = MaterialParentFinder.Object;
	}
}

void AVoxelSphereMarker::SetParametersCpp(AVoxelWorld* World, EVoxelMode Mode, FVector Position, FVector Normal, float SpotlightHeight, float ToolHeight, float Radius, bool bAdd)
{
	if (!Material && MaterialParent)
	{
		Material = UMaterialInstanceDynamic::Create(MaterialParent, this);
		StaticMesh->SetMaterial(0, Material);
	}

	SetActorLocation(Position);
	SetActorScale3D(FVector::OneVector * Radius / 50);

	switch (Mode)
	{
	case EVoxelMode::Edit:
		if (bAdd)
		{
			Material->SetVectorParameterValue("Color", FColor::Green);
		}
		else
		{
			Material->SetVectorParameterValue("Color", FColor::Red);
		}
		break;
	case EVoxelMode::Flatten:
		break;
	case EVoxelMode::Material:
		Material->SetVectorParameterValue("Color", FColor::White);
		break;
	default:
		check(false);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////

AVoxelProjectionMarker::AVoxelProjectionMarker()
{
	Spotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spotlight"));
	Spotlight->SetIntensity(100);
	Spotlight->bUseInverseSquaredFalloff = false;
	Spotlight->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder(TEXT("Material'/Voxel/Markers/Cookie.Cookie'"));
 
	if (MaterialFinder.Object)
	{
		Spotlight->SetLightFunctionMaterial(MaterialFinder.Object);
	}
}

void AVoxelProjectionMarker::SetParametersCpp(AVoxelWorld* World, EVoxelMode Mode, FVector Position, FVector Normal, float SpotlightHeight, float ToolHeight, float Radius, bool bAdd)
{
	SetActorLocation(Position + Normal * SpotlightHeight);
	SetActorRotation(FRotationMatrix::MakeFromX(-Normal).Rotator());

	switch (Mode)
	{
	case EVoxelMode::Edit:
		if (bAdd)
		{
			Spotlight->SetLightColor(FColor::Green);
		}
		else
		{
			Spotlight->SetLightColor(FColor::Red);
		}
		break;
	case EVoxelMode::Flatten:
		Spotlight->SetLightColor(FColor::Blue);
		break;
	case EVoxelMode::Material:
		Spotlight->SetLightColor(FColor::White);
		break;
	default:
		check(false);
		break;
	}

	Spotlight->SetOuterConeAngle(FMath::RadiansToDegrees(FMath::Atan2(2 * Radius, SpotlightHeight)));
	Spotlight->SetAttenuationRadius(2 * (SpotlightHeight + ToolHeight));
}