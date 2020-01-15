// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelGraphOutputs.h"
#include "VoxelAxisDependencies.h"
#include "VoxelWorldGenerator.h"
#include "VoxelTexture.h"
#include "EdGraph/EdGraphPin.h"
#include "VoxelEditorDelegatesInterface.h"
#include "VoxelGraphGenerator.generated.h"

class UEdGraph;
class UVoxelNode;
class UTexture2D;
class UVoxelGraphPreviewSettings;
class UVoxelGraphOutputsConfig;
class FVoxelGraphGeneratorInstance;
struct FVoxelCompiledGraphs;

UENUM()
enum class EVoxelGraphGeneratorDebugLevel : uint8
{
	BeforeMacroInlining,
	AfterMacroInlining,
	AfterBiomeMergeReplace,
	Output,
	Function,
	Axis
};

/**
 * A graph world generator
 */
UCLASS(BlueprintType, HideCategories = (Object), HideDropdown)
class VOXELGRAPH_API UVoxelGraphGenerator : public UVoxelTransformableWorldGenerator, public IVoxelEditorDelegatesInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	UVoxelGraphOutputsConfig* Outputs;
	
	TMap<uint32, FVoxelGraphOutput> GetOutputs() const;
	TArray<FVoxelGraphPermutationArray> GetPermutations() const;

public:
	UPROPERTY()
	bool bAutomaticPreview = true;

	UPROPERTY()
	bool bEnableStats = false;
	
	UPROPERTY()
	bool bEnableRangeAnalysisDebug = false;

	UPROPERTY()
	bool bShowFlowMergeAndFunctionsWarnings = true;
	
public:
	UPROPERTY(EditAnywhere, Category = "Automatic compilation", meta= (DisplayName = "Compile to C++ on Save"))
	bool bCompileToCppOnSave = false;
	
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

	UPROPERTY(EditAnywhere, Category = "Range Analysis")
	bool bHideRangeAnalysisErrors = false;

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
	UPROPERTY(Transient)
	TMap<FName, float> FloatParameters;

	UPROPERTY(Transient)
	TMap<FName, int32> IntParameters;
	
	UPROPERTY(Transient)
	TMap<FName, bool> BoolParameters;
	
	UPROPERTY(Transient)
	TMap<FName, FLinearColor> ColorParameters;
	
	UPROPERTY(Transient)
	TMap<FName, FVoxelFloatTexture> VoxelTextureParameters;

	void ClearParametersOverrides();

	float GetFloatParameter(const FName& Name, float DefaultValue) const;
	int32 GetIntParameter(const FName& Name, int32 DefaultValue) const;
	bool GetBoolParameter(const FName& Name, bool DefaultValue) const;
	FLinearColor GetColorParameter(const FName& Name, FLinearColor DefaultValue) const;
	FVoxelFloatTexture GetTextureParameter(const FName& Name) const;

public:

	//~ Begin UVoxelWorldGenerator Interface
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
	virtual void SaveInstance(const FVoxelTransformableWorldGeneratorInstance& Instance, FArchive& Ar) const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> LoadInstance(FArchive& Ar) const override;
	//~ End UVoxelWorldGenerator Interface

#if WITH_EDITOR
	//~ Begin UObject Interface 
	void PostInitProperties() override;
	void PostLoad() override;
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
	
	//~ Begin IVoxelEditorDelegatesInterface Interface
	virtual void OnPreBeginPIE(bool bIsSimulating) override;
	virtual void OnEndPIE(bool bIsSimulating) override;
	//~ End IVoxelEditorDelegatesInterface Interface
	
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
	void BindUpdateSetterNodes();
};