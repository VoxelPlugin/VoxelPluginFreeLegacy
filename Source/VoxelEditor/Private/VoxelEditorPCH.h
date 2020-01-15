// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

#include "Misc/CoreMisc.h"
#include "Misc/MessageDialog.h"
#include "Misc/UObjectToken.h"
#include "Misc/ScopedSlowTask.h"

#include "UObject/Stack.h"
#include "UObject/UObjectIterator.h"
#include "UObject/GCObject.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SPanel.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SWindow.h"

#include "Framework/SlateDelegates.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#include "Materials/MaterialInstanceDynamic.h"

#include "ThumbnailRendering/ThumbnailManager.h"

#include "Editor/UnrealEdEngine.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "Editor/TransBuffer.h"

#include "AdvancedPreviewSceneModule.h"
#include "AdvancedPreviewScene.h"
#include "PreviewScene.h"
#include "IDetailsView.h"
#include "ObjectTools.h"
#include "ScopedTransaction.h"
#include "InputCoreTypes.h"
#include "PropertyEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "AssetTypeActions_Base.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "UnrealEdMisc.h"
#include "UnrealEdGlobals.h"
#include "UnrealClient.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "EditorStyleSet.h"
#include "EditorViewportClient.h"
#include "EditorUndoClient.h"
#include "EditorReimportHandler.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "LevelEditor.h"
#include "RawMesh.h"
#include "PropertyCustomizationHelpers.h"
#include "Interfaces/IPluginManager.h"
#include "IPlacementModeModule.h"

#include "Logging/TokenizedMessage.h"
#include "Logging/MessageLog.h"

#include "LandscapeFileFormatInterface.h"
#include "LandscapeEditorModule.h"
#include "LandscapeComponent.h"
#include "LandscapeDataAccess.h"

#include "Components/LineBatchComponent.h"

#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/StaticMesh.h"
#include "Engine/StreamableManager.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/World.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Kismet2/KismetDebugUtilities.h"
#include "Kismet2/KismetEditorUtilities.h"

#include "Modules/ModuleManager.h"

#include "AssetTools/AssetTypeActions_VoxelBase.h"