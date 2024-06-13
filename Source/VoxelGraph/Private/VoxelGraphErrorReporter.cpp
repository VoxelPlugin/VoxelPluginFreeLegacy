// Copyright Voxel Plugin SAS. All Rights Reserved.

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
		Messages.Add(FVoxelGraphMessage{ nullptr, Error, EVoxelGraphNodeMessageType::Error });
		bHasError = true;
	}
}

void FVoxelGraphErrorReporter::AddInternalError(const FString Error)
{
	ensureMsgf(false, TEXT("Internal voxel graph compiler error: %s"), *Error);

	const bool bOldHasErrors = bHasError;
	AddError("Internal error: " + Error +
		"\nPlease create a bug report here: https://gitlab.com/Phyronnaz/VoxelPluginIssues/issues \n"
		"Don't forget to attach the generated header file");
	bHasError = bOldHasErrors;
}

inline FString& GetErrorString(UVoxelGraphNodeInterface* Node, EVoxelGraphNodeMessageType Type)
{
	switch (Type)
	{
	default: ensure(false);
	case EVoxelGraphNodeMessageType::Info:
		return Node->InfoMsg;
	case EVoxelGraphNodeMessageType::Warning:
		return Node->WarningMsg;
	case EVoxelGraphNodeMessageType::Error:
		return Node->ErrorMsg;
	}
}

void FVoxelGraphErrorReporter::AddMessageToNode(
	const UVoxelNode* Node, 
	const FString& Message, 
	EVoxelGraphNodeMessageType Severity, 
	bool bSelectNode,
	bool bShowInList)
{
	check(Node);
	const FString MessageWithPrefix = AddPrefixToError(Message);

	if (Severity == EVoxelGraphNodeMessageType::Error)
	{
		bHasError = true;
	}

	if (bShowInList)
	{
		FVoxelGraphMessage NewMessage;
		NewMessage.Node = Node;
		NewMessage.Message = MessageWithPrefix;
		NewMessage.Type = Severity;
		Messages.Add(NewMessage);
	}

	if (bSelectNode)
	{
		AddNodeToSelect(Node);
	}

#if WITH_EDITORONLY_DATA
	if (UVoxelGraphNodeInterface* GraphNode = Node->GraphNode)
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
			if (Message.Type == EVoxelGraphNodeMessageType::Error)
			{
				LOG_VOXEL(Warning, TEXT("%s failed to compile: %s"), VoxelGraphGenerator ? *VoxelGraphGenerator->GetName() : TEXT(""), *Message.Message);
			}
		}
	}
}

void FVoxelGraphErrorReporter::CopyFrom(FVoxelGraphErrorReporter& Other)
{
	check(VoxelGraphGenerator == Other.VoxelGraphGenerator);

	bHasError |= Other.bHasError;
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
	for (auto Node : Graph->VoxelGraph->Nodes)
	{
		if (auto* Interface = Cast<UVoxelGraphNodeInterface>(Node))
		{
			for (auto Type : TEnumRange<EVoxelGraphNodeMessageType>())

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
					UVoxelGraphMacro* Macro = MacroNode->Macro;
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
	for (auto Type : TEnumRange<EVoxelGraphNodeMessageType>())
	{
		ClearMessages(Graph, false, Type);
		ClearNodesMessages(Graph, true, false, Type);
	}
}

void FVoxelGraphErrorReporter::AddMessageToNodeInternal(
	const UVoxelNode* Node,
	const FString& Message,
	EVoxelGraphNodeMessageType Severity)
{
#if WITH_EDITOR
	if (UVoxelGraphNodeInterface* GraphNode = Node->GraphNode)
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

