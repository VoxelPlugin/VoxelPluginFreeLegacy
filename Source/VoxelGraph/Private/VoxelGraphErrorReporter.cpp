// Copyright 2020 Phyronnaz

#include "VoxelGraphErrorReporter.h"
#include "VoxelNode.h"
#include "VoxelGraphGenerator.h"
#include "IVoxelGraphEditor.h"
#include "VoxelNodes/VoxelGraphMacro.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Modules/ModuleManager.h"

FVoxelGraphErrorReporter::FVoxelGraphErrorReporter(const UVoxelGraphGenerator* VoxelGraphGenerator)
	: VoxelGraphGenerator(VoxelGraphGenerator)
	, Parent(nullptr)
	, ErrorPrefix("")
{
	ensure(VoxelGraphGenerator);
}

FVoxelGraphErrorReporter::FVoxelGraphErrorReporter(FVoxelGraphErrorReporter& Parent, const FString& ErrorPrefix)
	: VoxelGraphGenerator(Parent.VoxelGraphGenerator)
	, Parent(&Parent)
	, ErrorPrefix(Parent.ErrorPrefix + ErrorPrefix + ": ")
{
	ensure(VoxelGraphGenerator);
}

FVoxelGraphErrorReporter::~FVoxelGraphErrorReporter()
{
	if (Parent)
	{
		Parent->CopyFrom(*this);
	}
}

void FVoxelGraphErrorReporter::AddError(const FString& Error)
{
	if (!Error.IsEmpty())
	{
		const FString ErrorWithPrefix = AddPrefixToError(Error);
		Messages.Add(FVoxelGraphMessage{ nullptr, Error, EVoxelGraphNodeMessageType::FatalError });
		bHasFatalError = true;
	}
}

void FVoxelGraphErrorReporter::AddInternalError(const FString Error)
{
	ensureMsgf(false, TEXT("Internal voxel graph compiler error: %s"), *Error);

	const bool bOldHasErrors = bHasFatalError;
	AddError("Internal error: " + Error +
		"\nPlease create a bug report here: https://gitlab.com/Phyronnaz/VoxelPluginIssues/issues \n"
		"Don't forget to attach the generated header file");
	bHasFatalError = bOldHasErrors;
}

inline FString& GetErrorString(UVoxelGraphNodeInterface* Node, EVoxelGraphNodeMessageType Type)
{
	switch (Type)
	{
	case EVoxelGraphNodeMessageType::Info:
		return Node->InfoMsg;
	case EVoxelGraphNodeMessageType::Warning:
		return Node->WarningMsg;
	case EVoxelGraphNodeMessageType::Error:
	case EVoxelGraphNodeMessageType::FatalError:
		return Node->ErrorMsg;
	case EVoxelGraphNodeMessageType::Dependencies:
		return Node->DependenciesMsg;
	case EVoxelGraphNodeMessageType::Stats:
		return Node->StatsMsg;
	case EVoxelGraphNodeMessageType::RangeAnalysisWarning:
		return Node->RangeAnalysisWarningMsg;
	case EVoxelGraphNodeMessageType::RangeAnalysisError:
		return Node->RangeAnalysisErrorMsg;
	case EVoxelGraphNodeMessageType::RangeAnalysisDebug:
		return Node->RangeAnalysisDebugMsg;
	default:
		check(false);
		static FString Ref;
		return Ref;
	}
}

void FVoxelGraphErrorReporter::AddMessageToNode(
	const UVoxelNode* Node, 
	const FString& Message, 
	EVoxelGraphNodeMessageType Severity, 
	bool bSelectNode)
{
	check(Node);
	const FString MessageWithPrefix = AddPrefixToError(Message);

	if (Severity == EVoxelGraphNodeMessageType::FatalError)
	{
		bHasFatalError = true;
	}
	
	switch (Severity)
	{
	case EVoxelGraphNodeMessageType::Info:
	case EVoxelGraphNodeMessageType::Warning:
	case EVoxelGraphNodeMessageType::Error:
	case EVoxelGraphNodeMessageType::FatalError:
	case EVoxelGraphNodeMessageType::RangeAnalysisWarning:
	case EVoxelGraphNodeMessageType::RangeAnalysisError:
	{
		FVoxelGraphMessage NewMessage;
		NewMessage.Node = Node;
		NewMessage.Message = MessageWithPrefix;
		NewMessage.Type = Severity;
		Messages.Add(NewMessage);
		break;
	}
	case EVoxelGraphNodeMessageType::Dependencies:
	case EVoxelGraphNodeMessageType::Stats:
	case EVoxelGraphNodeMessageType::RangeAnalysisDebug:
	default:
		break;
	}

	if (bSelectNode)
	{
		AddNodeToSelect(Node);
	}

#if WITH_EDITORONLY_DATA
	if (auto* GraphNode = Node->GraphNode)
	{
		AddMessageToNodeInternal(Node, MessageWithPrefix, Severity);
		GraphsToRefresh.Add(GraphNode->GetGraph());
	}
#endif
}

void FVoxelGraphErrorReporter::AddNodeToSelect(const UVoxelNode* Node)
{
#if WITH_EDITORONLY_DATA
	if (Node && Node->GraphNode)
	{
		NodesToSelect.Add(Node->GraphNode);
	}
#endif
}

