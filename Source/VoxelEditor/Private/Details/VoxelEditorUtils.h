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
#include "MessageDialog.h"
#include "PackageTools.h"

#include "AssetRegistryModule.h"
#include "SNotificationList.h"
#include "EditorViewportClient.h"
#include "NotificationManager.h"
#include "Editor.h"

class FVoxelEditorDetailsUtils
{
public:
	template<class T>
	static T* GetCurrentObjectFromDetails(IDetailLayoutBuilder& DetailLayout)
	{
#if ENGINE_MINOR_VERSION == 17
		const TArray< TWeakObjectPtr<UObject>>& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();
#else
		const TArray<TWeakObjectPtr<AActor>>& SelectedObjects = DetailLayout.GetDetailsView()->GetSelectedActors();
#endif
		for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
		{
#if ENGINE_MINOR_VERSION == 17
			const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
#else
			const TWeakObjectPtr<AActor>& CurrentObject = SelectedObjects[ObjectIndex];
#endif
			if (CurrentObject.IsValid())
			{
				T* CurrentCaptureActor = Cast<T>(CurrentObject.Get());
				if (CurrentCaptureActor != NULL)
				{
					// Enable realtime
					((FEditorViewportClient*)(GEditor->GetActiveViewport()->GetClient()))->SetRealtime(true);
					return CurrentCaptureActor;
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
