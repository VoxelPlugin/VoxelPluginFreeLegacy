// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelMeshImporter.generated.h"

class UTexture;
class UTextureRenderTarget2D;
class UVoxelDataAsset;
class UStaticMesh;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;
struct FVoxelDataAssetData;

struct FVoxelMeshImporterInputData
{
	TArray<FVector> Vertices;
	TArray<FIntVector> Triangles;
	TArray<FVector2D> UVs;
};

// We don't want to copy the arrays in the BP, so use an object for that
UCLASS(BlueprintType)
class VOXEL_API UVoxelMeshImporterInputData : public UObject
{
	GENERATED_BODY()

public:
	FVoxelMeshImporterInputData Data;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMeshImporterRenderTargetCache
{
	GENERATED_BODY()

	UPROPERTY(Category = "Cache", BlueprintReadOnly, VisibleAnywhere, Transient)
	UTextureRenderTarget2D* ColorsRenderTarget = nullptr;

	UPROPERTY(Category = "Cache", BlueprintReadOnly, VisibleAnywhere, Transient)
	UTextureRenderTarget2D* UVsRenderTarget = nullptr;
	
	UPROPERTY(Category = "Cache", BlueprintReadOnly, VisibleAnywhere, Transient)
	UMaterialInterface* LastRenderedColorsMaterial = nullptr;
	
	UPROPERTY(Category = "Cache", BlueprintReadOnly, VisibleAnywhere, Transient)
	UMaterialInterface* LastRenderedUVsMaterial = nullptr;
	
	UPROPERTY(Category = "Cache", BlueprintReadOnly, VisibleAnywhere, Transient)
	int32 LastRenderedRenderTargetSize = 0;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMeshImporterSettingsBase
{
	GENERATED_BODY()

	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0"))
	float VoxelSize = 100;

	// Sweep direction to determine the voxel signs. If you have a plane, use Z
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere)
	EVoxelAxis SweepDirection = EVoxelAxis::X;

	// Will do the sweep the other way around: eg, if SweepDirection = Z, the sweep will be done top to bottom if true
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere)
	bool bReverseSweep = true;

	// If true, will assume every line of voxels starts outside the mesh, then goes inside, then goes outside it
	// Set to false if you have a shell and not a true volume
	// For example:
	// - sphere: set to true
	// - half sphere with no bottom geometry: set to false
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere)
	bool bWatertight = true;
	
	// If true, will hide leaks by having holes instead
	// If false, leaks will be long tubes going through the entire asset
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, AdvancedDisplay)
	bool bHideLeaks = true;

	// Distance will be exact for voxels under this distance from a triangle
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, AdvancedDisplay, meta = (ClampMin = "1"))
	int32 ExactBand = 1;

	// Increase this if the shadows/normals quality is bad. Might require to increase MaxVoxelDistanceFromTriangle
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, AdvancedDisplay)
	float DistanceDivisor = 1;

public:
	friend bool operator==(const FVoxelMeshImporterSettingsBase& Lhs, const FVoxelMeshImporterSettingsBase& RHS)
	{
		return Lhs.VoxelSize == RHS.VoxelSize
			&& Lhs.SweepDirection == RHS.SweepDirection
			&& Lhs.bWatertight == RHS.bWatertight
			&& Lhs.bReverseSweep == RHS.bReverseSweep
			&& Lhs.bHideLeaks == RHS.bHideLeaks
			&& Lhs.ExactBand == RHS.ExactBand
			&& Lhs.DistanceDivisor == RHS.DistanceDivisor;
	}
	friend bool operator!=(const FVoxelMeshImporterSettingsBase& Lhs, const FVoxelMeshImporterSettingsBase& RHS)
	{
		return !(Lhs == RHS);
	}
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMeshImporterSettings : public FVoxelMeshImporterSettingsBase
{
	GENERATED_BODY()

	FVoxelMeshImporterSettings();
	explicit FVoxelMeshImporterSettings(const FVoxelMeshImporterSettingsBase& Base);
	
	// Will sample ColorsMaterial at the mesh UVs to get the voxel colors
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere)
	bool bImportColors = true;
	
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bPaintColors"))
	UMaterialInterface* ColorsMaterial = nullptr;

	// Will sample UVChannelsMaterial at the mesh UVs to get the voxel UVs
	// RG will go in first UV channel, BA in second
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere)
	bool bImportUVs = true;
	
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, meta = (EditCondition = bPaintUVs))
	UMaterialInterface* UVsMaterial = nullptr;

	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, AdvancedDisplay)
	int32 RenderTargetSize = 4096;

