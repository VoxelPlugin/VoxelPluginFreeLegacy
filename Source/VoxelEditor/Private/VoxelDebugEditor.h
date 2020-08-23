// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelImporters/VoxelMeshImporter.h"
#include "VoxelDebugEditor.generated.h"

class UStaticMesh;
class SDockTab;
class FSpawnTabArgs;

UENUM()
enum class EVoxelDebugType
{
	JumpFlood,
	CustomData
};

UENUM()
enum class EVoxelDebugSliceAxis
{
	X,
	Y,
	Z
};

UCLASS()
class UVoxelDebugParameters_Base : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "General")
	EVoxelDebugType DebugType = EVoxelDebugType::CustomData;
	
	UPROPERTY(EditAnywhere, Category = "General")
	bool bUpdate = false;
};

UCLASS()
class UVoxelDebugParameters_JumpFlood : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Source")
	bool bUseMesh = false;
	
	UPROPERTY(EditAnywhere, Category = "Source|Sphere", meta = (ClampMin = 1, ClampMax = 512, UIMin = 1, UIMax = 512, EditCondition = "!bUseMesh"))
	int32 TextureSize = 32;

	UPROPERTY(EditAnywhere, Category = "Source|Mesh", meta = (EditCondition = "bUseMesh"))
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Source|Mesh", meta = (EditCondition = "bUseMesh"))
	FVoxelMeshImporterSettingsBase MeshImporterSettings;

	UPROPERTY(EditAnywhere, Category = "Source|Mesh", meta = (EditCondition = "bUseMesh"))
	float BoxExtension = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Source|Mesh", meta = (EditCondition = "bUseMesh"))
	FTransform Transform;

	UPROPERTY(EditAnywhere, Category = "Preview")
	EVoxelDebugSliceAxis SliceAxis = EVoxelDebugSliceAxis::Z;
	
	UPROPERTY(EditAnywhere, Category = "Preview")
	int32 Slice = 0;

	UPROPERTY(EditAnywhere, Category = "Preview")
	bool bFlip = false;
	
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = -1))
	int32 Passes = -1;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bShowDistances = false;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = 1))
	int32 Divisor = 1;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bShrink = false;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bUseCPU = false;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bMultiThreaded = false;

	UPROPERTY(VisibleAnywhere, Category = "Info")
	FIntVector Size;

	UPROPERTY(VisibleAnywhere, Category = "Info")
	float TimeInSeconds = 0;
};

UCLASS()
class UVoxelDebugParameters_CustomData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	EVoxelDebugSliceAxis SliceAxis = EVoxelDebugSliceAxis::Z;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	int32 Slice = 0;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bFlip = false;
	
	UPROPERTY(EditAnywhere, Category = "Source", Transient)
	TMap<FName, bool> DataToDisplay;
	
	UPROPERTY(EditAnywhere, Category = "Source", Transient, meta = (UIMin = 0, UIMax = 99))
	int32 Frame = 0;

	UPROPERTY(VisibleAnywhere, Category = "Info")
	FIntVector Size;

public:
	template<typename T>
	struct TData
	{
		TArray<T> Data = { T{} };
		FIntVector Size = { 1, 1, 1};
	};
	
	TMap<FName, TArray<TData<FVoxelValue>>> ValueData;
	TMap<FName, TArray<TData<float>>> FloatData;

	FSimpleMulticastDelegate PostEditChange;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};

class FVoxelDebugEditor
{
public:
	static TSharedRef<SDockTab> CreateTab(const FSpawnTabArgs& Args);
};