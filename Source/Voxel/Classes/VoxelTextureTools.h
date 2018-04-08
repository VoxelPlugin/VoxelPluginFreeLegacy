// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTextureTools.generated.h"

class UTexture2D;

UCLASS()
class VOXEL_API UVoxelTextureTools : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static class UTexture2DArray* CreateTextureArrayFromTextures(const TArray<UTexture2D*>& Textures);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void GetTexturesInFolder(const FString& Path, TArray<FString>& Paths, TArray<FString>& Names);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void LoadTexturesFromFolder(const FString& Path,
									   const FString& DiffuseSuffix,
									   const FString& NormalSuffix,
									   TArray<UTexture2D*>& Diffuses,
									   TArray<UTexture2D*>& Normals);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static UTexture2D* LoadTexture(const FString& Path);
};