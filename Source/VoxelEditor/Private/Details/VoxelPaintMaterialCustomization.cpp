// Copyright 2020 Phyronnaz

#include "Details/VoxelPaintMaterialCustomization.h"

#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"
#include "VoxelEditorDetailsUtilities.h"

#include "Materials/MaterialInterface.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "IDetailGroup.h"
#include "IDetailPropertyRow.h"
#include "Widgets/Input/SComboBox.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#define GET_CHILD_PROPERTY(PropertyHandle, Class, Property) PropertyHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(Class, Property)).ToSharedRef()

void FVoxelPaintMaterialCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FVoxelPaintMaterialCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const auto& PaintMaterialEnum = *StaticEnum<EVoxelPaintMaterialType>();
	const auto& MaterialConfigEnum = *StaticEnum<EVoxelMaterialConfig>();
	
	TypeHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Type);
	const auto RestrictTypeHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, bRestrictType);
	const auto MaterialConfigToRestrictToHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, MaterialConfigToRestrictTo);

	FString TypeString;
	FString MaterialConfigToRestrictToString;
	
	EVoxelPaintMaterialType Type;
	bool bRestrictType = false;
	EVoxelMaterialConfig MaterialConfigToRestrictTo;
	
	{
		if (!ensure(TypeHandle->GetValueAsFormattedString(TypeString) == FPropertyAccess::Success)) return;
		if (!ensure(RestrictTypeHandle->GetValue(bRestrictType) == FPropertyAccess::Success)) return;
		if (!ensure(MaterialConfigToRestrictToHandle->GetValueAsFormattedString(MaterialConfigToRestrictToString) == FPropertyAccess::Success)) return;

		const int64 TypeValue = PaintMaterialEnum.GetValueByNameString(TypeString);
		if (!ensure(TypeValue != -1)) return;
		Type = EVoxelPaintMaterialType(TypeValue);

		const int64 MaterialConfigValue = MaterialConfigEnum.GetValueByNameString(MaterialConfigToRestrictToString);
		if (!ensure(MaterialConfigValue != -1)) return;
		MaterialConfigToRestrictTo = EVoxelMaterialConfig(MaterialConfigValue);
	}
	
	TSharedPtr<SWidget> TypeWidget;
	if (bRestrictType)
	{
		OptionsSource.Reset();
		if (MaterialConfigToRestrictTo == EVoxelMaterialConfig::RGB)
		{
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::Color));
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::FiveWayBlend));
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::UV));
		}
		else if (MaterialConfigToRestrictTo == EVoxelMaterialConfig::SingleIndex)
		{
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::Color));
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::FiveWayBlend));
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::SingleIndex));
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::UV));
		}
		else
		{
			ensure(MaterialConfigToRestrictTo == EVoxelMaterialConfig::MultiIndex);
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::MultiIndex));
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::MultiIndexWetness));
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::MultiIndexRaw));
			OptionsSource.Add(MakeShared<EVoxelPaintMaterialType>(EVoxelPaintMaterialType::UV));
		}

		const auto SearchTypePredicate = [&](auto& Ptr) { return *Ptr == Type; };
		
		if (!OptionsSource.ContainsByPredicate(SearchTypePredicate))
		{
			switch (MaterialConfigToRestrictTo)
			{
			case EVoxelMaterialConfig::RGB: Type = EVoxelPaintMaterialType::FiveWayBlend; break;
			case EVoxelMaterialConfig::SingleIndex: Type = EVoxelPaintMaterialType::SingleIndex; break;
			case EVoxelMaterialConfig::MultiIndex: Type = EVoxelPaintMaterialType::MultiIndex; break;
			default: ensure(false);
			}
			
			TypeHandle->SetValueFromFormattedString(PaintMaterialEnum.GetNameStringByValue(int64(Type)));
		}
		
		auto* TypeOptionSourcePtr = OptionsSource.FindByPredicate(SearchTypePredicate);
		check(TypeOptionSourcePtr);

		ComboBoxText = FVoxelEditorUtilities::CreateText(PaintMaterialEnum.GetDisplayNameTextByValue(int64(Type)));
		
		TypeWidget = SNew(SComboBox<TSharedPtr<EVoxelPaintMaterialType>>)
		.IsEnabled_Lambda([TypeHandle = TWeakPtr<IPropertyHandle>(TypeHandle)](){ return TypeHandle.IsValid() && !TypeHandle.Pin()->IsEditConst(); })
		.OptionsSource(&OptionsSource)
		.OnSelectionChanged(this, &FVoxelPaintMaterialCustomization::HandleComboBoxSelectionChanged)
		.OnGenerateWidget_Lambda([&](TSharedPtr<EVoxelPaintMaterialType> InValue)
		{
			return FVoxelEditorUtilities::CreateText(PaintMaterialEnum.GetDisplayNameTextByValue(int64(*InValue)));
		})
		.InitiallySelectedItem(*TypeOptionSourcePtr)
		[
			ComboBoxText.ToSharedRef()
		];
	}
	else
	{
		TypeWidget = TypeHandle->CreatePropertyValueWidget();
	}

	// Make sure to do that after possibly editing the type
	const FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([PropertyUtilities = MakeWeakPtr(CustomizationUtils.GetPropertyUtilities())]()
	{
		if (PropertyUtilities.IsValid())
		{
			PropertyUtilities.Pin()->ForceRefresh();
		}
	});
	TypeHandle->SetOnPropertyValueChanged(RefreshDelegate);

	const bool bShowOnlyInnerProperties = PropertyHandle->HasMetaData(STATIC_FNAME("ShowOnlyInnerProperties"));

	IDetailGroup* Group = nullptr;
	if (bShowOnlyInnerProperties)
	{
		ChildBuilder.AddCustomRow(VOXEL_LOCTEXT("Type"))
		.NameContent()
		[
			TypeHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			TypeWidget.ToSharedRef()
		];
	}
	else
	{
		Group = &ChildBuilder.AddGroup(TEXT("Paint Material Type"), PropertyHandle->GetPropertyDisplayName());
		Group->HeaderRow()
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			TypeWidget.ToSharedRef()
		];
	}
	
	const auto AddProperty = [&](const auto& InPropertyHandle) -> IDetailPropertyRow&
	{
		if (bShowOnlyInnerProperties)
		{
			return ChildBuilder.AddProperty(InPropertyHandle);
		}
		else
		{
			return Group->AddPropertyRow(InPropertyHandle);
		}
	};

	if (Type == EVoxelPaintMaterialType::Color)
	{
		const auto ColorHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, Color);
		const auto UseLinearColorHandle = GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bUseLinearColor);

		UseLinearColorHandle->SetOnPropertyValueChanged(RefreshDelegate);
		
		auto& UseLinearColor = AddProperty(UseLinearColorHandle);

		bool bUseLinearColor = false;
		if (UseLinearColorHandle->GetValue(bUseLinearColor) == FPropertyAccess::Success)
		{
			if (bUseLinearColor)
			{
				auto& LinearColor = AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, LinearColor));
			}
			else
			{
				auto& Color = AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, Color));
			}
		}
		else
		{
			auto& LinearColor = AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, LinearColor));
			auto& Color = AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, Color));
		}
		
		auto& PaintR = AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintR));
		auto& PaintG = AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintG));
		auto& PaintB = AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintB));
		auto& PaintA = AddProperty(GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintA));

		if (bRestrictType && MaterialConfigToRestrictTo == EVoxelMaterialConfig::SingleIndex)
		{
			PaintA.IsEnabled(false);
			PaintA.ToolTip(VOXEL_LOCTEXT("Disabled in Single Index, as it's used to store the index"));
			GET_CHILD_PROPERTY(ColorHandle, FVoxelPaintMaterialColor, bPaintA)->SetValue(false);
		}
	}
	else if (Type == EVoxelPaintMaterialType::FiveWayBlend)
	{
		const auto FiveWayBlendHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, FiveWayBlend);
		
		auto& Channel = AddProperty(GET_CHILD_PROPERTY(FiveWayBlendHandle, FVoxelPaintMaterialFiveWayBlend, Channel));
		auto& TargetValue = AddProperty(GET_CHILD_PROPERTY(FiveWayBlendHandle, FVoxelPaintMaterialFiveWayBlend, TargetValue));
		auto& LockedChannels = AddProperty(GET_CHILD_PROPERTY(FiveWayBlendHandle, FVoxelPaintMaterialFiveWayBlend, LockedChannels));
		auto& FourWayBlend = AddProperty(GET_CHILD_PROPERTY(FiveWayBlendHandle, FVoxelPaintMaterialFiveWayBlend, bFourWayBlend));

		if (bRestrictType && MaterialConfigToRestrictTo == EVoxelMaterialConfig::SingleIndex)
		{
			FourWayBlend.IsEnabled(false);
			FourWayBlend.ToolTip(VOXEL_LOCTEXT("Always enabled in Single Index, as alpha is used to store the index"));
			GET_CHILD_PROPERTY(FiveWayBlendHandle, FVoxelPaintMaterialFiveWayBlend, bFourWayBlend)->SetValue(true);
		}
	}
	else if (Type == EVoxelPaintMaterialType::SingleIndex)
	{
		const auto SingleIndexHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, SingleIndex);
		
		auto& Channel = AddProperty(GET_CHILD_PROPERTY(SingleIndexHandle, FVoxelPaintMaterialSingleIndex, Channel));
	}
	else if (Type == EVoxelPaintMaterialType::MultiIndex)
	{
		const auto MultiIndexHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, MultiIndex);

		auto& Channel = AddProperty(GET_CHILD_PROPERTY(MultiIndexHandle, FVoxelPaintMaterialMultiIndex, Channel));
		auto& TargetValue = AddProperty(GET_CHILD_PROPERTY(MultiIndexHandle, FVoxelPaintMaterialMultiIndex, TargetValue));
		auto& LockedChannels = AddProperty(GET_CHILD_PROPERTY(MultiIndexHandle, FVoxelPaintMaterialMultiIndex, LockedChannels));
	}
	else if (Type == EVoxelPaintMaterialType::MultiIndexWetness)
	{
		const auto MultiIndexWetnessHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, MultiIndexWetness);
		
		auto& TargetValue = AddProperty(GET_CHILD_PROPERTY(MultiIndexWetnessHandle, FVoxelPaintMaterialMultiIndexWetness, TargetValue));
	}
	else if (Type == EVoxelPaintMaterialType::MultiIndexRaw)
	{
		const auto MultiIndexRawHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, MultiIndexRaw);
		
		auto& Channel0 = AddProperty(GET_CHILD_PROPERTY(MultiIndexRawHandle, FVoxelPaintMaterialMultiIndexRaw, Channel0));
		auto& Strength0 = AddProperty(GET_CHILD_PROPERTY(MultiIndexRawHandle, FVoxelPaintMaterialMultiIndexRaw, Strength0));
		auto& Channel1 = AddProperty(GET_CHILD_PROPERTY(MultiIndexRawHandle, FVoxelPaintMaterialMultiIndexRaw, Channel1));
		auto& Strength1 = AddProperty(GET_CHILD_PROPERTY(MultiIndexRawHandle, FVoxelPaintMaterialMultiIndexRaw, Strength1));
		auto& Channel2 = AddProperty(GET_CHILD_PROPERTY(MultiIndexRawHandle, FVoxelPaintMaterialMultiIndexRaw, Channel2));
		auto& Strength2 = AddProperty(GET_CHILD_PROPERTY(MultiIndexRawHandle, FVoxelPaintMaterialMultiIndexRaw, Strength2));
		auto& Channel3 = AddProperty(GET_CHILD_PROPERTY(MultiIndexRawHandle, FVoxelPaintMaterialMultiIndexRaw, Channel3));
		auto& Strength3 = AddProperty(GET_CHILD_PROPERTY(MultiIndexRawHandle, FVoxelPaintMaterialMultiIndexRaw, Strength3));
	}
	else if (Type == EVoxelPaintMaterialType::UV)
	{
		const auto UVHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial, UV);
		auto& Channel = AddProperty(GET_CHILD_PROPERTY(UVHandle, FVoxelPaintMaterialUV, Channel));
		auto& UV = AddProperty(GET_CHILD_PROPERTY(UVHandle, FVoxelPaintMaterialUV, UV));
		auto& PaintU = AddProperty(GET_CHILD_PROPERTY(UVHandle, FVoxelPaintMaterialUV, bPaintU));
		auto& PaintV = AddProperty(GET_CHILD_PROPERTY(UVHandle, FVoxelPaintMaterialUV, bPaintV));

		if (bRestrictType && MaterialConfigToRestrictTo == EVoxelMaterialConfig::MultiIndex)
		{
			Channel.ToolTip(VOXEL_LOCTEXT("In multi index, the first 2 UV channels are used to store the indices"));
			const auto Handle = GET_CHILD_PROPERTY(UVHandle, FVoxelPaintMaterialUV, Channel);
			const auto FixupChannel = FSimpleDelegate::CreateLambda([=]()
			{
				int32 Value = 0;
				if (!ensure(Handle->GetValue(Value) == FPropertyAccess::Success))
				{
					return;
				}
				if (Value < 2)
				{
					Handle->SetValue(2);
				}
			});
			FixupChannel.Execute();
			Handle->SetOnPropertyValueChanged(FixupChannel);
		}
	}
	else
	{
		ensure(false);
	}
}

