// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LatentActions.h"
#include "VoxelPaintMaterial.h"
#include "VoxelWorld.h"
#include "VoxelProjectionTools.generated.h"

class UCurveFloat;

UENUM(BlueprintType)
enum class EBlueprintSuccess : uint8
{
	Success,
	Failed
};

UENUM(BlueprintType)
enum class EFailReason : uint8
{
	VoxelDataLocked,
	LinetracesPending,
	OtherError
};

USTRUCT(BlueprintType)
struct FModifiedVoxelValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float OldValue = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float NewValue = 0;
};

USTRUCT(BlueprintType)
struct FModifiedVoxelMaterial
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelMaterial OldMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelMaterial NewMaterial;
};

USTRUCT(BlueprintType)
struct FGetVoxelProjectionVoxel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Value;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelMaterial Material;
};

struct FVoxelToolHitVoxel
{
	FIntVector Position;
	float Distance;
	FVoxelToolHitVoxel(const FIntVector& Position, float Distance) : Position(Position), Distance(Distance) {}
};

struct FVoxelProjectionEditWork
{
	TWeakObjectPtr<AVoxelWorld> const World;
	bool const bShowRaycasts;
	bool const bAdd;

	int TracesCompleted = 0;
	int TotalTraces = 0;

	TSet<FIntVector> AddedPoints;
	
	// Out
	FIntBox Bounds;
	TArray<FVoxelToolHitVoxel> HitVoxels;

	FVoxelProjectionEditWork(AVoxelWorld* World, bool bShowRaycasts, bool bAdd) : World(World), bShowRaycasts(bShowRaycasts), bAdd(bAdd) {}

	inline bool IsDone() const { return TracesCompleted == TotalTraces; }
};

class FVoxelProjectionEditLatentAction : public FPendingLatentAction
{
public:
	TSharedPtr<FVoxelProjectionEditWork> Work;
	TFunction<void(FVoxelProjectionEditWork&)> Callback;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	FVoxelProjectionEditLatentAction(TSharedPtr<FVoxelProjectionEditWork> Work, const FLatentActionInfo& LatentInfo, TFunction<void(FVoxelProjectionEditWork&)> Callback)
		: Work(Work)
		, Callback(Callback)
		, ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		bool bFinished = Work->IsDone();
		if (bFinished)
		{
			Callback(*Work);
		}
		Response.FinishAndTriggerIf(bFinished, ExecutionFunction, OutputLink, CallbackTarget);
	}

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		return "ProjectionEdit: Waiting for completion (" + FString::FromInt(Work->TracesCompleted) + "/" + FString::FromInt(Work->TotalTraces) + ")";
	}
#endif
};

UCLASS()
class VOXEL_API UVoxelProjectionTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UVoxelProjectionTools()
	{
		if (!TraceDelegate.IsBound())
		{
			TraceDelegate.BindStatic(&TraceDone);
		}
	}
	
