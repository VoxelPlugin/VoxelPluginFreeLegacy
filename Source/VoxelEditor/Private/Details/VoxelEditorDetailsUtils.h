// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"
#include "UnrealClient.h"
#include "EditorViewportClient.h"
#include "Editor.h"
#include "ObjectTools.h"
#include "AssetToolsModule.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "Voxel"

class FVoxelEditorDetailsUtils
{
public:
	static inline bool ShowWarning(const FText& Text)
	{
		return FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(LOCTEXT("EditorUtilsWarning", "Warning: {0} \nContinue?"), Text)) == EAppReturnType::Yes;
	}

	static inline void ShowError(const FText& Text)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("EditorUtilsError", "Error: {0}"), Text));
	}

	template<class T>
	static T* GetCurrentObjectFromDetails(IDetailLayoutBuilder& DetailLayout, bool bEnableRealtime = true)
	{
		const TArray<TWeakObjectPtr<UObject>>& SelectedObjects = DetailLayout.GetDetailsView()->GetSelectedObjects();
		for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
		{
			const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
			if (CurrentObject.IsValid())
			{
				T* CurrentCaptureObject = Cast<T>(CurrentObject.Get());
				if (CurrentCaptureObject != NULL)
				{
					if (bEnableRealtime)
					{
						FViewport* Viewport = GEditor->GetActiveViewport();
						if (Viewport)
						{
							FViewportClient* Client = Viewport->GetClient();
							if (Client)
							{
								((FEditorViewportClient*)Client)->SetRealtime(true);
							}
						}
					}
					return CurrentCaptureObject;
				}
			}
		}
		return nullptr;
	}

	template<class TAsset, class TAssetFactory, typename F>
	static void CreateAsset(const F& CreateAsset)
	{
		// Create the asset
		TAssetFactory* AssetFactory = NewObject<TAssetFactory>();
		TAsset* Asset = Cast<TAsset>(FAssetToolsModule::GetModule().Get().CreateAssetWithDialog(TAsset::StaticClass(), AssetFactory));
		if (!Asset) return;

		FNotificationInfo Info = FNotificationInfo(FText());
		Info.ExpireDuration = 10.f;
		if (CreateAsset(Asset))
		{
			Info.Text = FText::FromString(Asset->GetPathName() + " was successfully created");
			Info.CheckBoxState = ECheckBoxState::Checked;
		}
		else
		{
			Info.Text = FText::FromString(Asset->GetPathName() + " was NOT successfully created");
			Info.CheckBoxState = ECheckBoxState::Unchecked;
			ObjectTools::DeleteSingleObject(Asset, false);
		}
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	static TSharedRef<STextBlock> CreateText(const FText& Text)
	{
		return SNew(STextBlock)
			   .Font(IDetailLayoutBuilder::GetDetailFont())
			   .Text(Text);
	}

	static TSharedRef<SButton> CreateButton(
		const FText& Text,
		const FOnClicked& OnClicked,
		const TAttribute<bool>& IsEnabled = true)
	{
		return SNew(SButton)
			   .ContentPadding(2)
			   .VAlign(VAlign_Center)
			   .HAlign(HAlign_Center)
			   .OnClicked(OnClicked)
			   .IsEnabled(IsEnabled)
			   [
				   CreateText(Text)
			   ];
	}

	static void AddButtonToCategory(
		IDetailLayoutBuilder& DetailLayout,
		const FName& CategoryName,
		const FText& FilterString,
		const FText& TextLeftToButton,
		const FText& ButtonText,
		const FOnClicked& OnClicked,
		const TAttribute<bool>& IsEnabled = true)
	{
		DetailLayout.EditCategory(CategoryName)
		.AddCustomRow(FilterString)
		.NameContent()
		[
			CreateText(TextLeftToButton)
		]
		.ValueContent()
		.MinDesiredWidth(125.0f)
		.MaxDesiredWidth(125.0f)
		[
			CreateButton(ButtonText, OnClicked, IsEnabled)
		];
	}

	static void AddPropertyToCategory(
		IDetailLayoutBuilder& DetailLayout,
		const FName& CategoryName,
		const FName& PropertyName,
		UClass* Class = nullptr)
	{
		auto Property = DetailLayout.GetProperty(PropertyName, Class);
		DetailLayout.EditCategory(CategoryName).AddCustomRow(FText::FromString(PropertyName.ToString()))
		.NameContent()
		[
			Property->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(125.0f)
		.MaxDesiredWidth(600.0f)
		[
			Property->CreatePropertyValueWidget()
		];
	}
};

#define RETURN_IF_CANCEL() { if (Progress.ShouldCancel()) { FVoxelEditorDetailsUtils::ShowError(LOCTEXT("Canceled", "Canceled!"));  return false; } }

#undef LOCTEXT_NAMESPACE