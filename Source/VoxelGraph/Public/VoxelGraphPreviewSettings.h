// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelIntBox.h"
#include "VoxelUtilities/VoxelIntVectorUtilities.h"
#include "VoxelGraphPreviewSettings.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UVoxelPlaceableItemManager;
class UVoxelMaterialCollectionBase;

UENUM()
enum class EVoxelGraphPreviewAxes : uint8
{
	X,
	Y,
	Z
};

UENUM()
enum class EVoxelGraphPreviewType : uint8
{
	Density,
	Material,
	Cost,
	RangeAnalysis
};


UENUM()
enum class EVoxelGraphPreviewShowValue : uint8
{
	ShowValue,
	ShowRange,
	ShowValueAndRange
};

UENUM()
enum class EVoxelGraphMaterialPreviewType : uint8
{
	// Show the material RGB values
	RGB,
	// Show the material Alpha value
	Alpha,
	// Assign one color per index
	SingleIndex,
	// Blends the indices colors
	MultiIndex_Overview,
	// Only shows the strength of a single index (set by MultiIndexToPreview)
	MultiIndex_SingleIndexPreview,
	// Wetness
	MultiIndex_Wetness,
	// Red-Green preview of UV0
	UV0,
	// Red-Green preview of UV1
	UV1,
	// Red-Green preview of UV2
	UV2,
	// Red-Green preview of UV3
	UV3
};

UCLASS(Within=VoxelGraphGenerator)
class VOXELGRAPH_API UVoxelGraphPreviewSettings : public UObject
{
	GENERATED_BODY()
	INTELLISENSE_DECLARE_WITHIN(UVoxelGraphGenerator);

public:
	UVoxelGraphPreviewSettings();
		
public:
	UPROPERTY()
	bool bShowStats = false;
	
	UPROPERTY()
	bool bShowValues = false;

public:
	// Min displayed value
	UPROPERTY(VisibleAnywhere, Category = "Preview Info")
	mutable FString MinValue;

	// Max displayed value
	UPROPERTY(VisibleAnywhere, Category = "Preview Info")
	mutable FString MaxValue;

	UPROPERTY(VisibleAnywhere, Category = "Preview Info", AdvancedDisplay)
	FVoxelIntBox PreviewedBounds;

public:
	UPROPERTY(EditAnywhere, Category = "Preview Zone")
	EVoxelGraphPreviewAxes LeftToRight = EVoxelGraphPreviewAxes::X;

	UPROPERTY(EditAnywhere, Category = "Preview Zone")
	EVoxelGraphPreviewAxes BottomToTop = EVoxelGraphPreviewAxes::Y;

	UPROPERTY(EditAnywhere, Category = "Preview Zone", meta = (ClampMin = 32, ClampMax = 8192, UIMin = 100, UIMax = 1000))
	int32 Resolution = 512;

	UPROPERTY(EditAnywhere, Category = "Preview Zone", meta = (UIMin = 0, UIMax = 20))
	int32 ResolutionMultiplierLog = 0;

	// Right click & pan the preview to change it
	UPROPERTY(EditAnywhere, Category = "Preview Zone")
	FIntVector Center = FIntVector(0, 0, 0);

	// Left click the preview to set it
	UPROPERTY(EditAnywhere, Category = "Preview Zone")
	FIntVector PreviewedVoxel = FIntVector(0, 0, 0);
	
	UPROPERTY(EditAnywhere, Category = "Preview Zone", AdvancedDisplay)
	EVoxelGraphPreviewShowValue ShowValue = EVoxelGraphPreviewShowValue::ShowValue;
		
public:
	// Set this to the material config your voxel world will use
	UPROPERTY(EditAnywhere, Category = "Voxel World Settings")
	EVoxelMaterialConfig MaterialConfig = EVoxelMaterialConfig::RGB;

	// Use to preview Get Index from Material Collection
	UPROPERTY(EditAnywhere, Category = "Voxel World Settings")
	UVoxelMaterialCollectionBase* MaterialCollection = nullptr;

