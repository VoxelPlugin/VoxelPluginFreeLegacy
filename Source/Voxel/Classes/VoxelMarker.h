// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelMarker.generated.h"

class USpotLightComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EVoxelMode : uint8
{
	Edit,
	Flatten,
	Material
};

UENUM(BlueprintType)
enum class EVoxelTool : uint8
{
	Projection,
	Sphere,
	Box
};

UCLASS(Blueprintable)
class VOXEL_API AVoxelMarker : public AActor
{
    GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetParameters(AVoxelWorld* World, EVoxelMode Mode, FVector Position, FVector Normal, float SpotlightHeight, float ToolHeight, float Radius, bool bAdd);

};

UCLASS(Blueprintable)
class VOXEL_API AVoxelEditorMarker : public AVoxelMarker
{
	GENERATED_BODY()
public:
	AVoxelEditorMarker();

	void Tick(float DeltaTime) override;
};

UCLASS(Blueprintable)
class VOXEL_API AVoxelBoxMarker : public AVoxelEditorMarker
{
    GENERATED_BODY()
public:
	AVoxelBoxMarker();

	UFUNCTION(BlueprintCallable)
	void SetParametersCpp(AVoxelWorld* World, EVoxelMode Mode, FVector Position, FVector Normal, float SpotlightHeight, float ToolHeight, float Radius, bool bAdd);

	UPROPERTY(BlueprintReadWrite)
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(BlueprintReadWrite)
	UMaterialInterface* MaterialParent;

	UPROPERTY(BlueprintReadWrite)
	UMaterialInstanceDynamic* Material;
};

UCLASS(Blueprintable)
class VOXEL_API AVoxelSphereMarker : public AVoxelEditorMarker
{
    GENERATED_BODY()
public:
	AVoxelSphereMarker();

	UFUNCTION(BlueprintCallable)
	void SetParametersCpp(AVoxelWorld* World, EVoxelMode Mode, FVector Position, FVector Normal, float SpotlightHeight, float ToolHeight, float Radius, bool bAdd);

	UPROPERTY(BlueprintReadWrite)
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(BlueprintReadWrite)
	UMaterialInterface* MaterialParent;

	UPROPERTY(BlueprintReadWrite)
	UMaterialInstanceDynamic* Material;
};

UCLASS(Blueprintable)
class VOXEL_API AVoxelProjectionMarker : public AVoxelEditorMarker
{
    GENERATED_BODY()
public:
	AVoxelProjectionMarker();

	UFUNCTION(BlueprintCallable)
	void SetParametersCpp(AVoxelWorld* World, EVoxelMode Mode, FVector Position, FVector Normal, float SpotlightHeight, float ToolHeight, float Radius, bool bAdd);

	UPROPERTY(BlueprintReadWrite)
	USpotLightComponent* Spotlight;
};