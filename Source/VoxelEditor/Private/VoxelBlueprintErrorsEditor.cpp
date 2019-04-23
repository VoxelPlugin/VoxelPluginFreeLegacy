// Copyright 2019 Phyronnaz

#include "VoxelBlueprintErrorsEditor.h"
#include "Logging/MessageLog.h"
#include "Misc/CoreMisc.h"
#include "Misc/UObjectToken.h"
#include "UObject/Stack.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Kismet2/KismetDebugUtilities.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "Voxel"
void FVoxelBlueprintErrorsEditor::LogError(const TSharedRef<FTokenizedMessage>& Message)
{
	struct Local
	{
		static void OnMessageLogLinkActivated(const class TSharedRef<IMessageToken>& Token)
		{
			if (Token->GetType() == EMessageToken::Object)
			{
				const TSharedRef<FUObjectToken> UObjectToken = StaticCastSharedRef<FUObjectToken>(Token);
				if (UObjectToken->GetObject().IsValid())
				{
					FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(UObjectToken->GetObject().Get());
				}
			}
		}
	};
	
	FBlueprintExceptionTracker& BlueprintExceptionTracker = FBlueprintExceptionTracker::Get();
	if (GIsEditor && GIsPlayInEditorWorld && BlueprintExceptionTracker.ScriptStack.Num() > 0)
	{
		const FFrame& StackFrame = *BlueprintExceptionTracker.ScriptStack.Last();
		UClass* ClassContainingCode = FKismetDebugUtilities::FindClassForNode(nullptr, StackFrame.Node);
		UBlueprint* BlueprintObj = (ClassContainingCode ? Cast<UBlueprint>(ClassContainingCode->ClassGeneratedBy) : NULL);
		if (BlueprintObj)
		{
			bool bResetObjectBeingDebuggedWhenFinished = false;
			UObject* ObjectBeingDebugged = BlueprintObj->GetObjectBeingDebugged();
			UObject* SavedObjectBeingDebugged = ObjectBeingDebugged;
			UWorld* WorldBeingDebugged = BlueprintObj->GetWorldBeingDebugged();

			const int32 BreakpointOffset = StackFrame.Code - StackFrame.Node->Script.GetData() - 1;

			Message->AddToken(FTextToken::Create(LOCTEXT("RuntimeErrorBlueprintObjectLabel", "Blueprint: ")));
			Message->AddToken(FUObjectToken::Create(BlueprintObj, FText::FromString(BlueprintObj->GetName()))
				->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&Local::OnMessageLogLinkActivated))
			);

			// NOTE: StackFrame.Node is not a blueprint node like you may think ("Node" has some legacy meaning)
			Message->AddToken(FTextToken::Create(LOCTEXT("RuntimeErrorBlueprintFunctionLabel", "Function: ")));
			Message->AddToken(FUObjectToken::Create(StackFrame.Node, StackFrame.Node->GetDisplayNameText())
				->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&Local::OnMessageLogLinkActivated))
			);

#if WITH_EDITORONLY_DATA // to protect access to GeneratedClass->DebugData
			UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(ClassContainingCode);
			if ((GeneratedClass != NULL) && GeneratedClass->DebugData.IsValid())
			{
				UEdGraphNode* BlueprintNode = GeneratedClass->DebugData.FindSourceNodeFromCodeLocation(StackFrame.Node, BreakpointOffset, true);
				// if instead, there is a node we can point to...
				if (BlueprintNode != NULL)
				{
					Message->AddToken(FTextToken::Create(LOCTEXT("RuntimeErrorBlueprintGraphLabel", "Graph: ")));
					Message->AddToken(FUObjectToken::Create(BlueprintNode->GetGraph(), FText::FromString(GetNameSafe(BlueprintNode->GetGraph())))
						->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&Local::OnMessageLogLinkActivated))
					);

					Message->AddToken(FTextToken::Create(LOCTEXT("RuntimeErrorBlueprintNodeLabel", "Node: ")));
					Message->AddToken(FUObjectToken::Create(BlueprintNode, BlueprintNode->GetNodeTitle(ENodeTitleType::ListView))
						->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&Local::OnMessageLogLinkActivated))
					);
				}
			}
#endif // WITH_EDITORONLY_DATA
		}
	}
	FMessageLog("PIE").AddMessage(Message);
}
#undef LOCTEXT_NAMESPACE