// Copyright 2020 Phyronnaz

#include "SVoxelGraphNode.h"
#include "VoxelGraphNode_Base.h"
#include "VoxelNodes/VoxelParameterNodes.h"
#include "VoxelNodes/VoxelAssetPickerNode.h"
#include "GraphEditorSettings.h"
#include "IDocumentation.h"
#include "TutorialMetaData.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"
#include "SCommentBubble.h"
#include "SLevelOfDetailBranchNode.h"
#include "SGraphPin.h"
#include "Engine/Engine.h"
#include "PropertyCustomizationHelpers.h"
#include "LevelEditor.h"

void SVoxelGraphNode::Construct(const FArguments& InArgs, class UVoxelGraphNode_Base* InNode)
{
	GraphNode = InNode;
	VoxelNode = InNode;

	SetCursor(EMouseCursor::CardinalCross);

	UpdateGraphNode();
}

void SVoxelGraphNode::RefreshErrorInfo()
{
	SetupErrorReporting();
}

void SVoxelGraphNode::UpdateGraphNode()
{
	if (CastChecked<UVoxelGraphNode_Base>(GraphNode)->IsCompact())
	{
		UpdateCompactNode();
	}
	else
	{
		UpdateStandardNode();
	}
}

void SVoxelGraphNode::CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox)
{
	TSharedRef<SWidget> AddPinButton = AddPinButtonContent(
		VOXEL_LOCTEXT("Add input"),
		VOXEL_LOCTEXT("Adds an input to the Voxel node")
	);

	FMargin AddPinPadding = Settings->GetOutputPinPadding();
	AddPinPadding.Top += 6.0f;

	OutputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(AddPinPadding)
		[
			AddPinButton
		];
}

EVisibility SVoxelGraphNode::IsAddPinButtonVisible() const
{
	EVisibility ButtonVisibility = SGraphNode::IsAddPinButtonVisible();
	if (ButtonVisibility == EVisibility::Visible)
	{
		if (!VoxelNode->CanAddInputPin())
		{
			ButtonVisibility = EVisibility::Collapsed;
		}
	}
	return ButtonVisibility;
}

FReply SVoxelGraphNode::OnAddPin()
{
	VoxelNode->AddInputPin();

	return FReply::Handled();
}

TSharedRef<SWidget> SVoxelGraphNode::CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle)
{
	SAssignNew(InlineEditableText, SInlineEditableTextBlock)
		.Style(FEditorStyle::Get(), "Graph.Node.NodeTitleInlineEditableText")
		.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
		.OnVerifyTextChanged(this, &SVoxelGraphNode::OnVerifyNameTextChanged)
		.OnTextCommitted(this, &SVoxelGraphNode::OnNameTextCommited)
		.IsReadOnly(this, &SVoxelGraphNode::IsNameReadOnly)
		.IsSelected(this, &SVoxelGraphNode::IsSelectedExclusively);
	InlineEditableText->SetColorAndOpacity(TAttribute<FLinearColor>::Create(TAttribute<FLinearColor>::FGetter::CreateSP(this, &SVoxelGraphNode::GetNodeTitleTextColor)));

	return InlineEditableText.ToSharedRef();
}

