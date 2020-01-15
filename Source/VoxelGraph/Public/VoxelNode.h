// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelPinCategory.h"
#include "EdGraph/EdGraphNode.h"
#include "VoxelNode.generated.h"

class UVoxelNode;
class FVoxelComputeNode;
class FVoxelCompilationNode;
class UEdGraphNode;
class FVoxelGraphErrorReporter;
class UVoxelGraphGenerator;

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelGraphNodeInterface : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FString InfoMsg;

	UPROPERTY(Transient)
	FString WarningMsg;
	
	UPROPERTY(Transient)
	FString DependenciesMsg;

	UPROPERTY(Transient)
	FString StatsMsg;

	UPROPERTY(Transient)
	FString RangeAnalysisWarningMsg;

	UPROPERTY(Transient)
	FString RangeAnalysisErrorMsg;
	
	UPROPERTY(Transient)
	FString RangeAnalysisDebugMsg;

	virtual UVoxelNode* GetVoxelNode() const { return nullptr; }
	virtual bool IsOutdated() const { return false; }
	
#if WITH_EDITOR
	virtual void PostLoad() override;
	virtual void ReconstructNode() override;
#endif
};

USTRUCT()
struct FVoxelPin
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid PinId;

	UPROPERTY()
	FString DefaultValue;

	UPROPERTY()
	TArray<UVoxelNode*> OtherNodes;

	UPROPERTY()
	TArray<FGuid> OtherPinIds;

	// Used for macros to check that the nodes are the same
	UPROPERTY()
	EVoxelPinCategory PinCategory = EVoxelPinCategory::Exec;
	
	FVoxelPin() = default;
	
	FVoxelPin(const FGuid& PinId, const FString& DefaultValue, EVoxelPinCategory PinCategory, const TArray<UVoxelNode*>& OtherNodes, const TArray<FGuid>& OtherPinIds)
		: PinId(PinId)
		, DefaultValue(DefaultValue)
		, OtherNodes(OtherNodes)
		, OtherPinIds(OtherPinIds)
		, PinCategory(PinCategory)
	{
	}
};

struct FVoxelPinDefaultValueBounds
{
	TOptional<float> Min;
	TOptional<float> Max;
};

/**
 * Base class for VoxelNodes
 */
UCLASS(Abstract, HideCategories = Object, EditInlineNew)
class VOXELGRAPH_API UVoxelNode : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FVoxelPin> InputPins;

	UPROPERTY()
	TArray<FVoxelPin> OutputPins;

	UPROPERTY()
	UVoxelGraphGenerator* Graph;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UVoxelGraphNodeInterface* GraphNode;
#endif

	UPROPERTY()
	int32 InputPinCount;

public:
	int32 GetInputPinIndex(const FGuid& PinId);
	int32 GetOutputPinIndex(const FGuid& PinId);

	bool HasInputPinWithCategory(EVoxelPinCategory Category) const;
	bool HasOutputPinWithCategory(EVoxelPinCategory Category) const;

public:
	//~ Begin UVoxelNode Interface
	virtual int32 GetMaxInputPins() const { return 0; }
	virtual int32 GetMinInputPins() const { return 0; }
	virtual int32 GetInputPinsIncrement() const { return 1; }
	virtual void OnInputPinCountModified() {}
	
	virtual int32 GetOutputPinsCount() const { return 0; }

	virtual FLinearColor GetColor() const { return FLinearColor::Black; }
	virtual FLinearColor GetNodeBodyColor() const { return FLinearColor::White; }
	virtual FText GetTitle() const;
	virtual FText GetTooltip() const;
	virtual bool IsCompact() const { return false; }

	virtual FName GetInputPinName(int32 PinIndex) const { return FName(); }
	virtual FName GetOutputPinName(int32 PinIndex) const { return FName(); }

	virtual FString GetInputPinToolTip(int32 PinIndex) const { return FString(); }
	virtual FString GetOutputPinToolTip(int32 PinIndex) const { return FString(); }

	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const { return EVoxelPinCategory::Float; }
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const { return EVoxelPinCategory::Float; }

	virtual FVoxelPinDefaultValueBounds GetInputPinDefaultValueBounds(int32 PinIndex) const { return {}; }
	virtual FString GetInputPinDefaultValue(int32 PinIndex) const { return ""; }
	

	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter);

	virtual bool CanUserDeleteNode() const { return true; }
	virtual bool CanDuplicateNode() const { return true; }
	
	/**
	 * Can this node be renamed?
	 */
	virtual bool CanRenameNode() const { return false; }

	/**
	 * Returns the current 'name' of the node
	 * Only valid to call on a node that previously returned CanRenameNode() = true.
	 */
	virtual FString GetEditableName() const { return ""; }

	/**
	 * Sets the current 'name' of the node
	 * Only valid to call on a node that previously returned CanRenameNode() = true.
	 */
	virtual void SetEditableName(const FString& NewName) {}

	/**
	 * Called after a node copy, once the outer is set correctly and that all new nodes are added to Graph->AllNodes
	 * @param	CopiedNodes	The nodes copied in this copy
	 */
	virtual void PostCopyNode(const TArray<UVoxelNode*>& CopiedNodes) {}
	//~ End UVoxelNode Interface

#if WITH_EDITOR
	//~ Begin UObject Interface
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void PostLoad() override;
	//~ End UObject Interface
#endif //WITH_EDITOR

private:
	bool IsOutdated() const;
};
