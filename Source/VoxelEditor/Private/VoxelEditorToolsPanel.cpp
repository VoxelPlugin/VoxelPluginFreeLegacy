// Copyright 2020 Phyronnaz

#include "VoxelEditorToolsPanel.h"
#include "VoxelTools/VoxelToolManager.h"
#include "VoxelTools/Tools/VoxelTool.h"

#include "VoxelTools/Tools/VoxelFlattenTool.h"
#include "VoxelTools/Tools/VoxelLevelTool.h"
#include "VoxelTools/Tools/VoxelMeshTool.h"
#include "VoxelTools/Tools/VoxelRevertTool.h"
#include "VoxelTools/Tools/VoxelSmoothTool.h"
#include "VoxelTools/Tools/VoxelSphereTool.h"
#include "VoxelTools/Tools/VoxelSurfaceTool.h"
#include "VoxelTools/Tools/VoxelTrimTool.h"

#include "VoxelWorld.h"
#include "VoxelToolsCommands.h"
#include "VoxelDelegateHelpers.h"
#include "VoxelScopedTransaction.h"
#include "ActorFactoryVoxelWorld.h"
#include "VoxelUtilities/VoxelConfigUtilities.h"

#include "EngineUtils.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "DetailLayoutBuilder.h"
#if ENGINE_MINOR_VERSION >= 25
#include "VariablePrecisionNumericInterface.h"
#endif
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Slate/SceneViewport.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"

static const FString ToolConfigSectionName = "VoxelTool";

FVoxelEditorToolsPanel::FVoxelEditorToolsPanel()
	: Widget(SNew(SBox))
{
}

FVoxelEditorToolsPanel::~FVoxelEditorToolsPanel()
{
	if (ToolManager)
	{
		// Always save before exiting the panel (eg shortcuts do not dirty saves)
		FVoxelConfigUtilities::SaveConfig(&ToolManager->GetSharedConfig(), ToolConfigSectionName);
		if (auto* Tool = ToolManager->GetActiveTool())
		{
			FVoxelConfigUtilities::SaveConfig(Tool, ToolConfigSectionName);
		}
		
		ToolManager->SetActiveTool(nullptr);
	}
}