void SVoxelGraphNode::UpdateStandardNode()
{	
	InputPins.Empty();
	OutputPins.Empty();
	
	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	//
	//             ______________________
	//            |      TITLE AREA      |
	//            +-------+------+-------+
	//            | (>) L |      | R (>) |
	//            | (>) E |      | I (>) |
	//            | (>) F |      | G (>) |
	//            | (>) T |      | H (>) |
	//            |       |      | T (>) |
	//            |_______|______|_______|
	//
	TSharedPtr<SVerticalBox> MainVerticalBox;
	SetupErrorReporting();

	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	// Get node icon
	IconColor = FLinearColor::White;
	const FSlateBrush* IconBrush = NULL;
	if (GraphNode != NULL && GraphNode->ShowPaletteIconOnNode())
	{
		IconBrush = GraphNode->GetIconAndTint(IconColor).GetOptionalIcon();
	}

	TSharedRef<SOverlay> DefaultTitleAreaWidget =
		SNew(SOverlay)
		+SOverlay::Slot()
		[
			SNew(SImage)
			.Image( FEditorStyle::GetBrush("Graph.Node.TitleGloss") )
			.ColorAndOpacity( this, &SGraphNode::GetNodeTitleIconColor )
		]
		+SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage( FEditorStyle::GetBrush("Graph.Node.ColorSpill") )
			// The extra margin on the right
			// is for making the color spill stretch well past the node title
			.Padding( FMargin(10,5,30,3) )
			.BorderBackgroundColor( this, &SGraphNode::GetNodeTitleColor )
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 0.f, 4.f, 0.f))
				.AutoWidth()
				[
					SNew(SImage)
					.Image(IconBrush)
					.ColorAndOpacity(this, &SGraphNode::GetNodeTitleIconColor)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						CreateTitleWidget(NodeTitle)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						NodeTitle.ToSharedRef()
					]
				]
			]
		]
		+SOverlay::Slot()
		.VAlign(VAlign_Top)
		[
			SNew(SBorder)
			.Visibility(EVisibility::HitTestInvisible)			
			.BorderImage( FEditorStyle::GetBrush( "Graph.Node.TitleHighlight" ) )
			.BorderBackgroundColor( this, &SGraphNode::GetNodeTitleIconColor )
			[
				SNew(SSpacer)
				.Size(FVector2D(20,20))
			]
		];

	SetDefaultTitleAreaWidget(DefaultTitleAreaWidget);

	TSharedRef<SWidget> TitleAreaWidget = 
		SNew(SLevelOfDetailBranchNode)
		.UseLowDetailSlot(this, &SVoxelGraphNode::UseLowDetailNodeTitles)
		.LowDetail()
		[
			SNew(SBorder)
			.BorderImage( FEditorStyle::GetBrush("Graph.Node.ColorSpill") )
			.Padding( FMargin(75.0f, 22.0f) ) // Saving enough space for a 'typical' title so the transition isn't quite so abrupt
			.BorderBackgroundColor( this, &SGraphNode::GetNodeTitleColor )
		]
		.HighDetail()
		[
			DefaultTitleAreaWidget
		];

	
	if (!SWidget::GetToolTip().IsValid())
	{
		TSharedRef<SToolTip> DefaultToolTip = IDocumentation::Get()->CreateToolTip( TAttribute< FText >( this, &SGraphNode::GetNodeTooltip ), NULL, GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName() );
		SetToolTip(DefaultToolTip);
	}

	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("Graphnode"));
	PopulateMetaTag(&TagMeta);
	
	TSharedPtr<SVerticalBox> InnerVerticalBox;
	this->ContentScale.Bind( this, &SGraphNode::GetContentScale );


	InnerVerticalBox = SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.Padding(Settings->GetNonPinNodeBodyPadding())
		[
			TitleAreaWidget
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		[
			CreateNodeContentArea()
		];

	if ((GraphNode->GetDesiredEnabledState() != ENodeEnabledState::Enabled) && !GraphNode->IsAutomaticallyPlacedGhostNode())
	{
		const bool bDevelopmentOnly = GraphNode->GetDesiredEnabledState() == ENodeEnabledState::DevelopmentOnly;
		const FText StatusMessage = bDevelopmentOnly ? VOXEL_LOCTEXT("Development Only") : VOXEL_LOCTEXT("Disabled");
		const FText StatusMessageTooltip = bDevelopmentOnly ?
			VOXEL_LOCTEXT("This node will only be executed in the editor and in Development builds in a packaged game (it will be treated as disabled in Shipping or Test builds cooked from a commandlet)") :
			VOXEL_LOCTEXT("This node is currently disabled and will not be executed");

		InnerVerticalBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.Padding(FMargin(2, 0))
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush(bDevelopmentOnly ? "Graph.Node.DevelopmentBanner" : "Graph.Node.DisabledBanner"))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(STextBlock)
					.Text(StatusMessage)
					.ToolTipText(StatusMessageTooltip)
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FLinearColor::White)
					.ShadowOffset(FVector2D::UnitVector)
					.Visibility(EVisibility::Visible)
				]
			];
	}
	
	InnerVerticalBox->AddSlot()
		.AutoHeight()
		.Padding(Settings->GetNonPinNodeBodyPadding())
		[
			InfoReporting->AsWidget()
		];
	InnerVerticalBox->AddSlot()
		.AutoHeight()
		.Padding(Settings->GetNonPinNodeBodyPadding())
		[
			WarningReporting->AsWidget()
		];
	InnerVerticalBox->AddSlot()
		.AutoHeight()
		.Padding(Settings->GetNonPinNodeBodyPadding())
		[
			ErrorReporting->AsWidget()
		];



	this->GetOrAddSlot( ENodeZone::Center )
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(MainVerticalBox, SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SOverlay)
				.AddMetaData<FGraphNodeMetaData>(TagMeta)
				+SOverlay::Slot()
				.Padding(Settings->GetNonPinNodeBodyPadding())
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("Graph.Node.Body"))
					.ColorAndOpacity(this, &SVoxelGraphNode::GetNodeBodyColor)
				]
				+SOverlay::Slot()
				[
					InnerVerticalBox.ToSharedRef()
				]
			]			
		];

	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew( CommentBubble, SCommentBubble )
	.GraphNode( GraphNode )
	.Text( this, &SGraphNode::GetNodeComment )
	.OnTextCommitted( this, &SGraphNode::OnCommentTextCommitted )
	.OnToggled( this, &SGraphNode::OnCommentBubbleToggled )
	.ColorAndOpacity( CommentColor )
	.AllowPinning( true )
	.EnableTitleBarBubble( true )
	.EnableBubbleCtrls( true )
	.GraphLOD( this, &SGraphNode::GetCurrentLOD )
	.IsGraphNodeHovered( this, &SGraphNode::IsHovered );

	GetOrAddSlot( ENodeZone::TopCenter )
	.SlotOffset( TAttribute<FVector2D>( CommentBubble.Get(), &SCommentBubble::GetOffset ))
	.SlotSize( TAttribute<FVector2D>( CommentBubble.Get(), &SCommentBubble::GetSize ))
	.AllowScaling( TAttribute<bool>( CommentBubble.Get(), &SCommentBubble::IsScalingAllowed ))
	.VAlign( VAlign_Top )
	[
		CommentBubble.ToSharedRef()
	];

	CreateBelowWidgetControls(MainVerticalBox);
	CreatePinWidgets();
	CreateInputSideAddButton(LeftNodeBox);
	CreateOutputSideAddButton(RightNodeBox);
	CreateBelowPinControls(InnerVerticalBox);
	CreateAdvancedViewArrow(InnerVerticalBox);
}

