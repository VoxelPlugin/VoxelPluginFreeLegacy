// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VoxelFactory.h"

#include "Containers/UnrealString.h"
#include "VoxelAsset.h"
#include "Misc/FileHelper.h"


/* UVoxelFactory structors
 *****************************************************************************/

UVoxelFactory::UVoxelFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("txt;")) + NSLOCTEXT("UVoxelFactory", "FormatTxt", "Text File").ToString());
	SupportedClass = UVoxelAsset::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
}


/* UFactory overrides
 *****************************************************************************/

 /* This is the old API (only for demonstration purposes)
 UObject* UVoxelFactory::FactoryCreateBinary(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
 {
	 UVoxel* Voxel = nullptr;
	 FString TextString;

	 if (FFileHelper::LoadFileToString(TextString, *CurrentFilename))
	 {
		 Voxel = NewObject<UVoxel>(InParent, Class, Name, Flags);
		 Voxel->Text = FText::FromString(TextString);
	 }

	 return Voxel;
 }*/


UObject* UVoxelFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	/*UVoxelAsset* Voxel = nullptr;
	FString TextString;

	if (FFileHelper::LoadFileToString(TextString, *Filename))
	{
		Voxel = NewObject<UVoxelAsset>(InParent, InClass, InName, Flags);
		Voxel->Text = FText::FromString(TextString);
	}

	bOutOperationCanceled = false;

	return Voxel;*/
	return nullptr;
}
