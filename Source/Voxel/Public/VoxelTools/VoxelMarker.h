// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IntBox.h"
#include "VoxelMarker.generated.h"

class USpotLightComponent;
class UDecalComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EVoxelMode : uint8
{
	Add,
	Remove,
	Flatten,
	Paint
};

UENUM(BlueprintType)
enum class EVoxelTool : uint8
{
	Projection,
	Sphere,
	Box
};

UCLASS(notplaceable)
class VOXEL_API AVoxelMarker : public AActor
{
	GENERATED_BODY()

public:
	AVoxelMarker();

	void Tick(float DeltaTime) override;
	void SetAsEditorMarker();
};

UCLASS(Blueprintable)
class VOXEL_API AVoxelProjectionMarker : public AVoxelMarker
{
    GENERATED_BODY()

public:
	AVoxelProjectionMarker();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateProjectionMarker(AVoxelWorld* World, EVoxelMode Mode, FVector Position, FVector Normal, float Radius, float EditDistance, float HeightOffset);

	UPROPERTY(BlueprintReadWrite, Category = "Voxel")
	UDecalComponent* Decal;
	UPROPERTY(BlueprintReadWrite, Category = "Voxel")
	UMaterialInstanceDynamic* MaterialInstance;
};

UCLASS(Blueprintable)
class VOXEL_API AVoxelSphereMarker : public AVoxelMarker
{
    GENERATED_BODY()

public:
	AVoxelSphereMarker();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateSphereMarker(AVoxelWorld* World, EVoxelMode Mode, FIntVector Position, float Radius);

	UPROPERTY(BlueprintReadWrite, Category = "Voxel")
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(BlueprintReadWrite, Category = "Voxel")
	UMaterialInterface* MaterialParent;

	UPROPERTY(BlueprintReadWrite, Category = "Voxel")
	UMaterialInstanceDynamic* Material;
};

UCLASS(Blueprintable)
class VOXEL_API AVoxelBoxMarker : public AVoxelMarker
{
    GENERATED_BODY()

public:
	AVoxelBoxMarker();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateBoxMarker(AVoxelWorld* World, EVoxelMode Mode, FIntBox Bounds);

	UPROPERTY(BlueprintReadWrite, Category = "Voxel")
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(BlueprintReadWrite, Category = "Voxel")
	UMaterialInterface* MaterialParent;

	UPROPERTY(BlueprintReadWrite, Category = "Voxel")
	UMaterialInstanceDynamic* Material;
};