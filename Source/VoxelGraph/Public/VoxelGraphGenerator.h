// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelTexture.h"
#include "VoxelGraphOutputs.h"
#include "VoxelAxisDependencies.h"
#include "VoxelGenerators/VoxelGenerator.h"

#include "Engine/EngineTypes.h"
#include "EdGraph/EdGraphPin.h"
#include "VoxelGraphGenerator.generated.h"

class UEdGraph;
class UVoxelNode;
class UTexture2D;
class UVoxelExposedNode;
class UVoxelGraphOutputsConfig;
class UVoxelGraphPreviewSettings;
class FVoxelGraphGeneratorInstance;
struct FVoxelCompiledGraphs;

UENUM()
enum class EVoxelGraphGeneratorDebugLevel : uint8
{
	BeforeMacroInlining,
	AfterMacroInlining,
	AfterBiomeMergeReplace,
	AfterSmartMinMaxReplace,
	BeforeFillFunctionSeparators,
	Output,
	Function,
	Axis
};

/**
 * A graph generator
 */
UCLASS(BlueprintType, HideCategories = (Object), HideDropdown)
class VOXELGRAPH_API UVoxelGraphGenerator : public UVoxelTransformableGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelGraphOutputsConfig> Outputs;
	
	TMap<uint32, FVoxelGraphOutput> GetOutputs() const;
	TArray<FVoxelGraphPermutationArray> GetPermutations() const;

public:
	UPROPERTY()
	bool bAutomaticPreview = true;

	UPROPERTY()
	bool bShowFlowMergeAndFunctionsWarnings = true;
	
public:
	UPROPERTY(EditAnywhere, Category = "Automatic compilation", meta= (DisplayName = "Compile to C++ on Save"))
	bool bCompileToCppOnSave = false;

	// Relative to project directory
	UPROPERTY(EditAnywhere, Category = "Automatic compilation", meta = (FilePathFilter = "h", EditCondition = bCompileToCppOnSave))
	FFilePath SaveLocation;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FString LastSavePath;
#endif

public:
	// Range analysis gives a pretty significant speed-up. You should not disable it
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Range Analysis")
	bool bEnableRangeAnalysis = true;

public:
	// Will show the nodes functions. If DetailedErrors is false, will only show TargetToDebug
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (Refresh))
	bool bShowFunctions = false;

	// Show errors callstacks
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (Refresh))
	bool bDetailedErrors = false;

	// Can be enabled in Window->Debug Graph
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (Refresh))
	bool bEnableDebugGraph = false;

	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	bool bShowPinsIds = false;

	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	bool bShowAxisDependencies = false;

	// The level of compilation to debug
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	EVoxelGraphGeneratorDebugLevel DebugLevel;

	// The target to debug, if DebugLevel is below or equal to Target
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	FString TargetToDebug = "Value";

	// The function to debug, if DebugLevel is below or equal to Function
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	int32 FunctionToDebug = 0;

	// The axis to debug, if DebugLevel is Axis
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	EVoxelFunctionAxisDependencies AxisDependenciesToDebug;

	// Increase this if your macro nodes are overlapping in the debug graph
	UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay, meta = (EditCondition = bEnableDebugGraph, Refresh))
	float NodesDepthScaleFactor = 1;
	
	UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay, meta = (EditCondition = bEnableDebugGraph, Refresh))
	bool bHideDataNodes = false;

public:
	UPROPERTY()
	TArray<TObjectPtr<UVoxelNode>> AllNodes;
	
	UPROPERTY()
	TArray<TObjectPtr<UVoxelNode>> DebugNodes;

	UPROPERTY()
	TObjectPtr<UVoxelNode> FirstNode;

	UPROPERTY()
	FGuid FirstNodePinId;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UEdGraph> VoxelGraph;

	UPROPERTY()
	TObjectPtr<UEdGraph> VoxelDebugGraph;
	
	FEdGraphPinReference PreviewedPin;
#endif
	
	UPROPERTY()
	TObjectPtr<UVoxelGraphPreviewSettings> PreviewSettings;

public:
	TMap<FName, FString> TransientParameters;

public:

	//~ Begin UVoxelGenerator Interface
	virtual void ApplyParameters(const TMap<FName, FString>& Parameters) override;
	virtual void GetParameters(TArray<FVoxelGeneratorParameter>& OutParameters) const override;
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance(const TMap<FName, FString>& Parameters) override;;
	//~ End UVoxelGenerator Interface

#if WITH_EDITOR
	//~ Begin UObject Interface 
	void PostInitProperties() override;
	void PostLoad() override;
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
	
	// Create a new node of NewNodeClass
	UVoxelNode* ConstructNewNode(UClass* NewNodeClass, const FVector2D& Position, bool bSelectNewNode = true);
	template<typename T>
	T* ConstructNewNode(const FVector2D& Position, bool bSelectNewNode = true)
	{
		return CastChecked<T>(ConstructNewNode(T::StaticClass(), Position, bSelectNewNode));
	}
	// Create the basic voxel graph
	void CreateGraphs();

	// Use the EdGraph representation to compile the VoxelNodes
	void CompileVoxelNodesFromGraphNodes();
#endif

#if WITH_EDITORONLY_DATA
public:
	UTexture2D* GetPreviewTexture();
	void SetPreviewTexture(const TArray<FColor>& Colors, int32 Size);
	
private:
	UPROPERTY(NonTransactional)
	TArray<FColor> PreviewTextureSave;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> PreviewTexture;
#endif

private:
	void UpdateSetterNodes();
	void BindUpdateSetterNodes();
};