void SVoxelGraphNode::UpdateCompactNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// error handling set-up
	SetupErrorReporting();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();
	
	if (!SWidget::GetToolTip().IsValid())
	{
		TSharedRef<SToolTip> DefaultToolTip = IDocumentation::Get()->CreateToolTip( TAttribute< FText >( this, &SGraphNode::GetNodeTooltip ), NULL, GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName() );
		SetToolTip(DefaultToolTip);
	}

	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("Graphnode"));
	PopulateMetaTag(&TagMeta);

	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode).Text(this, &SVoxelGraphNode::GetNodeCompactTitle);

	TSharedRef<SOverlay> NodeOverlay = SNew(SOverlay);

	// add optional node specific widget to the overlay:
	TSharedPtr<SWidget> OverlayWidget = GraphNode->CreateNodeImage();
	if (OverlayWidget.IsValid())
	{
		NodeOverlay->AddSlot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(70.f)
			.HeightOverride(70.f)
			[
				OverlayWidget.ToSharedRef()
			]
			];
	}

	NodeOverlay->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(45.f, 0.f, 45.f, 0.f)
		[
			// MIDDLE
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.AutoHeight()
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "Graph.CompactNode.Title")
		.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
		.WrapTextAt(128.0f)
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			NodeTitle.ToSharedRef()
		]
		];

	NodeOverlay->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 55.f, 0.f)
		[
			// LEFT
			SAssignNew(LeftNodeBox, SVerticalBox)
		];

	NodeOverlay->AddSlot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(55.f, 0.f, 0.f, 0.f)
		[
			// RIGHT
			SAssignNew(RightNodeBox, SVerticalBox)
		];

	//
	//             ______________________
	//            | (>) L |      | R (>) |
	//            | (>) E |      | I (>) |
	//            | (>) F |   +  | G (>) |
	//            | (>) T |      | H (>) |
	//            |       |      | T (>) |
	//            |_______|______|_______|
	//
	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		[
			// NODE CONTENT AREA
			SNew(SOverlay)
			+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("Graph.VarNode.Body"))
			.ColorAndOpacity(this, &SVoxelGraphNode::GetNodeBodyColor)
		]
	+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("Graph.VarNode.Gloss"))
			.ColorAndOpacity(this, &SVoxelGraphNode::GetNodeBodyColor)
		]
	+ SOverlay::Slot()
		.Padding(FMargin(0, 3))
		[
			NodeOverlay
		]
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(5.0f, 1.0f))
		[
			InfoReporting->AsWidget()
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(5.0f, 1.0f))
		[
			WarningReporting->AsWidget()
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(5.0f, 1.0f))
		[
			ErrorReporting->AsWidget()
		]
		];

	CreatePinWidgets();

	// Hide pin labels
	for (auto& InputPin : InputPins)
	{
		if (InputPin->GetPinObj()->ParentPin == nullptr)
		{
			InputPin->SetShowLabel(false);
		}
	}

	for (auto& OutputPin : OutputPins)
	{
		if (OutputPin->GetPinObj()->ParentPin == nullptr)
		{
			OutputPin->SetShowLabel(false);
		}
	}

	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SVoxelGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	CreateInputSideAddButton(LeftNodeBox);
	CreateOutputSideAddButton(RightNodeBox);
}

