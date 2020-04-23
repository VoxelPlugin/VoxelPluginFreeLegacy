// Copyright 2020 Phyronnaz

#include "VoxelMessagesEditor.h"
#include "VoxelSettings.h"
#include "VoxelGlobals.h"

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
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Editor.h"

inline void AddButton(
	FNotificationInfo& Info,
	const FSimpleDelegate& OnClick,
	const FText& Text,
	const FText& Tooltip,
	const TSharedRef<TWeakPtr<SNotificationItem>>& PtrToPtr)
{
	const auto Callback = FSimpleDelegate::CreateLambda([=]()
	{
		OnClick.ExecuteIfBound();
		auto Ptr = PtrToPtr->Pin();
		if (Ptr.IsValid())
		{
			Ptr->SetFadeOutDuration(0);
			Ptr->Fadeout();
		}
	});
	
	Info.ButtonDetails.Add(FNotificationButtonInfo(
		Text,
		Tooltip,
		Callback,
		SNotificationItem::CS_None));
}

void FVoxelMessagesEditor::LogMessage(const TSharedRef<FTokenizedMessage>& Message, EVoxelShowNotification ShouldShow)
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
	TArray<TSharedPtr<IMessageToken>> ReversedTokens;
	if (BlueprintExceptionTracker.ScriptStack.Num() > 0)
	{
		const FFrame& StackFrame = *BlueprintExceptionTracker.ScriptStack.Last();
		UClass* ClassContainingCode = FKismetDebugUtilities::FindClassForNode(nullptr, StackFrame.Node);
		UBlueprint* BlueprintObj = (ClassContainingCode ? Cast<UBlueprint>(ClassContainingCode->ClassGeneratedBy) : NULL);
		if (BlueprintObj)
		{
			const int32 BreakpointOffset = StackFrame.Code - StackFrame.Node->Script.GetData() - 1;

			ReversedTokens.Add(FUObjectToken::Create(BlueprintObj, FText::FromString(BlueprintObj->GetName()))
				->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&Local::OnMessageLogLinkActivated))
			);
			ReversedTokens.Add(FTextToken::Create(VOXEL_LOCTEXT("Blueprint: ")));

			// NOTE: StackFrame.Node is not a blueprint node like you may think ("Node" has some legacy meaning)
			ReversedTokens.Add(FUObjectToken::Create(StackFrame.Node, StackFrame.Node->GetDisplayNameText())
				->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&Local::OnMessageLogLinkActivated))
			);
			ReversedTokens.Add(FTextToken::Create(VOXEL_LOCTEXT("Function: ")));

#if WITH_EDITORONLY_DATA // to protect access to GeneratedClass->DebugData
			UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(ClassContainingCode);
			if ((GeneratedClass != NULL) && GeneratedClass->DebugData.IsValid())
			{
				UEdGraphNode* BlueprintNode = GeneratedClass->DebugData.FindSourceNodeFromCodeLocation(StackFrame.Node, BreakpointOffset, true);
				// if instead, there is a node we can point to...
				if (BlueprintNode != NULL)
				{
					ReversedTokens.Add(FUObjectToken::Create(BlueprintNode->GetGraph(), FText::FromString(GetNameSafe(BlueprintNode->GetGraph())))
						->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&Local::OnMessageLogLinkActivated))
					);
					ReversedTokens.Add(FTextToken::Create(VOXEL_LOCTEXT("Graph: ")));

					ReversedTokens.Add(FUObjectToken::Create(BlueprintNode, BlueprintNode->GetNodeTitle(ENodeTitleType::ListView))
						->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&Local::OnMessageLogLinkActivated))
					);
					ReversedTokens.Add(FTextToken::Create(VOXEL_LOCTEXT("Node: ")));
				}
			}
#endif // WITH_EDITORONLY_DATA
		}
	}
	for (int32 Index = ReversedTokens.Num() - 1; Index >= 0; --Index)
	{
		Message->AddToken(ReversedTokens[Index].ToSharedRef());
	}

	if (GEditor->PlayWorld || GIsPlayInEditorWorld)
	{
		FMessageLog("PIE").AddMessage(Message);
	}
	else
	{
		FMessageLog("Voxel").AddMessage(Message);
		if (GetDefault<UVoxelSettings>()->bShowNotifications && ShouldShow == EVoxelShowNotification::Show)
		{
			struct FLastNotification
			{
				TWeakPtr<SNotificationItem> Ptr;
				FText Text;
				int32 Count;
			};
			static TArray<FLastNotification> LastNotifications;
			
			const FText Text = Message->ToText();
			LastNotifications.RemoveAll([](auto& Notification) { return !Notification.Ptr.IsValid(); });
			
			for (auto& LastNotification : LastNotifications)
			{
				auto LastNotificationPtr = LastNotification.Ptr.Pin();
				if (ensure(LastNotificationPtr.IsValid()) && LastNotification.Text.EqualToCaseIgnored(Text))
				{
					LastNotification.Text = Text;
					LastNotification.Count++;

					LastNotificationPtr->SetText(FText::Format(VOXEL_LOCTEXT("{0} (x{1})"), Text, FText::AsNumber(LastNotification.Count)));
					LastNotificationPtr->ExpireAndFadeout();

					return;
				}
			}

			FNotificationInfo Info = FNotificationInfo(Text);
			Info.CheckBoxState = ECheckBoxState::Unchecked;
			Info.ExpireDuration = 10;
	
			const TSharedRef<TWeakPtr<SNotificationItem>> PtrToPtr = MakeShared<TWeakPtr<SNotificationItem>>();
			AddButton(Info, {}, VOXEL_LOCTEXT("Close"), VOXEL_LOCTEXT("Close"), PtrToPtr);
			const auto Ptr = FSlateNotificationManager::Get().AddNotification(Info);
			*PtrToPtr = Ptr;

			LastNotifications.Emplace(FLastNotification{ Ptr, Text, 1 });
		}
	}
}

void FVoxelMessagesEditor::ShowNotification(
	uint64 UniqueId,
	const FText& Message,
	const FText& ButtonText,
	const FText& ButtonTooltip,
	const FSimpleDelegate& OnClick)
{
	struct FLastNotification
	{
		TWeakPtr<SNotificationItem> Ptr;
		uint64 UniqueId;
	};
	static TArray<FLastNotification> LastNotifications;

	LastNotifications.RemoveAll([](auto& Notification) { return !Notification.Ptr.IsValid(); });
	if (LastNotifications.FindByPredicate([&](auto& Notification) { return Notification.UniqueId == UniqueId; }))
	{
		return;
	}

	FNotificationInfo Info(Message);
	Info.CheckBoxState = ECheckBoxState::Unchecked;
	Info.ExpireDuration = 10;
	
	const TSharedRef<TWeakPtr<SNotificationItem>> PtrToPtr = MakeShared<TWeakPtr<SNotificationItem>>();
	AddButton(Info, OnClick, ButtonText, ButtonTooltip, PtrToPtr);
	AddButton(Info, {}, VOXEL_LOCTEXT("Close"), VOXEL_LOCTEXT("Close"), PtrToPtr);
	const auto Ptr = FSlateNotificationManager::Get().AddNotification(Info);
	*PtrToPtr = Ptr;

	LastNotifications.Add({ Ptr, UniqueId });
}