void FVoxelGraphErrorReporter::Apply(bool bSelectNodes)
{
#if WITH_EDITOR
	if (VoxelGraphGenerator && VoxelGraphGenerator->VoxelGraph)
	{
		GraphsToRefresh.Add(VoxelGraphGenerator->VoxelGraph);
		if (auto* VoxelGraphEditor = IVoxelGraphEditor::GetVoxelGraphEditor())
		{
			for (auto* GraphToRefresh : GraphsToRefresh)
			{
				VoxelGraphEditor->RefreshNodesMessages(GraphToRefresh);
			}
			if (NodesToSelect.Num() > 0 && bSelectNodes)
			{
				VoxelGraphEditor->SelectNodesAndZoomToFit(VoxelGraphGenerator->VoxelGraph, NodesToSelect.Array());
			}
			VoxelGraphEditor->AddMessages(VoxelGraphGenerator, Messages);
		}
	}
	else
#endif
	{
		for (auto& Message : Messages)
		{
			if (Message.Type == EVoxelGraphNodeMessageType::FatalError)
			{
				LOG_VOXEL(Warning, TEXT("%s failed to compile: %s"), VoxelGraphGenerator ? *VoxelGraphGenerator->GetName() : TEXT(""), *Message.Message);
			}
		}
	}
}

void FVoxelGraphErrorReporter::CopyFrom(FVoxelGraphErrorReporter& Other)
{
	check(VoxelGraphGenerator == Other.VoxelGraphGenerator);

	bHasFatalError |= Other.bHasFatalError;
	Messages.Append(Other.Messages);
	NodesToSelect.Append(Other.NodesToSelect);
	GraphsToRefresh.Append(Other.GraphsToRefresh);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphErrorReporter::ClearMessages(const UVoxelGraphGenerator* Graph, bool bClearAll, EVoxelGraphNodeMessageType MessagesToClear)
{
#if WITH_EDITOR
	if (auto* VoxelGraphEditor = IVoxelGraphEditor::GetVoxelGraphEditor())
	{
		VoxelGraphEditor->ClearMessages(Graph, bClearAll, MessagesToClear);
	}
#endif
}

void FVoxelGraphErrorReporter::ClearNodesMessages(const UVoxelGraphGenerator* Graph, bool bRecursive, bool bClearAll, EVoxelGraphNodeMessageType MessagesToClear)
{
#if WITH_EDITOR
	if (!Graph->VoxelGraph)
	{
		return;
	}

	static TSet<const UVoxelGraphGenerator*> Stack;
	if (Stack.Contains(Graph))
	{
		return;
	}
	Stack.Add(Graph);

	TSet<UVoxelGraphMacro*> Macros;
	for (auto* Node : Graph->VoxelGraph->Nodes)
	{
		if (auto* Interface = Cast<UVoxelGraphNodeInterface>(Node))
		{
			for (auto Type :
				{ EVoxelGraphNodeMessageType::Info,
				  EVoxelGraphNodeMessageType::Warning,
				  EVoxelGraphNodeMessageType::Error,
				  EVoxelGraphNodeMessageType::FatalError,
				  EVoxelGraphNodeMessageType::Dependencies,
				  EVoxelGraphNodeMessageType::Stats,
				  EVoxelGraphNodeMessageType::RangeAnalysisWarning,
				  EVoxelGraphNodeMessageType::RangeAnalysisError,
				  EVoxelGraphNodeMessageType::RangeAnalysisDebug })

			{
				if (bClearAll || MessagesToClear == Type)
				{
					GetErrorString(Interface, Type).Empty();
				}
			}
			if (bRecursive)
			{
				if (auto* MacroNode = Cast<UVoxelGraphMacroNode>(Interface->GetVoxelNode()))
				{
					auto* Macro = MacroNode->Macro;
					if (!Macros.Contains(Macro) && Macro)
					{
						Macros.Add(Macro);
						ClearNodesMessages(Macro, bRecursive, bClearAll, MessagesToClear);
					}
				}
			}
		}
	}
	if (auto* VoxelGraphEditor = IVoxelGraphEditor::GetVoxelGraphEditor())
	{
		VoxelGraphEditor->RefreshNodesMessages(Graph->VoxelGraph);
	}

	Stack.Remove(Graph);
#endif
}

void FVoxelGraphErrorReporter::ClearCompilationMessages(const UVoxelGraphGenerator* Graph)
{
	for (auto Type :
		{ EVoxelGraphNodeMessageType::Info,
		  EVoxelGraphNodeMessageType::Warning,
		  EVoxelGraphNodeMessageType::Error,
		  EVoxelGraphNodeMessageType::FatalError,
		  EVoxelGraphNodeMessageType::Dependencies
		})
	{
		ClearMessages(Graph, false, Type);
		ClearNodesMessages(Graph, true, false, Type);
	}
}


void FVoxelGraphErrorReporter::AddPerfCounters(const UVoxelGraphGenerator* Graph)
{
}


void FVoxelGraphErrorReporter::GetStats(const TSet<UObject*>& SelectedNodes, double& OutTotalTimeInSeconds, uint64& OutTotalCalls)
{
}

void FVoxelGraphErrorReporter::AddRangeAnalysisErrors(const UVoxelGraphGenerator* Graph)
{
}

void FVoxelGraphErrorReporter::AddMessageToNodeInternal(
	const UVoxelNode* Node,
	const FString& Message,
	EVoxelGraphNodeMessageType Severity)
{
#if WITH_EDITOR
	if (auto* GraphNode = Node->GraphNode)
	{
		FString& Text = GetErrorString(GraphNode, Severity);
		if (!Text.IsEmpty())
		{
			Text += "\n";
		}
		Text += Message;
	}
#endif
}

FString FVoxelGraphErrorReporter::AddPrefixToError(const FString& Error) const
{
	if (!VoxelGraphGenerator || VoxelGraphGenerator->bDetailedErrors)
	{
		return ErrorPrefix + Error;
	}
	else
	{
		return Error;
	}
}