FText SVoxelGraphNode::GetNodeCompactTitle() const
{
	return GraphNode->GetNodeTitle(ENodeTitleType::FullTitle);
}

FSlateColor SVoxelGraphNode::GetNodeBodyColor() const
{
	return CastChecked<UVoxelGraphNode_Base>(GraphNode)->GetNodeBodyColor();
}

void SVoxelGraphNode::SetupErrorReporting()
{
	UpdateErrorInfo();
	
	if (!InfoReporting.IsValid())
	{
		TSharedPtr<SErrorText> InfoTextWidget;

		// generate widget
		SAssignNew(InfoTextWidget, SErrorText)
			.BackgroundColor(this, &SVoxelGraphNode::GetInfoColor)
			.ToolTipText(this, &SVoxelGraphNode::GetInfoMsgToolTip);

		InfoReporting = InfoTextWidget;
	}
	if (!WarningReporting.IsValid())
	{
		TSharedPtr<SErrorText> WarningTextWidget;

		// generate widget
		SAssignNew(WarningTextWidget, SErrorText)
			.BackgroundColor(this, &SVoxelGraphNode::GetWarningColor)
			.ToolTipText(this, &SVoxelGraphNode::GetWarningMsgToolTip);

		WarningReporting = WarningTextWidget;
	}
	if (!ErrorReporting.IsValid())
	{
		TSharedPtr<SErrorText> ErrorTextWidget;

		// generate widget
		SAssignNew(ErrorTextWidget, SErrorText)
			.BackgroundColor(this, &SVoxelGraphNode::GetErrorColor)
			.ToolTipText(this, &SVoxelGraphNode::GetErrorMsgToolTip);

		ErrorReporting = ErrorTextWidget;
	}
	InfoReporting->SetError(InfoMsg);
	WarningReporting->SetError(WarningMsg);
	ErrorReporting->SetError(ErrorMsg);
}

void SVoxelGraphNode::UpdateErrorInfo()
{
	InfoColor = FEditorStyle::GetColor("InfoReporting.BackgroundColor");
	WarningColor = FEditorStyle::GetColor("ErrorReporting.WarningBackgroundColor");
	ErrorColor = FEditorStyle::GetColor("ErrorReporting.BackgroundColor");

	InfoMsg = VoxelNode->InfoMsg;
	WarningMsg = VoxelNode->WarningMsg;
	ErrorMsg = VoxelNode->ErrorMsg;
}