void FVoxelPaintMaterialCustomization::HandleComboBoxSelectionChanged(TSharedPtr<EVoxelPaintMaterialType> NewSelection, ESelectInfo::Type SelectInfo) const
{
	if (ensure(NewSelection.IsValid()) && ensure(TypeHandle.IsValid()) && ensure(ComboBoxText.IsValid()))
	{
		const auto& Enum = *StaticEnum<EVoxelPaintMaterialType>();
		const int64 Value = int64(*NewSelection);
		TypeHandle->SetValueFromFormattedString(Enum.GetNameStringByValue(Value));
		ComboBoxText->SetText(Enum.GetDisplayNameTextByValue(Value));
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelPaintMaterial_MaterialCollectionChannelCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> CollectionHandle;
	{
		auto ParentHandle = PropertyHandle;
		while (!CollectionHandle.IsValid() && ensure(ParentHandle->GetParentHandle().IsValid()))
		{
			ParentHandle = ParentHandle->GetParentHandle().ToSharedRef();
			CollectionHandle = ParentHandle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelPaintMaterial, PreviewMaterialCollection));
		}
	}
	if (!ensure(CollectionHandle.IsValid()))
	{
		return;
	}

	const auto ChannelHandle = GET_CHILD_PROPERTY(PropertyHandle, FVoxelPaintMaterial_MaterialCollectionChannel, Channel);
	
	const auto Thumbnail = MakeShared<FAssetThumbnail>(nullptr, 64, 64, CustomizationUtils.GetThumbnailPool());

	const auto SelectedMaterial = MakeShared<UVoxelMaterialCollectionBase::FMaterialInfo>();
	const auto AssetsToMaterials = MakeShared<TMap<TWeakObjectPtr<UObject>, UVoxelMaterialCollectionBase::FMaterialInfo>>();
	const auto IndicesToMaterials = MakeShared<TMap<uint8, UVoxelMaterialCollectionBase::FMaterialInfo>>();
	
	const auto OnChanged = FSimpleDelegate::CreateLambda([=]()
	{
		UObject* Collection;
		CollectionHandle->GetValue(Collection);
		UVoxelMaterialCollectionBase* MaterialCollection = Cast<UVoxelMaterialCollectionBase>(Collection);
		if (!MaterialCollection)
		{
			return;
		}

		AssetsToMaterials->Reset();
		for (auto& MaterialInfo : MaterialCollection->GetMaterials())
		{
			if (MaterialInfo.Material.IsValid() && MaterialInfo.Name.IsNone())
			{
				// Fixup name if needed
				MaterialInfo.Name = MaterialInfo.Material->GetFName();
			}
			AssetsToMaterials->Add(MaterialInfo.Material, MaterialInfo);
			IndicesToMaterials->Add(MaterialInfo.Index, MaterialInfo);
		}
		
		void* Data = nullptr;
		if (PropertyHandle->GetValueData(Data) != FPropertyAccess::Success)
		{
			return;
		}
		
		auto& Channel = *static_cast<FVoxelPaintMaterial_MaterialCollectionChannel*>(Data);
		*SelectedMaterial = IndicesToMaterials->FindRef(Channel);
		Thumbnail->SetAsset(SelectedMaterial->Material.Get());
	});
	OnChanged.Execute();

	const auto OnClose = MakeShared<FSimpleDelegate>();
	
	const auto AssetComboButton = SNew(SComboButton)
		.ButtonStyle( FEditorStyle::Get(), "PropertyEditor.AssetComboStyle" )
		.ForegroundColor(FEditorStyle::GetColor("PropertyEditor.AssetName.ColorAndOpacity"))
		.OnGetMenuContent_Lambda([=]()
		{
			FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

			FSetARFilterDelegate FilterDelegate;

			FAssetPickerConfig PickerConfig;
			PickerConfig.SelectionMode = ESelectionMode::Single;
			PickerConfig.bAllowDragging = false;
			PickerConfig.bAllowNullSelection = false;
			PickerConfig.InitialAssetViewType = EAssetViewType::Tile;
			PickerConfig.InitialAssetSelection = SelectedMaterial->Material.Get();
			PickerConfig.Filter.ObjectPaths.Add("FAKE"); // Remove all real results, we add our own assets below
			PickerConfig.OnGetCustomSourceAssets = FOnGetCustomSourceAssets::CreateLambda([=](const FARFilter& SourceFilter, TArray<FAssetData>& AddedAssets)
			{
				for (auto& It : *AssetsToMaterials)
				{
					if (It.Key.IsValid())
					{
						FAssetData AssetData(It.Key.Get());
						AssetData.AssetName = It.Value.Name;
						AddedAssets.Add(AssetData);
					}
				}
			});
			
			PickerConfig.OnAssetSelected.BindLambda([=](const FAssetData& AssetData)
			{
				auto* NewAsset = AssetData.GetAsset();
				if (auto* Info = AssetsToMaterials->Find(NewAsset))
				{
					ChannelHandle->SetValue(Info->Index);
					OnClose->ExecuteIfBound();
				}

			});
			return
				SNew(SBox)
				.WidthOverride(300.f)
				.HeightOverride(300.f)
				[
					ContentBrowserModule.Get().CreateAssetPicker(PickerConfig)
				];
		})
		.IsEnabled(ChannelHandle, &IPropertyHandle::IsEditable)
		.ContentPadding(2.0f)
		.ButtonContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(1)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle( FEditorStyle::Get(), "PropertyEditor.AssetClass" )
				.Font( FEditorStyle::GetFontStyle( "PropertyWindow.NormalFont" ) )
				.Text_Lambda([=]()
				{
					return FText::FromName(SelectedMaterial->Name);
				})
			]
		];

	*OnClose = FSimpleDelegate::CreateLambda([=]() { AssetComboButton->SetIsOpen(false); });
	
	ChannelHandle->SetOnPropertyValueChanged(OnChanged);
	
	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(140.f)
	.MaxDesiredWidth(140.f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(4.f, 0.f, 4.f, 0.f)
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(64.f)
			.HeightOverride(64.f)
			[
				Thumbnail->MakeThumbnailWidget()
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(4.f, 0.f, 4.f, 0.f)
		.AutoWidth()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				AssetComboButton
			]
			+ SVerticalBox::Slot()
			.Padding(0.f, 4.f, 0.f, 0.f)
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(40.f)
				[
					ChannelHandle->CreatePropertyValueWidget()
				]
			]
		]
	];
}

#undef GET_CHILD_PROPERTY