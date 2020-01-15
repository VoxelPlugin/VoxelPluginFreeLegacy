// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Framework/SlateDelegates.h"

class STextBlock;
class SButton;
class UFactory;
class IDetailLayoutBuilder;

namespace FVoxelEditorUtilities
{
	bool ShowWarning(const FText& Text);
	void ShowError(const FText& Text);
	void EnableRealtime();

	TSharedRef<STextBlock> CreateText(const FText& Text, TAttribute<FSlateColor> ColorAndOpacity = {});

	TSharedRef<SButton> CreateButton(
		const FText& Text,
		const FOnClicked& OnClicked,
		const TAttribute<bool>& IsEnabled = true);

	void AddButtonToCategory(
		IDetailLayoutBuilder& DetailLayout,
		const FName& CategoryName,
		const FText& FilterString,
		const FText& TextLeftToButton,
		const FText& ButtonText,
		bool bForAdvanced,
		const FOnClicked& OnClicked,
		const TAttribute<bool>& IsEnabled = true);

	void AddPropertyToCategory(
		IDetailLayoutBuilder& DetailLayout,
		const FName& CategoryName,
		const FName& PropertyName,
		bool bForAdvanced,
		UClass* Class = nullptr);

	// Same as engine one, but don't allow replacing assets
	UObject* CreateAssetWithDialog(
		const FString& AssetName,
		const FString& PackagePath,
		UClass* AssetClass,
		UFactory* Factory,
		FName CallingContext);
	
	// Same as engine one, but don't allow replacing assets
	UObject* CreateAssetWithDialog(
		UClass* AssetClass, 
		UFactory* Factory, 
		FName CallingContext = NAME_None);
};