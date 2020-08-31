// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "VoxelToolManager.generated.h"

class UVoxelToolSharedConfig;
class UVoxelTool;

UCLASS(BlueprintType, Blueprintable)
class VOXEL_API UVoxelToolManager : public UObject
{
	GENERATED_BODY()

public:
	UVoxelToolManager();

private:
	UPROPERTY()
	UVoxelToolSharedConfig* SharedConfig = nullptr;

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools", meta = (DisplayName = "Get Shared Config"))
	UVoxelToolSharedConfig* K2_GetSharedConfig() const { return SharedConfig; }
	UVoxelToolSharedConfig& GetSharedConfig() const { check(SharedConfig); return *SharedConfig; }
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools")
	UVoxelTool* GetActiveTool() const { return ActiveTool; }

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools")
	const TArray<UVoxelTool*>& GetTools() const { return Tools; }

public:
	// If bLoadBlueprints is true, all the blueprints inheriting from VoxelTool will be force loaded
	// If false, tools whose blueprints are not loaded won't show up
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools")
	void CreateDefaultTools(bool bLoadBlueprints = false);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools")
	void SetActiveTool(UVoxelTool* NewActiveTool);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools")
	void SetActiveToolByClass(TSubclassOf<UVoxelTool> NewActiveTool);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools")
	void SetActiveToolByName(FName NewActiveTool);

public:
	template<typename T>
	T* GetActiveTool() const
	{
		return Cast<T>(ActiveTool);
	}
	template<typename T>
	void SetActiveTool()
	{
		static_assert(TIsDerivedFrom<T, UVoxelTool>::IsDerived, "T must be derived from UVoxelTool");
		SetActiveToolByClass(T::StaticClass());
	}
	template<typename T>
	T& GetOrSetActiveTool()
	{
		static_assert(TIsDerivedFrom<T, UVoxelTool>::IsDerived, "T must be derived from UVoxelTool");
		
		if (auto* Tool = Cast<T>(ActiveTool))
		{
			return *Tool;
		}

		SetActiveTool<T>();

		return *CastChecked<T>(ActiveTool);
	}
	
private:
	UPROPERTY(Transient)
	UVoxelTool* ActiveTool = nullptr;

	UPROPERTY(Transient)
	TArray<UVoxelTool*> Tools;
};