public:
	friend bool operator==(const FVoxelMeshImporterSettings& Lhs, const FVoxelMeshImporterSettings& RHS)
	{
		return static_cast<const FVoxelMeshImporterSettingsBase&>(Lhs) == static_cast<const FVoxelMeshImporterSettingsBase&>(RHS)
			&& Lhs.bImportColors == RHS.bImportColors
			&& Lhs.ColorsMaterial == RHS.ColorsMaterial
			&& Lhs.bImportUVs == RHS.bImportUVs
			&& Lhs.UVsMaterial == RHS.UVsMaterial
			&& Lhs.RenderTargetSize == RHS.RenderTargetSize;
	}
	friend bool operator!=(const FVoxelMeshImporterSettings& Lhs, const FVoxelMeshImporterSettings& RHS)
	{
		return !(Lhs == RHS);
	}
};

UCLASS()
class VOXEL_API UVoxelMeshImporterLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void CreateMeshDataFromStaticMesh(UStaticMesh* StaticMesh, FVoxelMeshImporterInputData& Data);
	
	static bool ConvertMeshToVoxels(
		UObject* WorldContextObject,
		const FVoxelMeshImporterInputData& Mesh,
		const FTransform& Transform,
		const FVoxelMeshImporterSettings& Settings,
		FVoxelMeshImporterRenderTargetCache& RenderTargetCache,
		FVoxelDataAssetData& OutAsset,
		FIntVector& OutOffset,
		int32& OutNumLeaks);

	static void ConvertMeshToDistanceField(
		const FVoxelMeshImporterInputData& Mesh,
		const FTransform& Transform,
		const FVoxelMeshImporterSettingsBase& Settings,
		// Needed if we want a smooth import, in voxels
		float BoxExtension,
		TArray<float>& OutDistanceField,
		TArray<FVector>& OutSurfacePositions,
		FIntVector& OutSize,
		FIntVector& OutOffset,
		int32& OutNumLeaks,
		EVoxelComputeDevice Device = EVoxelComputeDevice::GPU,
		bool bMultiThreaded = true,
		int32 MaxPasses_Debug = -1);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Mesh Importer")
	static UVoxelMeshImporterInputData* CreateMeshDataFromStaticMesh(UStaticMesh* StaticMesh);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Mesh Importer", meta = (WorldContext = "WorldContextObject"))
	static UTextureRenderTarget2D* CreateTextureFromMaterial(
		UObject* WorldContextObject,
		UMaterialInterface* Material, 
		int32 Width = 1024, 
		int32 Height = 1024);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Mesh Importer", meta = (WorldContext = "WorldContextObject"))
	static void ConvertMeshToVoxels(
		UObject* WorldContextObject,
		UVoxelMeshImporterInputData* Mesh,
		FTransform Transform,
		bool bSubtractive,
		FVoxelMeshImporterSettings Settings,
		UPARAM(ref) FVoxelMeshImporterRenderTargetCache& RenderTargetCache,
		UVoxelDataAsset*& Asset,
		int32& NumLeaks);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Mesh Importer", meta = (WorldContext = "WorldContextObject"))
	static void ConvertMeshToVoxels_NoMaterials(
		UObject* WorldContextObject,
		UVoxelMeshImporterInputData* Mesh,
		FTransform Transform,
		bool bSubtractive,
		FVoxelMeshImporterSettingsBase Settings,
		UVoxelDataAsset*& Asset,
		int32& NumLeaks);
};

/**
 * Actor that creates a VoxelDataAsset from a static mesh
 */
UCLASS(NotBlueprintType, NotBlueprintable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HOLD", "LOD", "Cooking"))
class VOXEL_API AVoxelMeshImporter : public AActor
{
	GENERATED_BODY()

public:
	// The static mesh to import from
	UPROPERTY(EditAnywhere, Category = "Import Configuration")
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, Category = "Import Configuration", meta = (ShowOnlyInnerProperties))
	FVoxelMeshImporterSettings Settings;

	UPROPERTY(VisibleAnywhere, Category = "Expected Size")
	uint32 SizeX;

	UPROPERTY(VisibleAnywhere, Category = "Expected Size")
	uint32 SizeY;
	
	UPROPERTY(VisibleAnywhere, Category = "Expected Size")
	uint32 SizeZ;
	
	UPROPERTY(VisibleAnywhere, Category = "Expected Size")
	uint64 NumberOfVoxels;

	UPROPERTY(VisibleAnywhere, Category = "Expected Size")
	float SizeInMB;

	AVoxelMeshImporter();

protected:
	virtual void Tick(float DeltaSeconds) override;
#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
#endif

private:
	UPROPERTY()
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* MaterialInstance;

	UPROPERTY(Transient)
	FBox CachedBox;

	UPROPERTY(Transient)
	UStaticMesh* CachedStaticMesh;

	UPROPERTY(Transient)
	TArray<FVector> CachedVertices;
	
	UPROPERTY(Transient)
	FTransform CachedTransform;

	void InitMaterialInstance();
	void UpdateSizes();
};