void FVoxelEditorToolsPanel::Init(const TSharedPtr<FUICommandList>& CommandListOverride)
{
	CommandList = CommandListOverride;
	if (!CommandList.IsValid())
	{
		CommandList = MakeShared<FUICommandList>();
	}
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea);
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
	
	ToolManager = NewObject<UVoxelToolManager>(GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional);
	ToolManager->CreateDefaultTools();
	ToolManager->GetSharedConfig().RefreshDetails.AddSP(this, &FVoxelEditorToolsPanel::RefreshDetails);
	ToolManager->GetSharedConfig().RegisterTransaction.AddLambda([](FName Name, AVoxelWorld* VoxelWorld)
	{
		FVoxelScopedTransaction Transaction(VoxelWorld, Name, EVoxelChangeType::Edit);
	});
	ToolsOptions.Reset();
	for (auto* Tool : ToolManager->GetTools())
	{
		FVoxelConfigUtilities::LoadConfig(Tool, ToolConfigSectionName);
		ToolsOptions.Add(MakeSharedCopy(Tool->GetClass()));
	}
	FVoxelConfigUtilities::LoadConfig(&ToolManager->GetSharedConfig(), ToolConfigSectionName);

	const auto IsPropertyVisibleDelegate = MakeWeakPtrDelegate(this, [=](const FPropertyAndParent& PropertyAndParent)
		{
#if ENGINE_MINOR_VERSION < 24
			TArray<const UProperty*> ParentProperties;
			if (PropertyAndParent.ParentProperty)
			{
				ParentProperties.Add(PropertyAndParent.ParentProperty);
			}
#else
			const auto& ParentProperties = PropertyAndParent.ParentProperties;
#endif
			return IsPropertyVisible(PropertyAndParent.Property, ParentProperties);
		});

	SharedConfigDetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	SharedConfigDetailsPanel->SetObject(&ToolManager->GetSharedConfig());
	SharedConfigDetailsPanel->SetIsPropertyVisibleDelegate(IsPropertyVisibleDelegate);
	SharedConfigDetailsPanel->OnFinishedChangingProperties().AddWeakLambda(ToolManager, [=](auto&)
	{
		FVoxelConfigUtilities::SaveConfig(&ToolManager->GetSharedConfig(), ToolConfigSectionName);
	});
	
	ToolDetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	ToolDetailsPanel->SetObject(ToolManager->GetActiveTool());
	ToolDetailsPanel->SetIsPropertyVisibleDelegate(IsPropertyVisibleDelegate);
	ToolDetailsPanel->OnFinishedChangingProperties().AddWeakLambda(ToolManager, [=](auto&)
	{
		if (auto* Tool = ToolManager->GetActiveTool())
		{
			FVoxelConfigUtilities::SaveConfig(Tool, ToolConfigSectionName);
		}
	});

	TSharedPtr<SVerticalBox> ToolBarsVerticalBox;

	Widget->SetContent(
	SNew(SScrollBox)
	+ SScrollBox::Slot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(FMargin(3, 5))
		.AutoHeight()
		[
			SNew(SBorder)
			.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FVoxelEditorToolsPanel::GetAddVoxelWorldVisibility)))
			.BorderBackgroundColor(FLinearColor::Red)
			.BorderImage(FCoreStyle::Get().GetBrush("ErrorReporting.Box"))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(FMargin(3, 0))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.ColorAndOpacity(FLinearColor::White)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular",12))
					.Text(VOXEL_LOCTEXT("No Voxel World in the scene!"))
				]
				+ SVerticalBox::Slot()
				.Padding(FMargin(0, 5))
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateSP(this, &FVoxelEditorToolsPanel::AddVoxelWorld))
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::Black)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular",12))
						.Text(VOXEL_LOCTEXT("Add Voxel World"))
						.Justification(ETextJustify::Center)
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SAssignNew(ToolBarsVerticalBox, SVerticalBox)
			/*+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				[
					SAssignNew(ComboBox, SComboBox<TSharedPtr<UClass*>>)
					.OptionsSource(&ToolsOptions)
					.OnSelectionChanged_Lambda([&](TSharedPtr<UClass*> Value, ESelectInfo::Type Type)
					{
						if (Type != ESelectInfo::Direct)
						{
							SetActiveTool(*Value);
						}
					})
					.OnGenerateWidget_Lambda([&](TSharedPtr<UClass*> Value)
					{
						return
							SNew(STextBlock)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.Text(FText::FromName((**Value).GetDefaultObject<UVoxelTool>()->GetToolName()));
					})
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text_Lambda([=]() 
						{
							if (auto* Tool = ToolManager->GetActiveTool())
							{
								return FText::FromName(Tool->GetToolName());
							}
							else
							{
								return FText();
							}
						})
					]
				]
			]*/
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SharedConfigDetailsPanel.ToSharedRef()
		]
		+ SVerticalBox::Slot()
		.FillHeight(1)
		[
			ToolDetailsPanel.ToSharedRef()
		]
	]);

	TArray<FToolBarBuilder> ToolBarBuilders;
	BuildToolBars(ToolBarBuilders);
	BindCommands();

	for (auto& ToolBarBuilder : ToolBarBuilders)
	{
		ToolBarsVerticalBox->AddSlot()
		.AutoHeight()
		[
			ToolBarBuilder.MakeWidget()
		];
	}

	FString ActiveTool;
	GConfig->GetString(TEXT("VoxelEditorToolsPanel"), TEXT("ActiveTool"), ActiveTool, GEditorPerProjectIni);

	SetActiveTool(*ToolsOptions[0]);
	for (auto& It : ToolsOptions)
	{
		if ((**It).GetName() == ActiveTool)
		{
			SetActiveTool(*It);
		}
	}

	// Show tooltip dialog
	bool bShowToolsShortcutsDialog = true;
	if (!GConfig->GetBool( TEXT("VoxelEditorToolsPanel"), TEXT("ShowToolsShortcutsDialog"), bShowToolsShortcutsDialog, GEditorPerProjectIni) || bShowToolsShortcutsDialog)
	{
		const auto Result = FMessageDialog::Open(EAppMsgType::YesNo, VOXEL_LOCTEXT(
			"The voxel plugin tools shortcuts are the following:\n"
			"1-8 (alphanum): select tools\n"
			"[ ]: decrease/increase brush size\n"
			"< >: decrease/increase brush strength\n\n"
			"You can configure these shortcuts at any time in Edit/Editor Preferences/Keyboard Shortcuts/Voxel\n\n"
			"Continue to show this popup?"));

		if (Result == EAppReturnType::No)
		{
			GConfig->SetBool( TEXT("VoxelEditorToolsPanel"), TEXT("ShowToolsShortcutsDialog"), false, GEditorPerProjectIni);
		}
	}
}

