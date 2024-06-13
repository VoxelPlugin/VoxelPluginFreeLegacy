// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/EnumRange.h"
#include "UObject/WeakObjectPtr.h"

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
	Error
};
ENUM_RANGE_BY_FIRST_AND_LAST(EVoxelGraphNodeMessageType, EVoxelGraphNodeMessageType::Info, EVoxelGraphNodeMessageType::Error);

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
	
	bool HasError() const { return bHasError; }

	void AddError(const FString& Error);
	void AddInternalError(const FString Error); // Won't break but will report an error

	void AddMessageToNode(
		const UVoxelNode* Node, 
		const FString& Message, 
		EVoxelGraphNodeMessageType Severity, 
		bool bSelectNode = true,
		bool bShowInList = true);

	void AddNodeToSelect(const UVoxelNode* Node);
	
public:
	void Apply(bool bSelectNodes);

public:
	void CopyFrom(FVoxelGraphErrorReporter& Other);
	
public:
	static void ClearMessages(const UVoxelGraphGenerator* Graph, bool bClearAll = true, EVoxelGraphNodeMessageType MessagesToClear = EVoxelGraphNodeMessageType::Error);
	static void ClearNodesMessages(const UVoxelGraphGenerator* Graph, bool bRecursive = true, bool bClearAll = true, EVoxelGraphNodeMessageType MessagesToClear = EVoxelGraphNodeMessageType::Error);
	static void ClearCompilationMessages(const UVoxelGraphGenerator* Graph);

	static void AddMessageToNodeInternal(
		const UVoxelNode* Node,
		const FString& Message,
		EVoxelGraphNodeMessageType Severity);

private:
	const UVoxelGraphGenerator* const VoxelGraphGenerator;
	FVoxelGraphErrorReporter* const Parent;
	const FString ErrorPrefix;

	bool bHasError = false;

	TArray<FVoxelGraphMessage> Messages;

	TSet<UEdGraphNode*> NodesToSelect;
	TSet<UEdGraph*> GraphsToRefresh;

	FString AddPrefixToError(const FString& Error) const;
};

