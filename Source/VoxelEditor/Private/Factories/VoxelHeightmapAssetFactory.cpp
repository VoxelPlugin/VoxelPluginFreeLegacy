// Copyright 2020 Phyronnaz

#include "Factories/VoxelHeightmapAssetFactory.h"
#include "VoxelEditorDetailsUtilities.h"
#include "VoxelMessages.h"

#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"

#include "Editor.h"
#include "EditorStyleSet.h"
#include "PropertyEditorModule.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"

#include "Modules/ModuleManager.h"
#include "Misc/ScopedSlowTask.h"

#include "LandscapeEditorModule.h"
#include "LandscapeComponent.h"
#include "LandscapeDataAccess.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelHeightmapAssetFloatFactory::UVoxelHeightmapAssetFloatFactory()
{
	bCreateNew = false;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = UVoxelHeightmapAssetFloat::StaticClass();
}

UObject* UVoxelHeightmapAssetFloatFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	FVoxelMessages::ShowVoxelPluginProError("Importing Landscapes requires Voxel Plugin Pro");
	return nullptr;
}

FString UVoxelHeightmapAssetFloatFactory::GetDefaultNewAssetName() const
{
	return AssetName;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelHeightmapAssetUINT16Factory::UVoxelHeightmapAssetUINT16Factory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = UVoxelHeightmapAssetUINT16::StaticClass();
}

bool UVoxelHeightmapAssetUINT16Factory::ConfigureProperties()
{
	// Load from default
	Heightmap = GetDefault<UVoxelHeightmapAssetUINT16Factory>()->Heightmap;
	MaterialConfig = GetDefault<UVoxelHeightmapAssetUINT16Factory>()->MaterialConfig;
	WeightmapsInfos = GetDefault<UVoxelHeightmapAssetUINT16Factory>()->WeightmapsInfos;

	TSharedRef<SWindow> PickerWindow = SNew(SWindow)
		.Title(VOXEL_LOCTEXT("Import Heightmap"))
		.SizingRule(ESizingRule::Autosized);

	bool bSuccess = false;

	auto OnOkClicked = FOnClicked::CreateLambda([&]() 
	{
		if (TryLoad())
		{
			bSuccess = true;
			PickerWindow->RequestDestroyWindow();
		}
		return FReply::Handled();
	});
	auto OnCancelClicked = FOnClicked::CreateLambda([&]() 
	{
		bSuccess = false;
		PickerWindow->RequestDestroyWindow();
		return FReply::Handled();
	});

	class FVoxelHeightmapFactoryDetails : public IDetailCustomization
	{
	public:
		static TSharedRef<IDetailCustomization> MakeInstance()
		{
			return MakeShared<FVoxelHeightmapFactoryDetails>();
		}

		FVoxelHeightmapFactoryDetails() = default;

	private:
		virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
		{
			FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&DetailLayout]()
			{
				DetailLayout.ForceRefreshDetails();
			});
			DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(UVoxelHeightmapAssetUINT16Factory, MaterialConfig))->SetOnPropertyValueChanged(RefreshDelegate);
		}
	};

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea);

	auto DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	FOnGetDetailCustomizationInstance LayoutDelegateDetails = FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelHeightmapFactoryDetails::MakeInstance);
	DetailsPanel->RegisterInstancedCustomPropertyLayout(UVoxelHeightmapAssetUINT16Factory::StaticClass(), LayoutDelegateDetails);
	DetailsPanel->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateLambda([&](const FPropertyAndParent& Property)
	{
		FName Name = Property.Property.GetFName();
		if (Name == GET_MEMBER_NAME_STATIC(FVoxelHeightmapImporterWeightmapInfos, Layer))
		{
			return MaterialConfig == EVoxelMaterialConfig::RGB;
		}
		else if (Name == GET_MEMBER_NAME_STATIC(FVoxelHeightmapImporterWeightmapInfos, Index))
		{
			return MaterialConfig == EVoxelMaterialConfig::SingleIndex || MaterialConfig == EVoxelMaterialConfig::DoubleIndex;
		}
		else
		{
			return true;
		}
	}));
	DetailsPanel->SetObject(this);

	auto Widget =
		SNew(SBorder)
		.Visibility(EVisibility::Visible)
		.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
		[
			SNew(SBox)
			.Visibility(EVisibility::Visible)
			.WidthOverride(520.0f)
		[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				.MaxHeight(500)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						DetailsPanel
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				.Padding(8)
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(VOXEL_LOCTEXT("Create"))
						.HAlign(HAlign_Center)
						.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda([&]()
						{
							if (Heightmap.FilePath.IsEmpty())
							{
								return EVisibility::Hidden;
							}
							for (auto& Weightmap : WeightmapsInfos)
							{
								if (Weightmap.File.FilePath.IsEmpty())
								{
									return EVisibility::Hidden;
								}
							}
							return EVisibility::Visible;
						})))
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(OnOkClicked)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
					]
					+SUniformGridPanel::Slot(1,0)
					[
						SNew(SButton)
						.Text(VOXEL_LOCTEXT("Cancel"))
						.HAlign(HAlign_Center)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(OnCancelClicked)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
					]
				]
			]
		];

	PickerWindow->SetContent(Widget);

	GEditor->EditorAddModalWindow(PickerWindow);

	// Save to default
	GetMutableDefault<UVoxelHeightmapAssetUINT16Factory>()->Heightmap = Heightmap;
	GetMutableDefault<UVoxelHeightmapAssetUINT16Factory>()->MaterialConfig = MaterialConfig;
	GetMutableDefault<UVoxelHeightmapAssetUINT16Factory>()->WeightmapsInfos = WeightmapsInfos;

	return bSuccess;
}