void FVoxelEditorToolsPanel::CustomizeToolbar(FToolBarBuilder& ToolBarBuilder)
{
	const auto& Commands = FVoxelToolsCommands::Get();
	
#if ENGINE_MINOR_VERSION >= 25
	ToolBarBuilder.AddToolBarButton(Commands.SurfaceTool);
	ToolBarBuilder.AddToolBarButton(Commands.SmoothTool);
	ToolBarBuilder.AddToolBarButton(Commands.MeshTool);
	ToolBarBuilder.AddToolBarButton(Commands.SphereTool);
	
	ToolBarBuilder.AddToolBarButton(Commands.FlattenTool);
	ToolBarBuilder.AddToolBarButton(Commands.LevelTool);
	ToolBarBuilder.AddToolBarButton(Commands.TrimTool);
	ToolBarBuilder.AddToolBarButton(Commands.RevertTool);

	ToolBarBuilder.AddSeparator();

	const auto NumericInterface = MakeShared<FVariablePrecisionNumericInterface>();

	//  Brush Size 
	{
		FProperty* BrushRadiusProperty = UVoxelToolSharedConfig::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UVoxelToolSharedConfig, BrushSize));
		const FString& UIMinString = BrushRadiusProperty->GetMetaData("UIMin");
		const FString& UIMaxString = BrushRadiusProperty->GetMetaData("UIMax");
		const FString& SliderExponentString = BrushRadiusProperty->GetMetaData("SliderExponent");
		float UIMin = TNumericLimits<float>::Lowest();
		float UIMax = TNumericLimits<float>::Max();
		TTypeFromString<float>::FromString(UIMin, *UIMinString);
		TTypeFromString<float>::FromString(UIMax, *UIMaxString);
		float SliderExponent = 1.0f;
		if (SliderExponentString.Len())
		{
			TTypeFromString<float>::FromString(SliderExponent, *SliderExponentString);
		}

		const auto SizeControl = 
			SNew(SSpinBox<float>)
			.Style(&FEditorStyle::Get().GetWidgetStyle<FSpinBoxStyle>("LandscapeEditor.SpinBox"))
			.PreventThrottling(true)
			.Value_Lambda([=]() -> float { return ToolManager->GetSharedConfig().BrushSize; })
			.OnValueChanged_Lambda([=](float NewValue) { ToolManager->GetSharedConfig().BrushSize = NewValue; })

			.MinValue(UIMin)
			.MaxValue(UIMax)
			.SliderExponent(SliderExponent)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			.MinDesiredWidth(40.f)
			.TypeInterface(NumericInterface)
			.Justification(ETextJustify::Center);
		ToolBarBuilder.AddToolBarWidget( SizeControl, VOXEL_LOCTEXT("Brush Size") );
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelEditorToolsPanel::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ToolManager);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelEditorToolsPanel::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
}

