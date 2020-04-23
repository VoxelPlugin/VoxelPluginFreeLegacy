// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.h"
#include "VoxelIntVectorUtilities.h"
#include "IntBox.h"
#include "VoxelGraphPreviewSettings.generated.h"

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
	Material
};

UCLASS()
class VOXELGRAPH_API UVoxelGraphPreviewSettings : public UObject
	{
	GENERATED_BODY()

public:
	UVoxelGraphPreviewSettings();

public:
	UPROPERTY(EditAnywhere, Category = "Preview Zone")
	EVoxelGraphPreviewAxes LeftToRight = EVoxelGraphPreviewAxes::X;

	UPROPERTY(EditAnywhere, Category = "Preview Zone")
	EVoxelGraphPreviewAxes BottomToTop = EVoxelGraphPreviewAxes::Y;

	UPROPERTY(EditAnywhere, Category = "Preview Zone")
	FIntVector Center = FIntVector(0, 0, 0);
	
	UPROPERTY(EditAnywhere, Category = "Preview Zone")
	int32 Resolution = 512;

	UPROPERTY(EditAnywhere, Category = "Preview Zone", meta = (ClampMin = 0, ClampMax = 20, UIMin = 0, UIMax = 20))
	int32 ResolutionScale = 0;

	UPROPERTY(VisibleAnywhere, Category = "Preview Zone")
	FIntBox PreviewedBounds;

public:
	// Black
	UPROPERTY(EditAnywhere, Category = "Preview Range")
	float MinValue = -1;

	// White
	UPROPERTY(EditAnywhere, Category = "Preview Range")
	float MaxValue = 1;

public:
	UPROPERTY(EditAnywhere, Category = "2D Preview Color")
	EVoxelGraphPreviewType PreviewType2D = EVoxelGraphPreviewType::Density;

	UPROPERTY(EditAnywhere, Category = "2D Preview Color")
	EVoxelMaterialConfig MaterialConfig = EVoxelMaterialConfig::RGB;

	UPROPERTY(EditAnywhere, Category = "2D Preview Color")
	TArray<FLinearColor> IndexColors = { FLinearColor::Green, FLinearColor::Red, FLinearColor::Blue, FLinearColor::White };

public:
	UPROPERTY(EditAnywhere, Category = "3D Preview Color")
	bool bHeightBasedColor = true;

	UPROPERTY(EditAnywhere, Category = "3D Preview Color", meta = (EditCondition = bHeightBasedColor))
	bool bEnableWater = false;

public:
	UPROPERTY(EditAnywhere, Category = "3D Preview Settings", meta = (NoRebuild))
	float Height = 200;

	UPROPERTY(EditAnywhere, Category = "3D Preview Settings", meta = (NoRebuild))
	FVector LightDirection = FVector(1, 1, 1);

	UPROPERTY(EditAnywhere, Category = "3D Preview Settings|Raytraced Shadows", meta = (NoRebuild))
	float StartBias = 0.01;

	UPROPERTY(EditAnywhere, Category = "3D Preview Settings|Raytraced Shadows", meta = (NoRebuild))
	int32 MaxSteps = 128;

	UPROPERTY(EditAnywhere, Category = "3D Preview Settings|Raytraced Shadows", meta = (NoRebuild, UIMin = 0, UIMax = 1))
	float Brightness = 1;

	UPROPERTY(EditAnywhere, Category = "3D Preview Settings|Raytraced Shadows", meta = (NoRebuild, UIMin = 0))
	float ShadowDensity = 8;

	UPROPERTY(EditAnywhere, Category = "3D Preview Settings|Mesh", meta = (NoRebuild))
	class UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "3D Preview Settings|Mesh", meta = (NoRebuild))
	class UMaterialInterface* Material = nullptr;

	UPROPERTY(EditAnywhere, Category = "3D Preview Settings|Mesh", meta = (NoRebuild))
	FVector MeshScale = FVector(10, 10, 10);

public:
	// Simulate querying a chunk at a specific LOD, eg to check fractal noise settings
	UPROPERTY(EditAnywhere, Category = "Misc", meta = (ClampMin = 0, ClampMax = 26, UIMin = 0, UIMax = 26, DisplayName = "LOD to preview"))
	int32 LODToPreview = 0;
	
public:
	UPROPERTY()
	class UVoxelGraphGenerator* Graph;

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	inline int32 GetStep() const
	{
		return 1 << ResolutionScale;
	}

	inline FIntVector GetCenter() const
	{
		const int32 Step = GetStep();
		return FVoxelUtilities::RoundToInt(FVector(Center) / Step) * Step;
	}

	inline FIntVector GetStart() const
	{
		FIntVector Start = GetCenter();

		const int32 Offset = Resolution / 2 * GetStep();
		for (EVoxelGraphPreviewAxes Axis : { LeftToRight, BottomToTop })
		{
			GetAxis(Start, Axis) -= Offset;
		}

		return Start;
	}

	inline FIntVector GetSize() const
	{
		FIntVector Size = FIntVector(1, 1, 1);
		for (EVoxelGraphPreviewAxes Axis : { LeftToRight, BottomToTop })
		{
			GetAxis(Size, Axis) = Resolution;
		}
		return Size;
	}

	inline FIntBox GetBounds() const
	{
		const FIntVector Start = GetStart();
		return FIntBox(Start, Start + GetSize() * GetStep());
	}

	inline FIntVector GetPosition(int32 X, int32 Y) const
	{
		FIntVector Position(0, 0, 0);
		GetAxis(Position, LeftToRight) = X;
		GetAxis(Position, BottomToTop) = Resolution - 1 - Y;
		return Position;
	}

private:
	static inline int32& GetAxis(FIntVector& Vector, EVoxelGraphPreviewAxes Axis)
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
};