UObject* UVoxelHeightmapAssetUINT16Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto* Asset = NewObject<UVoxelHeightmapAssetUINT16>(InParent, Class, Name, Flags | RF_Transactional);
	if(DoImport(Asset))
	{
		return Asset;
	}
	else
	{
		return nullptr;
	}
}

FString UVoxelHeightmapAssetUINT16Factory::GetDefaultNewAssetName() const
{
	return FPaths::GetBaseFilename(Heightmap.FilePath);
}

bool UVoxelHeightmapAssetUINT16Factory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (auto* Asset = Cast<UVoxelHeightmapAssetUINT16>(Obj))
	{
		OutFilenames.Add(Asset->Heightmap);
		for (auto& Weightmap : Asset->WeightmapsInfos)
		{
			OutFilenames.Add(Weightmap.File.FilePath);
		}
		return true;
	}
	else
	{
		return false;
	}
}

void UVoxelHeightmapAssetUINT16Factory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	if (auto* Asset = Cast<UVoxelHeightmapAssetUINT16>(Obj))
	{
		for (int32 Index = 0; Index < NewReimportPaths.Num(); Index++)
		{
			if (Index == 0)
			{
				Asset->Heightmap = NewReimportPaths[0];
			}
			else if (ensure(Index - 1 < Asset->WeightmapsInfos.Num()))
			{
				Asset->WeightmapsInfos[Index - 1].File.FilePath = NewReimportPaths[Index];
			}
		}
	}
}

EReimportResult::Type UVoxelHeightmapAssetUINT16Factory::Reimport(UObject* Obj)
{
	if (auto* Asset = Cast<UVoxelHeightmapAssetUINT16>(Obj))
	{
		Heightmap.FilePath = Asset->Heightmap;
		MaterialConfig = Asset->MaterialConfig;
		WeightmapsInfos = Asset->WeightmapsInfos;
		if (!TryLoad())
		{
			return EReimportResult::Failed;
		}
		return DoImport(Asset) ? EReimportResult::Succeeded : EReimportResult::Cancelled;
	}
	else
	{
		return EReimportResult::Failed;
	}
}

int32 UVoxelHeightmapAssetUINT16Factory::GetPriority() const
{
	return ImportPriority;
}

bool UVoxelHeightmapAssetUINT16Factory::TryLoad()
{
	FVoxelMessages::ShowVoxelPluginProError("Importing Heightmaps requires Voxel Plugin Pro");
	return false;
}

bool UVoxelHeightmapAssetUINT16Factory::DoImport(UVoxelHeightmapAssetUINT16* Asset)
{
	FVoxelMessages::ShowVoxelPluginProError("Importing Heightmaps requires Voxel Plugin Pro");
	return false;
}