void FVoxelEditorToolsPanel::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	auto* World = ViewportClient->GetWorld();
	if (!ensure(World)) return;
	
	if (!LastWorld.IsValid())
	{
		// Toggle voxel worlds if none are created on first tick
		const auto ToggleVoxelWorld = [&]()
		{
			for (auto* VoxelWorld : TActorRange<AVoxelWorld>(World))
			{
				if (VoxelWorld->IsCreated())
				{
					return;
				}
			}
			for (auto* VoxelWorld : TActorRange<AVoxelWorld>(World))
			{
				if (!VoxelWorld->IsCreated())
				{
					VoxelWorld->Toggle();
				}
			}
		};
		ToggleVoxelWorld();
	}
	LastWorld = ViewportClient->GetWorld();
	
	if (ToolManager)
	{
		auto* Tool = ToolManager->GetActiveTool();
		if (Tool)
		{
			check(!ViewportClientForDeproject);
			TGuardValue<FEditorViewportClient*> Guard(ViewportClientForDeproject, ViewportClient);

			FViewport* Viewport = ViewportClient->Viewport;
			TUniquePtr<FSceneViewFamilyContext> ViewFamily;
			FSceneView* SceneView = GetSceneView(ViewFamily);

			if (Viewport && SceneView)
			{
				TMap<FName, bool> Keys;
				Keys.Add(FVoxelToolKeys::AlternativeMode, bAlternativeMode);

				FVoxelToolTickData TickData;
				{
					auto* SceneViewport = static_cast<FSceneViewport*>(Viewport);

					const auto& Geometry = SceneViewport->GetCachedGeometry();

					FVector2D MousePosition = FSlateApplication::Get().GetCursorPos();
					if (Geometry.IsUnderLocation(MousePosition) || bClick) // Don't modify the position if editing
					{
						MousePosition = Geometry.AbsoluteToLocal(MousePosition);
					}
					else
					{
						MousePosition = Geometry.Size / 2;
					}

					const float DPIScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(MousePosition.X, MousePosition.Y);
					MousePosition *= DPIScaleFactor;
					
					TickData.MousePosition = FVector2D(MousePosition);
					TickData.CameraViewDirection = SceneView->ViewMatrices.GetInvViewMatrix().TransformVector(FVector(0, 0, 1));
					TickData.bEdit = bClick;
					TickData.Keys = Keys;
					
					TickData.Axes.Add(FVoxelToolAxes::BrushSize, BrushSizeDelta);
					TickData.Axes.Add(FVoxelToolAxes::Falloff, FalloffDelta);
					TickData.Axes.Add(FVoxelToolAxes::Strength, StrengthDelta);

					BrushSizeDelta = 0;
					FalloffDelta = 0;
					StrengthDelta = 0;

					const auto DeprojectLambda = [this, WeakPtr = MakeWeakPtr(this)](const FVector2D& InScreenPosition, FVector& OutWorldPosition, FVector& OutWorldDirection)
					{
						return WeakPtr.IsValid() && Deproject(InScreenPosition, OutWorldPosition, OutWorldDirection);
					};
					TickData.Init(DeprojectLambda);
				}
				Tool->AdvancedTick(World, TickData);

				if (LastVoxelWorld != Tool->GetVoxelWorld())
				{
					LastVoxelWorld = Tool->GetVoxelWorld();
					SharedConfigDetailsPanel->ForceRefresh();
					ToolDetailsPanel->ForceRefresh();
				}

				ViewportClientForDeproject = nullptr;
			}
		}
	}

	TimeUntilNextGC -= DeltaTime;
	if (TimeUntilNextGC <= 0)
	{
		TimeUntilNextGC = 30;
		LOG_VOXEL(Log, TEXT("Forcing GC"));
		GEngine->ForceGarbageCollection(true);
	}
}

void FVoxelEditorToolsPanel::HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
}

bool FVoxelEditorToolsPanel::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{	
	if (Event != IE_Released && CommandList->ProcessCommandBindings(Key, FSlateApplication::Get().GetModifierKeys(), false/*Event == IE_Repeat*/))
	{
		return true;
	}
	
	if (Key == EKeys::LeftMouseButton)
	{
		if (Event == EInputEvent::IE_Pressed)
		{
			bClick = true;
		}
		else if (Event == EInputEvent::IE_Released)
		{
			bClick = false;
		}
		return true;
	}
	else if (Key == EKeys::LeftShift || Key == EKeys::RightShift)
	{
		if (Event == EInputEvent::IE_Pressed)
		{
			bAlternativeMode = true;
		}
		else if (Event == EInputEvent::IE_Released)
		{
			bAlternativeMode = false;
		}
		return true;
	}
	else 
	{
		return false;
	}
}

bool FVoxelEditorToolsPanel::InputAxis(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, float Delta, float DeltaTime)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EVisibility FVoxelEditorToolsPanel::GetAddVoxelWorldVisibility() const
{
	if (!LastWorld.IsValid())
	{
		return EVisibility::Collapsed;
	}
	
	for (auto* VoxelWorld : TActorRange<AVoxelWorld>(LastWorld.Get()))
	{
		return EVisibility::Collapsed;
	}

	return EVisibility::Visible;
}

