// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SVoxelEditor.h"

#include "Fonts/SlateFontInfo.h"
#include "Internationalization/Text.h"
#include "VoxelAsset.h"
#include "UObject/Class.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

#include "VoxelEditorSettings.h"


#define LOCTEXT_NAMESPACE "SVoxelEditor"


/* SVoxelEditor interface
 *****************************************************************************/

SVoxelEditor::~SVoxelEditor()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}


void SVoxelEditor::Construct(const FArguments& InArgs, AVoxelAsset* InVoxel, const TSharedRef<ISlateStyle>& InStyle)
{
	Voxel = InVoxel;

	auto Settings = GetDefault<UVoxelEditorSettings>();

	/*ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(EditableTextBox, SMultiLineEditableTextBox)
					.BackgroundColor((Settings != nullptr) ? Settings->BackgroundColor : FLinearColor::White)
					.Font((Settings != nullptr) ? Settings->Font : FSlateFontInfo())
					.ForegroundColor((Settings != nullptr) ? Settings->ForegroundColor : FLinearColor::Black)
					.Margin((Settings != nullptr) ? Settings->Margin : 4.0f)
					.OnTextChanged(this, &SVoxelEditor::HandleEditableTextBoxTextChanged)
					.OnTextCommitted(this, &SVoxelEditor::HandleEditableTextBoxTextCommitted)
					.Text(Voxel->Text)
			]
	];*/

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(this, &SVoxelEditor::HandleVoxelPropertyChanged);
}


/* SVoxelEditor callbacks
 *****************************************************************************/

void SVoxelEditor::HandleEditableTextBoxTextChanged(const FText& NewText)
{
	Voxel->MarkPackageDirty();
}


void SVoxelEditor::HandleEditableTextBoxTextCommitted(const FText& Comment, ETextCommit::Type CommitType)
{
	//Voxel->Text = EditableTextBox->GetText();
}


void SVoxelEditor::HandleVoxelPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	/*if (Object == Voxel)
	{
		EditableTextBox->SetText(Voxel->Text);
	}*/
}


#undef LOCTEXT_NAMESPACE
