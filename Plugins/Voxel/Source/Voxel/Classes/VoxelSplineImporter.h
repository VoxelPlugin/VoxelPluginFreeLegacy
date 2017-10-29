//// Copyright 2017 Phyronnaz
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "VoxelModifier.h"
//#include "SplineVoxelModifier.generated.h"
//
//class AVoxelWorld;
//class FVoxelData;
//class FVoxelRender;
//class USplineComponent;
//class UVoxelDataAsset;
//
///**
//*
//*/
//UCLASS(Blueprintable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "Hide"))
//class VOXEL_API ASplineVoxelModifier : public AVoxelModifier
//{
//	GENERATED_BODY()
//
//public:
//	ASplineVoxelModifier();
//	~ASplineVoxelModifier();
//
//
//	UPROPERTY(EditAnywhere)
//		UVoxelDataAsset* VoxelDataAsset;
//
//	UPROPERTY(EditAnywhere)
//		TArray<USplineComponent*> Splines;
//
//	UPROPERTY(EditAnywhere)
//		float Size;
//
//	UPROPERTY(EditAnywhere)
//		bool bSubstrative;
//
//	UPROPERTY(EditAnywhere)
//		bool bSetMaterial;
//
//	UPROPERTY(EditAnywhere, meta = (EditCondition = "bSetMaterial"))
//		FVoxelMaterial Material;
//
//	UPROPERTY(EditAnywhere, Category = "Preview")
//		AVoxelWorld* PreviewWorld;
//
//	UPROPERTY(EditAnywhere, Category = "Preview")
//		bool bEnablePreview;
//
//	UPROPERTY(EditAnywhere, Category = "Preview|May crash the editor - please save before using this")
//		bool bLivePreview;
//
//	UPROPERTY(EditAnywhere, Category = "Preview|May crash the editor - please save before using this")
//		float LivePreviewFPS;
//
//	UPROPERTY(EditAnywhere, Category = "Preview|May crash the editor - please save before using this")
//		bool bNoSizeLimit;
//
//	virtual void ApplyToWorld(AVoxelWorld* World) override;
//
//protected:
//	virtual void BeginPlay() override;
//#if WITH_EDITOR
//	void Tick(float DeltaTime) override;
//	bool ShouldTickIfViewportsOnly() const override;
//	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
//#endif
//
//private:
//	FVoxelData* Data;
//	FVoxelRender* Render;
//
//	UPROPERTY()
//		UVoxelWorldGenerator* Generator;
//
//	float TimeSinceUpdate;
//
//	void UpdateRender();
//};
