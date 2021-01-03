// Copyright 2021 Phyronnaz

#include "VoxelGraphEditorModule.h"

#include "VoxelGraphEditor.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphEditorToolkit.h"
#include "VoxelGraphPanelPinFactory.h"
#include "VoxelGraphConnectionDrawingPolicy.h"
#include "VoxelGraphNodes/VoxelGraphNodeFactory.h"

#include "ContentBrowserModule.h"
#include "Modules/ModuleManager.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

const FVector2D Icon14x14(14.0f, 14.0f);
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);
const FVector2D Icon512x512(512.0f, 512.0f);

/**
 * Implements the VoxelEditor module.
 */
class FVoxelGraphEditorModule : public IVoxelGraphEditorModule
{
public:
	virtual void StartupModule() override
	{
		IVoxelGraphEditor::SetVoxelGraphEditor(MakeShared<FVoxelGraphEditor>());
		
		FEdGraphUtilities::RegisterVisualPinConnectionFactory(MakeShared<FVoxelGraphConnectionDrawingPolicyFactory>());
		FEdGraphUtilities::RegisterVisualNodeFactory(MakeShared<FVoxelGraphNodeFactory>());
		FEdGraphUtilities::RegisterVisualPinFactory(MakeShared<FVoxelGraphPanelPinFactory>());

		
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedAssets::CreateLambda([=](const TArray<FAssetData>& SelectedAssets)
		{
			const auto Extender = MakeShared<FExtender>();
			
			for (auto& It : SelectedAssets)
			{
				if (!It.GetClass()->IsChildOf<UVoxelGraphGenerator>())
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
					VOXEL_LOCTEXT("Compile voxel graph to C++"),
					TAttribute<FText>(),
					FSlateIcon(NAME_None, NAME_None),
					FUIAction(FExecuteAction::CreateLambda([=]()
					{
					})));
				}));

			return Extender;
		}));

		// Icons
		{
			StyleSet = MakeShareable(new FSlateStyleSet("VoxelGraphStyle"));
			StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
			StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

			// Compile To C++
			StyleSet->Set("VoxelGraphEditor.CompileToCpp"                    , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_compile_40x.png")), Icon40x40));
			StyleSet->Set("VoxelGraphEditor.CompileToCpp.Small"              , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_compile_40x.png")), Icon20x20));
																		     
			// Update Macros											     
			StyleSet->Set("VoxelGraphEditor.RecreateNodes"                   , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_Cascade_RestartInLevel_40x.png")), Icon40x40));
			StyleSet->Set("VoxelGraphEditor.RecreateNodes.Small"             , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_Cascade_RestartInLevel_40x.png")), Icon20x20));
																		     
			// Enable automatic preview									     
			StyleSet->Set("VoxelGraphEditor.ToggleAutomaticPreview"          , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_MatEd_LivePreview_40x.png")), Icon40x40));
			StyleSet->Set("VoxelGraphEditor.ToggleAutomaticPreview.Small"    , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_MatEd_LivePreview_40x.png")), Icon20x20));
																		     
			// Update preview											     
			StyleSet->Set("VoxelGraphEditor.UpdatePreview"                   , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_Cascade_RestartInLevel_40x.png")), Icon40x40));
			StyleSet->Set("VoxelGraphEditor.UpdatePreview.Small"             , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_Cascade_RestartInLevel_40x.png")), Icon20x20));
																		     
			// Clear messages											     
			StyleSet->Set("VoxelGraphEditor.ClearNodesMessages"              , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_file_new_40x.png")), Icon40x40));
			StyleSet->Set("VoxelGraphEditor.ClearNodesMessages.Small"        , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_file_new_16px.png")), Icon20x20));
					
			// Show Axis Dependencies
			StyleSet->Set("VoxelGraphEditor.ShowAxisDependencies"            , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/Profiler/profiler_Calls_32x.png")), Icon40x40));
			StyleSet->Set("VoxelGraphEditor.ShowAxisDependencies.Small"      , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/Profiler/profiler_Calls_32x.png")), Icon20x20));
		
			// Show stats												     
			StyleSet->Set("VoxelGraphEditor.ShowStats"                       , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/Profiler/Profiler_Data_Capture_40x.png")), Icon40x40));
			StyleSet->Set("VoxelGraphEditor.ShowStats.Small"                 , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/Profiler/Profiler_Data_Capture_40x.png")), Icon20x20));

			// Show values											     
			StyleSet->Set("VoxelGraphEditor.ShowValues"                      , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_TextureEd_CompressNow_512x.png")), Icon40x40));
			StyleSet->Set("VoxelGraphEditor.ShowValues.Small"                , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_TextureEd_CompressNow_512x.png")), Icon20x20));

			FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
		}
	}

	virtual void ShutdownModule() override
	{
		if (StyleSet.IsValid())
		{
			FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
			StyleSet.Reset();
		}
	}

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

	virtual TSharedRef<IVoxelGraphEditorToolkit> CreateVoxelGraphEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UVoxelGraphGenerator* Generator) override
	{
		TSharedRef<FVoxelGraphEditorToolkit> NewVoxelEditor(new FVoxelGraphEditorToolkit());
		NewVoxelEditor->InitVoxelEditor(Mode, InitToolkitHost, Generator);
		return NewVoxelEditor;
	}

private:
	TSharedPtr<FSlateStyleSet> StyleSet;
};

IMPLEMENT_MODULE(FVoxelGraphEditorModule, VoxelGraphEditor);

#undef IMAGE_PLUGIN_BRUSH