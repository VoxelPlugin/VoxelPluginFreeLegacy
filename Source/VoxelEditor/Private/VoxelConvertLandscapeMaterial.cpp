// Copyright 2020 Phyronnaz

#include "VoxelConvertLandscapeMaterial.h"
#include "VoxelMinimal.h"
#include "VoxelUtilities/VoxelConfigUtilities.h"
#include "VoxelRender/VoxelMaterialExpressions.h"

#include "Editor.h"
#include "Materials/Material.h"
#include "Containers/Ticker.h"
#include "ContentBrowserModule.h"
#include "ScopedTransaction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

void FVoxelConvertLandscapeMaterial::Init()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedAssets::CreateLambda([=](const TArray<FAssetData>& SelectedAssets)
	{
		const auto Extender = MakeShared<FExtender>();

		for (auto& Asset : SelectedAssets)
		{
			if (Asset.GetClass() != UMaterial::StaticClass())
			{
				return Extender;
			}
		}
		
		Extender->AddMenuExtension(
			"CommonAssetActions",
			EExtensionHook::After,
			nullptr,
			FMenuExtensionDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddMenuEntry(
				VOXEL_LOCTEXT("Convert landscape material to voxel"),
				VOXEL_LOCTEXT("Will replace all landscape layer nodes with nodes compatible with both voxels and landscapes"),
				FSlateIcon(NAME_None, NAME_None),
				FUIAction(FExecuteAction::CreateLambda([=]()
				{
					for (auto& Asset : SelectedAssets)
					{
						auto* Material = Cast<UMaterial>(Asset.GetAsset());
						if (ensure(Material))
						{
							ConvertMaterial(Material);
						}
					}
				})));
			}));

		return Extender;
	}));
}

void FVoxelConvertLandscapeMaterial::ConvertMaterial(UMaterial* Material)
{
	FScopedTransaction Transaction(TEXT("ConvertMaterial"), VOXEL_LOCTEXT("Convert landscape material to voxel"), Material);

	TSet<UMaterialFunction*> VisitedFunctions;
	const int32 NumReplaced = ConvertExpressions(Material, Material->Expressions, VisitedFunctions);

	const FText Text = FText::Format(VOXEL_LOCTEXT("{0} expressions replaced in {1}"), NumReplaced, FText::FromName(Material->GetFName()));
	LOG_VOXEL(Log, TEXT("%s"), *Text.ToString());

	FNotificationInfo Info(Text);
	Info.ExpireDuration = 10.f;
	Info.CheckBoxState = ECheckBoxState::Checked;
	FSlateNotificationManager::Get().AddNotification(Info);
}

int32 FVoxelConvertLandscapeMaterial::ConvertExpressions(UObject* Owner, const TArray<UMaterialExpression*>& Expressions, TSet<UMaterialFunction*>& VisitedFunctions)
{
	int32 NumReplaced = 0;
	
	const auto ExpressionsCopy = Expressions;
	for (auto* Expression : ExpressionsCopy)
	{
		auto* VoxelClass = FVoxelMaterialExpressionUtilities::GetVoxelExpression(Expression->GetClass());
		if (VoxelClass)
		{
			ConvertExpression(Owner, Expression, VoxelClass);
			NumReplaced++;
		}
		if (auto* FunctionCall = Cast<UMaterialExpressionMaterialFunctionCall>(Expression))
		{
			auto* Function = Cast<UMaterialFunction>(FunctionCall->MaterialFunction);
			if (Function && !VisitedFunctions.Contains(Function))
			{
				VisitedFunctions.Add(Function);
				NumReplaced += ConvertExpressions(Function, Function->FunctionExpressions, VisitedFunctions);
			}
		}
	}
	
	return NumReplaced;
}

void FVoxelConvertLandscapeMaterial::ConvertExpression(UObject* Owner, UMaterialExpression* Expression, UClass* NewClass)
{
	Owner->Modify();
	
	auto* NewExpression = NewObject<UMaterialExpression>(Owner, NewClass, NAME_None, RF_Transactional);
	check(NewClass->IsChildOf(Expression->GetClass()));

	Expression->Modify();
	NewExpression->Modify();

	auto& Expressions = Owner->IsA<UMaterial>() ? CastChecked<UMaterial>(Owner)->Expressions : CastChecked<UMaterialFunction>(Owner)->FunctionExpressions;
	ensure(Expressions.Remove(Expression) == 1);
	Expressions.Add(NewExpression);

	// Copy data
	for (TFieldIterator<FProperty> It(Expression->GetClass()); It; ++It)
	{
		auto* Property = *It;
		if (!Property->HasAnyPropertyFlags(CPF_Transient))
		{
			Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(NewExpression), Property->ContainerPtrToValuePtr<void>(Expression));
		}
	}
	
	// Fixup other links
	for (UMaterialExpression* OtherExpression : Expressions)
	{
		if (OtherExpression != Expression)
		{
			for (FExpressionInput* Input : OtherExpression->GetInputs())
			{
				if (Input->Expression == Expression)
				{
					OtherExpression->Modify();
					Input->Expression = NewExpression;
				}
			}
		}
	}
	
	// Check material parameter inputs
	if (auto* Material = Cast<UMaterial>(Owner))
	{
		for (int32 InputIndex = 0; InputIndex < MP_MAX; InputIndex++)
		{
			FExpressionInput* Input = Material->GetExpressionInputForProperty((EMaterialProperty)InputIndex);
			if (Input && Input->Expression == Expression)
			{
				Input->Expression = NewExpression;
			}
		}
	}
}