public:
	/**
	 * Set Value Projection Async
	 * @param	ModifiedVoxels
	 * @param	World
	 * @param	Position				In world space
	 * @param	Normal					
	 * @param	Radius					In world space
	 * @param	Strength				Between -1 and 1
	 * @param	StrengthCurve			Strength multiplier = Curve(DistanceToCenter/Radius)
	 * @param	ToolHeight				
	 * @param	EditDistance
	 * @param	StepInVoxel				Step between traces
	 * @param	TimeoutInMicroSeconds	Timeout on the lock query
	 * @param	bShowRaycasts
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (Latent, LatentInfo="LatentInfo", ExpandEnumAsExecs = "Branches", WorldContext = "WorldContextObject", AdvancedDisplay = 11))
	 static void SetValueProjectionAsync(
		UObject* WorldContextObject,
		struct FLatentActionInfo LatentInfo,
		EBlueprintSuccess& Branches,
		EFailReason& FailReason,
		TArray<FModifiedVoxelValue>& ModifiedVoxels,
		AVoxelWorld* World,
		FVector Position,
		FVector Normal, 
		float Radius = 100, 
		float Strength = 0.5,
		UCurveFloat* StrengthCurve = nullptr,
		float ToolHeight = 200, 
		float EditDistance = 400, 
		float StepInVoxel = 0.5f, 
		int TimeoutInMicroSeconds = 500,
		bool bShowRaycasts = false);
	
	/**
	 * Set Material Projection Async
	 * @param	ModifiedVoxels
	 * @param	World
	 * @param	Position				In world space
	 * @param	Normal					
	 * @param	Radius					In world space
	 * @param	PaintMaterial			
	 * @param	ToolHeight				
	 * @param	EditDistance
	 * @param	StepInVoxel				Step between traces
	 * @param	TimeoutInMicroSeconds	Timeout on the lock query
	 * @param	bShowRaycasts
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (Latent, LatentInfo="LatentInfo", ExpandEnumAsExecs = "Branches", WorldContext = "WorldContextObject", AdvancedDisplay = 11))
	static void SetMaterialProjectionAsync(
		UObject* WorldContextObject,
		struct FLatentActionInfo LatentInfo,
		EBlueprintSuccess& Branches, 
		EFailReason& FailReason,
		TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
		AVoxelWorld* World, 
		FVector Position, 
		FVector Normal, 
		FVoxelPaintMaterial PaintMaterial,
		UCurveFloat* StrengthCurve = nullptr,
		float Radius = 100, 
		float ToolHeight = 200,
		float EditDistance = 400,
		float StepInVoxel = 0.5f, 
		int TimeoutInMicroSeconds = 500,
		bool bShowRaycasts = false);

	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (Latent, LatentInfo="LatentInfo", ExpandEnumAsExecs = "Branches", WorldContext = "WorldContextObject", AdvancedDisplay = 8))
	static void GetVoxelsProjectionAsync(
		UObject* WorldContextObject,
		struct FLatentActionInfo LatentInfo,
		EBlueprintSuccess& Branches, 
		TArray<FGetVoxelProjectionVoxel>& OutVoxels,
		AVoxelWorld* World, 
		FVector Position, 
		FVector Normal, 
		float Radius = 100, 
		float ToolHeight = 200,
		float EditDistance = 400,
		float StepInVoxel = 0.5f, 
		bool bShowRaycasts = false);
	
public:
	/**
	 * Set Value Projection
	 * @param	ModifiedVoxels
	 * @param	World
	 * @param	Position				In world space
	 * @param	Normal					
	 * @param	Radius					In world space
	 * @param	Strength				Between -1 and 1
	 * @param	StrengthCurve			Strength multiplier = Curve(DistanceToCenter/Radius)
	 * @param	ToolHeight				
	 * @param	EditDistance
	 * @param	StepInVoxel				Step between traces
	 * @param	TimeoutInMicroSeconds	Timeout on the lock query
	 * @param	bShowRaycasts
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (ExpandEnumAsExecs = "Branches", AdvancedDisplay = 9, DisplayName = "Set Value Projection"))
	 static void SetValueProjectionNew(
		EBlueprintSuccess& Branches, 
		EFailReason& FailReason,
		TArray<FModifiedVoxelValue>& ModifiedVoxels,
		AVoxelWorld* World,
		FVector Position,
		FVector Normal, 
		float Radius = 100, 
		float Strength = 0.5,
		UCurveFloat* StrengthCurve = nullptr,
		float ToolHeight = 200, 
		float EditDistance = 400, 
		float StepInVoxel = 0.5f, 
		int TimeoutInMicroSeconds = 500,
		bool bShowRaycasts = false);
	
	/**
	 * Set Material Projection
	 * @param	ModifiedVoxels
	 * @param	World
	 * @param	Position				In world space
	 * @param	Normal					
	 * @param	Radius					In world space
	 * @param	PaintMaterial			
	 * @param	ToolHeight				
	 * @param	EditDistance
	 * @param	StepInVoxel				Step between traces
	 * @param	TimeoutInMicroSeconds	Timeout on the lock query
	 * @param	bShowRaycasts
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (ExpandEnumAsExecs = "Branches", AdvancedDisplay = 9, DisplayName = "Set Material Projection"))
	static void SetMaterialProjectionNew(
		EBlueprintSuccess& Branches, 
		EFailReason& FailReason,
		TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
		AVoxelWorld* World, 
		FVector Position, 
		FVector Normal, 
		FVoxelPaintMaterial PaintMaterial,
		UCurveFloat* StrengthCurve = nullptr,
		float Radius = 100, 
		float ToolHeight = 200,
		float EditDistance = 400,
		float StepInVoxel = 0.5f, 
		int TimeoutInMicroSeconds = 500,
		bool bShowRaycasts = false);

	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (ExpandEnumAsExecs = "Branches", AdvancedDisplay = 6))
	static void GetVoxelsProjection(
		EBlueprintSuccess& Branches, 
		TArray<FGetVoxelProjectionVoxel>& OutVoxels,
		AVoxelWorld* World, 
		FVector Position, 
		FVector Normal, 
		float Radius = 100, 
		float ToolHeight = 200,
		float EditDistance = 400,
		float StepInVoxel = 0.5f, 
		bool bShowRaycasts = false);

private:
	struct FTraceDetails
	{
		FVoxelProjectionEditWork* Work = nullptr;
		float Distance;
	};

	static TMap<struct FTraceHandle, FTraceDetails> TraceMap;
	static FTraceDelegate TraceDelegate;
	static void TraceDone(const struct FTraceHandle& TraceHandle, struct FTraceDatum& TraceDatum);
	
private:
	template<bool bAsync>
	static void FindHitVoxelsForRaycasts(FVoxelProjectionEditWork& Work, FVector Position, FVector Normal, float Radius, float ToolHeight, float EditDistance, float StepInVoxel);
};