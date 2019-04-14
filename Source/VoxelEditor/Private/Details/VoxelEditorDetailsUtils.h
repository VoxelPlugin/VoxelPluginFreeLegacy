// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Framework/SlateDelegates.h"

class STextBlock;
class SButton;
class UFactory;
class IDetailLayoutBuilder;

class FVoxelEditorDetailsUtils
{
public:
	static bool ShowWarning(const FText& Text);
	static void ShowError(const FText& Text);
	static void EnableRealtime();

	static void CreateAsset(UFactory* Factory, UClass* AssetClass, TFunction<bool(UObject*)> CreateAssetFunction);

	static TSharedRef<STextBlock> CreateText(const FText& Text);

	static TSharedRef<SButton> CreateButton(
		const FText& Text,
		const FOnClicked& OnClicked,
		const TAttribute<bool>& IsEnabled = true);

	static void AddButtonToCategory(
		IDetailLayoutBuilder& DetailLayout,
		const FName& CategoryName,
		const FText& FilterString,
		const FText& TextLeftToButton,
		const FText& ButtonText,
		const FOnClicked& OnClicked,
		const TAttribute<bool>& IsEnabled = true);

	static void AddPropertyToCategory(
		IDetailLayoutBuilder& DetailLayout,
		const FName& CategoryName,
		const FName& PropertyName,
		UClass* Class = nullptr);

	// Same as engine one, but don't allow replacing assets
	static UObject* CreateAssetWithDialog(
		const FString& AssetName,
		const FString& PackagePath,
		UClass* AssetClass,
		UFactory* Factory,
		FName CallingContext);
	
	// Same as engine one, but don't allow replacing assets
	static UObject* CreateAssetWithDialog(
		UClass* AssetClass, 
		UFactory* Factory, 
		FName CallingContext = NAME_None);
};