	// Used to preview placeable items
	UPROPERTY(EditAnywhere, Category = "Voxel World Settings", Instanced, meta = (Automatic, UpdateItems))
	UVoxelPlaceableItemManager* PlaceableItemManager = nullptr;

	// Value returned by the Voxel Size node
	UPROPERTY(EditAnywhere, Category = "Voxel World Settings", AdvancedDisplay)
	float VoxelSize = 100;

	UPROPERTY(EditAnywhere, Category = "Voxel World Settings", AdvancedDisplay)
	EVoxelRenderType RenderType = EVoxelRenderType::MarchingCubes;

public:
	UPROPERTY(EditAnywhere, Category = "2D Preview Color")
	EVoxelGraphPreviewType PreviewType2D = EVoxelGraphPreviewType::Density;

	// If true, will color the distance field orange when positive, blue when negative, and will apply a cosine to make progression easier to see
	// This coloring is directly derived from Inigo Quilez's work
	UPROPERTY(EditAnywhere, Category = "2D Preview Color")
	bool bDrawColoredDistanceField = true;

	UPROPERTY(EditAnywhere, Category = "2D Preview Color")
	EVoxelGraphMaterialPreviewType MaterialPreviewType = EVoxelGraphMaterialPreviewType::RGB;
		
	// Used if material preview type is MultiIndex_SingleIndexPreview
	UPROPERTY(EditAnywhere, Category = "2D Preview Color")
	int32 MultiIndexToPreview = 0;
		
	UPROPERTY(EditAnywhere, Category = "2D Preview Color")
	TArray<FColor> IndexColors;

	// If true, areas where the density is > 0 will be shown as black
	UPROPERTY(EditAnywhere, Category = "2D Preview Color", AdvancedDisplay)
	bool bHybridMaterialRendering = true;

	// Increase this if there's too much noise in the cost view
	UPROPERTY(EditAnywhere, Category = "2D Preview Color", AdvancedDisplay, meta = (UIMin = 0, UIMax = 1))
	float CostPercentile = 0.05f;
	
	UPROPERTY(EditAnywhere, Category = "2D Preview Color", AdvancedDisplay, meta = (UIMin = 1, UIMax = 1024))
	int32 NumRangeAnalysisChunksPerAxis = 64;

public:
	UPROPERTY(EditAnywhere, Category = "3D Preview Settings")
	bool bHeightmapMode = true;
	
	UPROPERTY(EditAnywhere, Category = "3D Preview Heightmap Settings")
	bool bHeightBasedColor = true;

	UPROPERTY(EditAnywhere, Category = "3D Preview Heightmap Settings", meta = (MeshOnly, EditCondition = bHeightBasedColor))
	bool bEnableWater = false;
	
	UPROPERTY(EditAnywhere, Category = "3D Preview Heightmap Settings", meta = (MeshOnly))
	float Height = 200;

	UPROPERTY(EditAnywhere, Category = "3D Preview Heightmap Settings", meta = (MeshOnly))
	FVector LightDirection = FVector(1, 1, 1);

	UPROPERTY(EditAnywhere, Category = "3D Preview Heightmap Settings|Raytraced Shadows", meta = (MeshOnly))
	float StartBias = 0.01;

	UPROPERTY(EditAnywhere, Category = "3D Preview Heightmap Settings|Raytraced Shadows", meta = (MeshOnly))
	int32 MaxSteps = 128;

	UPROPERTY(EditAnywhere, Category = "3D Preview Heightmap Settings|Raytraced Shadows", meta = (MeshOnly, UIMin = 0, UIMax = 1))
	float Brightness = 1;

	UPROPERTY(EditAnywhere, Category = "3D Preview Heightmap Settings|Raytraced Shadows", meta = (MeshOnly, UIMin = 0))
	float ShadowDensity = 8;

public:
	UPROPERTY()
	UStaticMesh* Mesh = nullptr;