FReply FVoxelEditorToolsPanel::AddVoxelWorld() const
{
	if (!LastWorld.IsValid())
	{
		return FReply::Handled();
	}

	auto* Factory = NewObject<UActorFactoryVoxelWorld>();
	if (ensure(Factory))
	{
		Factory->CreateActor(nullptr, LastWorld->GetLevel(0), FTransform::Identity);
	}

	return FReply::Handled();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelEditorToolsPanel::RefreshDetails() const
{
	SharedConfigDetailsPanel->ForceRefresh();
	ToolDetailsPanel->ForceRefresh();
}

bool FVoxelEditorToolsPanel::IsPropertyVisible(const UProperty& Property, const TArray<const UProperty*>& ParentProperties, int32 ParentPropertyIndex) const
{
	if (Property.HasMetaData(STATIC_FNAME("HideInPanel")))
	{
		return false;
	}
	
	if (Property.HasMetaData(STATIC_FNAME("PaintMaterial")))
	{
		auto* ActiveTool = ToolManager->GetActiveTool();
		if (ActiveTool && !ActiveTool->bShowPaintMaterial)
		{
			return false;
		}
	}

	if (Property.HasMetaData(STATIC_FNAME("ShowForMaterialConfigs")))
	{
		const FString& ShowForMaterialConfigs = Property.GetMetaData(STATIC_FNAME("ShowForMaterialConfigs"));
		if (LastVoxelWorld.IsValid())
		{
			const FString MaterialConfig = StaticEnum<EVoxelMaterialConfig>()->GetNameStringByValue(int64(LastVoxelWorld->MaterialConfig));
			if (!ShowForMaterialConfigs.Contains(MaterialConfig))
			{
				return false;
			}
		}
	}

	// TODO

	if (ParentProperties.IsValidIndex(ParentPropertyIndex))
	{
		return IsPropertyVisible(*ParentProperties[ParentPropertyIndex], ParentProperties, ParentPropertyIndex + 1);
	}
	else
	{
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelEditorToolsPanel::SetActiveTool(UClass* ToolClass)
{
	ToolManager->SetActiveToolByClass(ToolClass);
	ToolDetailsPanel->SetObject(ToolManager->GetActiveTool());
	SharedConfigDetailsPanel->ForceRefresh();

	if (ComboBox)
	{
		ComboBox->SetSelectedItem(*ToolsOptions.FindByPredicate([&](auto& ClassPtr) { return *ClassPtr == ToolClass; }));
	}

	GConfig->SetString(TEXT("VoxelEditorToolsPanel"), TEXT("ActiveTool"), *ToolClass->GetName(), GEditorPerProjectIni);
}

bool FVoxelEditorToolsPanel::IsToolActive(UClass* ToolClass) const
{
	auto* Tool = ToolManager->GetActiveTool();
	return Tool && Tool->GetClass() == ToolClass;
}

void FVoxelEditorToolsPanel::BuildToolBars(TArray<FToolBarBuilder>& OutToolBars)
{
	const auto& Commands = FVoxelToolsCommands::Get();

	int32 NumTools = 0;
	const auto GetToolBar = [&]()
	{
		if (NumTools % 4 == 0)
		{
			OutToolBars.Emplace(CommandList, FMultiBoxCustomization("VoxelEditorTools"));
		}
		NumTools++;
		return OutToolBars.Last();
	};

	const auto GetUIAction = [&](auto* Class)
	{
		return FUIAction(
			FExecuteAction::CreateSP(this, &FVoxelEditorToolsPanel::SetActiveTool, Class),
			FCanExecuteAction::CreateLambda([]() { return true; }),
			FIsActionChecked::CreateSP(this, &FVoxelEditorToolsPanel::IsToolActive, Class));
	};

	const auto AddTool = [&](auto& Command, auto* Class)
	{
		CommandList->MapAction(Command, GetUIAction(Class));
	};

	AddTool(Commands.FlattenTool, UVoxelFlattenTool::StaticClass());
	AddTool(Commands.LevelTool, UVoxelLevelTool::StaticClass());
	AddTool(Commands.MeshTool, UVoxelMeshTool::StaticClass());
	AddTool(Commands.SmoothTool, UVoxelSmoothTool::StaticClass());
	AddTool(Commands.SphereTool, UVoxelSphereTool::StaticClass());
	AddTool(Commands.SurfaceTool, UVoxelSurfaceTool::StaticClass());
	AddTool(Commands.RevertTool, UVoxelRevertTool::StaticClass());
	AddTool(Commands.TrimTool, UVoxelTrimTool::StaticClass());
	
	GetToolBar().AddToolBarButton(Commands.SurfaceTool);
	GetToolBar().AddToolBarButton(Commands.SmoothTool);
	GetToolBar().AddToolBarButton(Commands.MeshTool);
	GetToolBar().AddToolBarButton(Commands.SphereTool);

	GetToolBar().AddToolBarButton(Commands.FlattenTool);
	GetToolBar().AddToolBarButton(Commands.LevelTool);
	GetToolBar().AddToolBarButton(Commands.TrimTool);
	GetToolBar().AddToolBarButton(Commands.RevertTool);

	for (auto* Tool : ToolManager->GetTools())
	{
		auto* Class = Tool->GetClass();
		if (Class->GetPathName().StartsWith(TEXT("/Script/Voxel.")))
		{
			// Builtin tools
			continue;
		}

		GetToolBar().AddToolBarButton(
			GetUIAction(Class),
			NAME_None,
			FText::FromName(Tool->GetToolName()),
			Tool->ToolTip,
			FSlateIcon("VoxelStyle", "VoxelTools.Surface"),
			EUserInterfaceActionType::ToggleButton);
	}
	
}

void FVoxelEditorToolsPanel::BindCommands()
{
	const auto& Commands = FVoxelToolsCommands::Get();

	CommandList->MapAction(Commands.IncreaseBrushSize, MakeWeakPtrDelegate(this, [&](){ BrushSizeDelta++; }));
	CommandList->MapAction(Commands.DecreaseBrushSize, MakeWeakPtrDelegate(this, [&](){ BrushSizeDelta--; }));

	CommandList->MapAction(Commands.IncreaseBrushFalloff, MakeWeakPtrDelegate(this, [&](){ FalloffDelta++; }));
	CommandList->MapAction(Commands.DecreaseBrushFalloff, MakeWeakPtrDelegate(this, [&](){ FalloffDelta--; }));

	CommandList->MapAction(Commands.IncreaseBrushStrength, MakeWeakPtrDelegate(this, [&](){ StrengthDelta++; }));
	CommandList->MapAction(Commands.DecreaseBrushStrength, MakeWeakPtrDelegate(this, [&](){ StrengthDelta--; }));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FSceneView* FVoxelEditorToolsPanel::GetSceneView(TUniquePtr<FSceneViewFamilyContext>& ViewFamily) const
{
	if (!ensure(ViewportClientForDeproject))
	{
		return nullptr;
	}

	FViewport* Viewport = ViewportClientForDeproject->Viewport;
	
	// Make sure we have a valid viewport, otherwise we won't be able to construct an FSceneView
	if (!Viewport || Viewport->GetSizeXY().GetMin() <= 0)
	{
		return nullptr;
	}
	
	ViewFamily = MakeUnique<FSceneViewFamilyContext>(FSceneViewFamily::ConstructionValues(
		Viewport,
		ViewportClientForDeproject->GetScene(),
		ViewportClientForDeproject->EngineShowFlags)
		.SetRealtimeUpdate(ViewportClientForDeproject->IsRealtime()));
	
	FSceneView* SceneView = ViewportClientForDeproject->CalcSceneView(ViewFamily.Get());
	ensure(SceneView);
	return SceneView;
}

bool FVoxelEditorToolsPanel::Deproject(const FVector2D& ScreenPosition, FVector& OutWorldPosition, FVector& OutWorldDirection) const
{
	TUniquePtr<FSceneViewFamilyContext> ViewFamily;
	auto* SceneView = GetSceneView(ViewFamily);
	if (!SceneView)
	{
		return false;
	}

	const FViewportCursorLocation MouseViewportRay(SceneView, ViewportClientForDeproject, FMath::RoundToInt(ScreenPosition.X), FMath::RoundToInt(ScreenPosition.Y));

	OutWorldPosition = MouseViewportRay.GetOrigin();
	OutWorldDirection = MouseViewportRay.GetDirection();

	// If we're dealing with an orthographic view, push the origin of the ray backward along the viewport forward axis
	// to make sure that we can select objects that are behind the origin!
	if (!ViewportClientForDeproject->IsPerspective())
	{
		OutWorldPosition -= OutWorldDirection * WORLD_MAX / 2;
	}

	return true;
}