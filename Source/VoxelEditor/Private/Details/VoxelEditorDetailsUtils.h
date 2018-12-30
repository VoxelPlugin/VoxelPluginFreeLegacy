// Copyright 2018 Phyronnaz

#pragma once

#include "Runtime/Launch/Resources/Version.h"

#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Misc/MessageDialog.h"
#include "PackageTools.h"

#include "AssetRegistryModule.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "EditorViewportClient.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Editor.h"
#include "ObjectTools.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "VoxelEditorDetailsUtils"

class FVoxelEditorDetailsUtils
{
public:
	static inline bool ShowWarning(const FText& Text)
	{
		return FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(LOCTEXT("Warning", "Warning: {0} \nContinue?"), Text)) == EAppReturnType::Yes;
	}

	static inline void ShowError(const FText& Text)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("Error", "Error: {0}"), Text));
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

};

#define ADD_BUTTON_TO_CATEGORY(DetailLayout, CategoryName, NewRowFilterString, TextLeftToButton, ButtonText, ObjectPtr, FunctionPtr) \
	{ \
		DetailLayout.EditCategory((CategoryName)) \
			.AddCustomRow((NewRowFilterString)) \
			.NameContent() \
			[ \
				SNew(STextBlock) \
				.Font(IDetailLayoutBuilder::GetDetailFont()) \
				.Text((TextLeftToButton)) \
			] \
			.ValueContent() \
			.MaxDesiredWidth(125.f) \
			.MinDesiredWidth(125.f) \
			[ \
				SNew(SButton) \
				.ContentPadding(2) \
				.VAlign(VAlign_Center) \
				.HAlign(HAlign_Center) \
				.OnClicked((ObjectPtr), (FunctionPtr)) \
				[ \
					SNew(STextBlock) \
					.Font(IDetailLayoutBuilder::GetDetailFont()) \
					.Text((ButtonText)) \
				] \
			]; \
	}

#define ADD_PROPERTY_TO_CATEGORY(CategoryName, PropertyName, Class) \
DetailLayout.EditCategory(CategoryName).AddCustomRow(LOCTEXT(PropertyName, PropertyName)) \
.NameContent() \
[ \
	DetailLayout.GetProperty(PropertyName, Class::StaticClass())->CreatePropertyNameWidget() \
] \
.ValueContent() \
[ \
	DetailLayout.GetProperty(PropertyName, Class::StaticClass())->CreatePropertyValueWidget() \
];

#define RETURN_IF_CANCEL() { if (Progress.ShouldCancel()) { FVoxelEditorDetailsUtils::ShowError(LOCTEXT("Canceled", "Canceled!"));  return false; } }

#undef LOCTEXT_NAMESPACE