// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

#include "EditorViewportClient.h"
#include "AdvancedPreviewScene.h"
#include "PreviewScene.h"

#include "SCommonEditorViewportToolbarBase.h"
#include "SEditorViewport.h"
#include "SGraphNode.h"

#include "KismetPins/SGraphPinExec.h"
#include "KismetPins/SGraphPinBool.h"
#include "KismetPins/SGraphPinInteger.h"
#include "KismetPins/SGraphPinNum.h"
#include "KismetPins/SGraphPinColor.h"

#include "SGraphPalette.h"
#include "Slate/SceneViewport.h"
#include "Widgets/SWindow.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"

#include "ThumbnailRendering/ThumbnailManager.h"
#include "Toolkits/AssetEditorToolkit.h"

#include "Editor.h"
#include "EditorUndoClient.h"
#include "EditorReimportHandler.h"
#include "TickableEditorObject.h"
#include "TickableEditorObject.h"
#include "LevelEditor.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#include "Engine/EngineTypes.h"
#include "Engine/Font.h"
#include "UObject/GCObject.h"

#include "EdGraph/EdGraphSchema.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraph.h"
#include "GraphEditorActions.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/SClassPickerDialog.h"

#include "PropertyEditorModule.h"
#include "PropertyCustomizationHelpers.h"
#include "DesktopPlatformModule.h"
#include "GameProjectGenerationModule.h"
#include "Modules/ModuleManager.h"
#include "ClassViewerFilter.h"
#include "GameProjectUtils.h"

#include "HAL/PlatformFilemanager.h"

#include "Misc/NotifyHook.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#include "ScopedTransaction.h"
#include "MessageLogModule.h"
#include "IMessageLogListing.h"
#include "Logging/TokenizedMessage.h"

#include "Curves/RichCurve.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"

#include "Materials/MaterialInstanceDynamic.h"