	UPROPERTY()
	UMaterialInterface* HeightmapMaterial = nullptr;

	UPROPERTY()
	UMaterialInterface* SliceMaterial = nullptr;
	
public:
	// Will set black to the lowest value in the image, and white to the highest
	UPROPERTY(EditAnywhere, Category = "Misc")
	bool bAutoNormalize = true;
		
	// Black
	UPROPERTY(EditAnywhere, Category = "Misc", AdvancedDisplay, meta = (EditCondition = "!bAutoNormalize"))
	float NormalizeMinValue = -1;

	// White
	UPROPERTY(EditAnywhere, Category = "Misc", AdvancedDisplay, meta = (EditCondition = "!bAutoNormalize"))
	float NormalizeMaxValue = 1;
		
	// Simulate querying a chunk at a specific LOD, eg to check fractal noise settings
	UPROPERTY(EditAnywhere, Category = "Misc", meta = (ClampMin = 0, ClampMax = 26, UIMin = 0, UIMax = 26, DisplayName = "LOD to preview"))
	int32 LODToPreview = 0;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

struct VOXELGRAPH_API FVoxelGraphPreviewSettingsWrapper
{
public:
	int32 LOD; // != LOD to preview!
	int32 Step;
	int32 Resolution;

	FIntVector Start;
	FIntVector Size;
	FIntVector Center;
	
	FVoxelIntBox Bounds;

	EVoxelGraphPreviewAxes LeftToRight = EVoxelGraphPreviewAxes::X;
	EVoxelGraphPreviewAxes BottomToTop = EVoxelGraphPreviewAxes::Y;

	explicit FVoxelGraphPreviewSettingsWrapper(const UVoxelGraphPreviewSettings& Settings);

public:
	template<typename T>
	static auto& GetAxis(T& Vector, EVoxelGraphPreviewAxes Axis)
	{
		switch (Axis)
		{
		case EVoxelGraphPreviewAxes::X:
			return Vector.X;
		case EVoxelGraphPreviewAxes::Y:
			return Vector.Y;
		case EVoxelGraphPreviewAxes::Z:
		default:
			return Vector.Z;
		}
	}

	FIntVector GetRelativePosition(int32 X, int32 Y) const
	{
		FIntVector Position(0, 0, 0);
		GetAxis(Position, LeftToRight) = X;
		GetAxis(Position, BottomToTop) = Y;
		return Position;
	}
	FVector GetRelativePosition(float X, float Y) const
	{
		FVector Position(0, 0, 0);
		GetAxis(Position, LeftToRight) = X;
		GetAxis(Position, BottomToTop) = Y;
		return Position;
	}
	
	FIntVector GetWorldPosition(int32 X, int32 Y) const
	{
		return Start + Step * GetRelativePosition(X, Y);
	}
	
	FIntPoint GetScreenPosition(FIntVector WorldPosition) const
	{
		WorldPosition -= Start;
		WorldPosition = FVoxelUtilities::DivideRound(WorldPosition, Step);

		FIntPoint Result;
		Result.X = GetAxis(WorldPosition, LeftToRight);
		Result.Y = GetAxis(WorldPosition, BottomToTop);
		return Result;
	}
	FVector2D GetScreenPosition(FVector WorldPosition) const
	{
		WorldPosition -= FVector(Start);
		WorldPosition /= Step;

		FVector2D Result;
		Result.X = GetAxis(WorldPosition, LeftToRight);
		Result.Y = GetAxis(WorldPosition, BottomToTop);
		return Result;
	}
	
	int32 GetDataIndex(int32 X, int32 Y) const
	{
		const FIntVector Position = GetRelativePosition(X, Y);
		return Position.X + Position.Y * Size.X + Position.Z * Size.X * Size.Y;
	}
	int32 GetTextureIndex(int32 X, int32 Y) const
	{
		return X + Resolution * (Resolution - 1 - Y);
	}
};