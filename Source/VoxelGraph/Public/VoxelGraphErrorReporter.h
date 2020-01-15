// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UVoxelNode;
class UVoxelGraphGenerator;
class UEdGraph;
class UEdGraphNode;
class FVoxelCompilationNode;
class FVoxelComputeNode;

enum class EVoxelGraphNodeMessageType : int32
{
	Info,
	Warning,
	Error,
	FatalError,
	Dependencies,
	Stats,
	RangeAnalysisWarning,
	RangeAnalysisError,
	RangeAnalysisDebug
};

struct FVoxelGraphMessage
{
	TWeakObjectPtr<const UVoxelNode> Node;
	FString Message;
	EVoxelGraphNodeMessageType Type;
};

class VOXELGRAPH_API FVoxelGraphErrorReporter
{
public:
	FVoxelGraphErrorReporter(const UVoxelGraphGenerator* VoxelGraphGenerator);
	// Will copy to Parent on deletion
	FVoxelGraphErrorReporter(FVoxelGraphErrorReporter& Parent, const FString& ErrorPrefix);
	~FVoxelGraphErrorReporter();
	
	inline bool HasFatalError() const { return bHasFatalError; }

	void AddError(const FString& Error);
	void AddInternalError(const FString Error); // Won't break but will report an error

	void AddMessageToNode(
		const UVoxelNode* Node, 
		const FString& Message, 
		EVoxelGraphNodeMessageType Severity, 
		bool bSelectNode = true);

	void AddNodeToSelect(const UVoxelNode* Node);
	
public:
	void Apply(bool bSelectNodes);

public:
	void CopyFrom(FVoxelGraphErrorReporter& Other);
	
public:
	static void ClearMessages(const UVoxelGraphGenerator* Graph, bool bClearAll = true, EVoxelGraphNodeMessageType MessagesToClear = EVoxelGraphNodeMessageType::Error);
	static void ClearNodesMessages(const UVoxelGraphGenerator* Graph, bool bRecursive = true, bool bClearAll = true, EVoxelGraphNodeMessageType MessagesToClear = EVoxelGraphNodeMessageType::Error);
	static void ClearCompilationMessages(const UVoxelGraphGenerator* Graph);

	static void AddPerfCounters(const UVoxelGraphGenerator* Graph);
	static void GetStats(const TSet<UObject*>& SelectedNodes, double& OutTotalTimeInSeconds, uint64& OutTotalCalls);
	static void AddRangeAnalysisErrors(const UVoxelGraphGenerator* Graph);
	static void AddMessageToNodeInternal(
		const UVoxelNode* Node,
		const FString& Message,
		EVoxelGraphNodeMessageType Severity);

private:
	const UVoxelGraphGenerator* const VoxelGraphGenerator;
	FVoxelGraphErrorReporter* const Parent;
	const FString ErrorPrefix;

	bool bHasFatalError = false;

	TArray<FVoxelGraphMessage> Messages;

	TSet<UEdGraphNode*> NodesToSelect;
	TSet<UEdGraph*> GraphsToRefresh;

	FString AddPrefixToError(const FString& Error) const;
};