void SVoxelGraphNode::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	OnTextCommitted.ExecuteIfBound(InText, CommitInfo, GraphNode);
	
	UpdateErrorInfo();
	if (ErrorReporting.IsValid())
	{
		ErrorReporting->SetError(ErrorMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelColorGraphNode::Construct(const FArguments& InArgs, UVoxelGraphNode_Base* InNode, UVoxelNode_ColorParameter* InColorNode)
{
	ColorNode = InColorNode;
	SVoxelGraphNode::Construct({}, InNode);
}

void SVoxelColorGraphNode::CreateBelowPinControls(TSharedPtr<SVerticalBox> MainBox)
{
	if (ColorNode)
	{
		const float NegativeHPad = FMath::Max<float>(-Settings->PaddingTowardsNodeEdge, 0.0f);
		const float ExtraPad = 5;
		
		const float ExpressionPreviewSize = 50;
		const float CentralPadding = 5.0f;
		
		LeftNodeBox->AddSlot()
		.Padding(FMargin(NegativeHPad + ExtraPad, 0.0f, 0.0f, 0.0f))
		.AutoHeight()
		[
			SNew(SBox)
			.WidthOverride(ExpressionPreviewSize)
			.HeightOverride(ExpressionPreviewSize)
			[
				SNew(SBorder)
				.Padding(CentralPadding)
				.BorderImage( FEditorStyle::GetBrush("NoBorder") )
				[
					SAssignNew(DefaultValueWidget, SColorBlock)
					.Color(this, &SVoxelColorGraphNode::GetParameterColor)
					.ShowBackgroundForAlpha(true)
					.OnMouseButtonDown(this, &SVoxelColorGraphNode::OnColorBoxClicked)
				]
			]
		];

		DefaultValueWidget->SetCursor(EMouseCursor::Default);
	}
}

FLinearColor SVoxelColorGraphNode::GetParameterColor() const
{
	return ColorNode->Color;
}

void SVoxelColorGraphNode::SetParameterColor(FLinearColor Color)
{
	ColorNode->Color = Color;
}

FReply SVoxelColorGraphNode::OnColorBoxClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SelectedColor = ColorNode->Color;
		TArray<FLinearColor*> LinearColorArray;
		LinearColorArray.Add(&SelectedColor);

		FColorPickerArgs PickerArgs;
		PickerArgs.bIsModal = true;
		PickerArgs.ParentWidget = DefaultValueWidget;
		PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
		PickerArgs.LinearColorArray = &LinearColorArray;
		PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SVoxelColorGraphNode::SetParameterColor);
		PickerArgs.bUseAlpha = true;

		OpenColorPicker(PickerArgs);

		return FReply::Handled();
	}
	else
	{
		return FReply::Unhandled();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelAssetPickerGraphNode::Construct(const FArguments& InArgs, UVoxelGraphNode_Base* InNode, UVoxelAssetPickerNode* InAssetPickerNode)
{
	AssetPickerNode = InAssetPickerNode;
	SVoxelGraphNode::Construct({}, InNode);
}

void SVoxelAssetPickerGraphNode::CreateBelowPinControls(TSharedPtr<SVerticalBox> MainBox)
{
	if (AssetPickerNode)
	{
		const float NegativeHPad = FMath::Max<float>(-Settings->PaddingTowardsNodeEdge, 0.0f);
		const float ExtraPad = 5;
		
		const float CentralPadding = 0.0f;
		
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		TSharedPtr<FAssetThumbnailPool> ThumbnailPool = LevelEditorModule.GetFirstLevelEditor()->GetThumbnailPool();
		
		LeftNodeBox->AddSlot()
		.Padding(FMargin(NegativeHPad + ExtraPad, 0.0f, 0.0f, 0.0f))
		.AutoHeight()
		[
			SNew(SBox)
			[
				SNew(SBorder)
				.Padding(CentralPadding)
				.BorderImage( FEditorStyle::GetBrush("NoBorder") )
				[
					SAssignNew(DefaultValueWidget, SObjectPropertyEntryBox)
					.IsEnabled(true)
					.ObjectPath(TAttribute<FString>::Create(TAttribute<FString>::FGetter::CreateSP(this, &SVoxelAssetPickerGraphNode::GetObjectPath)))
					.AllowedClass(AssetPickerNode->GetAssetClass())
					.OnObjectChanged(FOnSetObject::CreateSP(this, &SVoxelAssetPickerGraphNode::SetAsset))
					.OnShouldFilterAsset(FOnShouldFilterAsset::CreateSP(this, &SVoxelAssetPickerGraphNode::OnShouldFilterAsset))
					.ThumbnailPool(ThumbnailPool)
				]
			]
		];
		DefaultValueWidget->SetCursor(EMouseCursor::Default);
	}
}

void SVoxelAssetPickerGraphNode::SetAsset(const FAssetData& Asset)
{
	AssetPickerNode->SetAsset(Asset.GetAsset());
}

bool SVoxelAssetPickerGraphNode::OnShouldFilterAsset(const FAssetData& Asset)
{
	return AssetPickerNode->ShouldFilterAsset(Asset);
}

FString SVoxelAssetPickerGraphNode::GetObjectPath() const
{
	UObject* PickedAsset = AssetPickerNode->GetAsset();
	return PickedAsset ? PickedAsset->GetPathName() : "";
}