// Copyright 2021 Phyronnaz

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
	UPROPERTY()
	UVoxelGraphOutputsConfig* Outputs_DEPRECATED;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FVoxelGraphOutput> CustomOutputs;
	
	TMap<uint32, FVoxelGraphOutput> GetOutputs() const;
	TArray<FVoxelGraphPermutationArray> GetPermutations() const;

public:
	UPROPERTY()
	bool bAutomaticPreview = true;

	UPROPERTY()
	bool bShowFlowMergeAndFunctionsWarnings = true;
	
public:
	// If enabled, will use the compiled to C++ graph instead of the graph VM, even if the graph is used directly
	UPROPERTY(EditAnywhere, Category = "Compilation", meta = (DisplayName = "Use C++ class instead of graph"))
	bool bUseCppClassInsteadOfGraph = false;

	// The class to use instead of the graph
	UPROPERTY(EditAnywhere, Category = "Compilation", meta = (DisplayName = "Generated C++ class", EditCondition = "bUseCppClassInsteadOfGraph"))
	TSoftClassPtr<UVoxelGenerator> GeneratedCppClass;
	
	UPROPERTY(EditAnywhere, Category = "Compilation", AdvancedDisplay, meta = (DisplayName = "Compile to C++ on Save"))
	bool bCompileToCppOnSave = false;

	// Relative to project directory
	UPROPERTY(EditAnywhere, Category = "Compilation", AdvancedDisplay, meta = (FilePathFilter = "h"))
	FFilePath SaveLocation;

	// If true, SaveLocation is relative to the plugin directory
	UPROPERTY(EditAnywhere, Category = "Compilation", AdvancedDisplay)
	bool bBuiltinPluginGenerator = false;

public:
	// Range analysis gives a pretty significant speed-up. You should not disable it
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Config")
	bool bEnableRangeAnalysis = true;

public:
	// Can be enabled in Window->Debug Graph
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (Refresh))
	bool bEnableDebugGraph = false;
	
	// Will show the nodes functions. If DetailedErrors is false, will only show TargetToDebug
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Debug", meta = (Refresh))
	bool bShowFunctions = false;

	// Show errors callstacks
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Debug", meta = (Refresh))
	bool bDetailedErrors = false;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	bool bShowPinsIds = false;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	bool bShowAxisDependencies = false;

	// The level of compilation to debug
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	EVoxelGraphGeneratorDebugLevel DebugLevel;

	// The target to debug, if DebugLevel is below or equal to Target
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	FString TargetToDebug = "Value";

	// The function to debug, if DebugLevel is below or equal to Function
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	int32 FunctionToDebug = 0;

	// The axis to debug, if DebugLevel is Axis
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	EVoxelFunctionAxisDependencies AxisDependenciesToDebug;

	// Increase this if your macro nodes are overlapping in the debug graph
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	float NodesDepthScaleFactor = 1;
	
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Debug", meta = (EditCondition = bEnableDebugGraph, Refresh))
	bool bHideDataNodes = false;

public:
	UPROPERTY()
	TArray<UVoxelNode*> AllNodes;
	
	UPROPERTY()
	TArray<UVoxelNode*> DebugNodes;

	UPROPERTY()
	UVoxelNode* FirstNode;

	UPROPERTY()
	FGuid FirstNodePinId;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UEdGraph* VoxelGraph;

	UPROPERTY()
	UEdGraph* VoxelDebugGraph;
	
	FEdGraphPinReference PreviewedPin;
#endif
	
	UPROPERTY()
	UVoxelGraphPreviewSettings* PreviewSettings;

public:
	TMap<FName, FString> TransientParameters;

public:

	//~ Begin UVoxelGenerator Interface
	virtual void ApplyParameters(const TMap<FName, FString>& Parameters) override;
	virtual TArray<FVoxelGeneratorParameter> GetParameters() const override;
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance(const TMap<FName, FString>& Parameters) override;
	virtual FVoxelGeneratorOutputs GetGeneratorOutputs() const override;
	//~ End UVoxelGenerator Interface

#if WITH_EDITOR
	//~ Begin UObject Interface 
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
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
	UTexture2D* PreviewTexture;
#endif

private:
	void UpdateSetterNodes();
};