// Copyright 2020 Phyronnaz

#include "VoxelGraphConnectionDrawingPolicy.h"
#include "VoxelGraphSchema.h"
#include "VoxelPinCategory.h"

#include "BlueprintEditorSettings.h"
#include "EdGraph/EdGraph.h"

FConnectionDrawingPolicy* FVoxelGraphConnectionDrawingPolicyFactory::CreateConnectionPolicy(const UEdGraphSchema* Schema, int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
	if (Schema->IsA(UVoxelGraphSchema::StaticClass()))
	{
		return new FVoxelGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements, InGraphObj);
	}
	return nullptr;
}

FVoxelGraphConnectionDrawingPolicy::FVoxelGraphConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
	, GraphObj(InGraphObj)
{
	// Don't want to draw ending arrowheads
	ArrowImage = nullptr;
	ArrowRadius = FVector2D::ZeroVector;

	// But we do want to draw midpoint arrowheads
	if (GetDefault<UBlueprintEditorSettings>()->bDrawMidpointArrowsInBlueprints)
	{
		MidpointImage = FEditorStyle::GetBrush(TEXT("Graph.Arrow"));
		MidpointRadius = MidpointImage->ImageSize * ZoomFactor * 0.5f;
	}

	// Cache off the editor options
	AttackColor = Settings->TraceAttackColor;
	SustainColor = Settings->TraceSustainColor;
	ReleaseColor = Settings->TraceReleaseColor;

	AttackWireThickness = Settings->TraceAttackWireThickness;
	SustainWireThickness = Settings->TraceSustainWireThickness;
	ReleaseWireThickness = Settings->TraceReleaseWireThickness;
	DefaultDataWireThickness = Settings->DefaultDataWireThickness;
	DefaultExecutionWireThickness = Settings->DefaultExecutionWireThickness;

	TracePositionBonusPeriod = Settings->TracePositionBonusPeriod;
	TracePositionExponent = Settings->TracePositionExponent;
	AttackHoldPeriod = Settings->TraceAttackHoldPeriod;
	DecayPeriod = Settings->TraceDecayPeriod;
	DecayExponent = Settings->TraceDecayExponent;
	SustainHoldPeriod = Settings->TraceSustainHoldPeriod;
	ReleasePeriod = Settings->TraceReleasePeriod;
	ReleaseExponent = Settings->TraceReleaseExponent;
}

void FVoxelGraphConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	// Draw everything
	FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
}

// Give specific editor modes a chance to highlight this connection or darken non-interesting connections
void FVoxelGraphConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/ FConnectionParams& Params)
{
	Params.WireThickness = DefaultDataWireThickness;
	Params.AssociatedPin1 = OutputPin;
	Params.AssociatedPin2 = InputPin;

	// Get the schema and grab the default color from it
	check(OutputPin);
	check(GraphObj);
	const UEdGraphSchema* Schema = GraphObj->GetSchema();
	
	if (OutputPin->bOrphanedPin || (InputPin && InputPin->bOrphanedPin))
	{
		Params.WireColor = FLinearColor::Red;
	}
	else
	{
		Params.WireColor = Schema->GetPinTypeColor(OutputPin->PinType);
	}
	
	if (FVoxelPinCategory::FromString(OutputPin->PinType.PinCategory) == EVoxelPinCategory::Exec)
	{
		Params.WireThickness = DefaultExecutionWireThickness;
	}

	if (OutputPin->bIsDiffing)
	{
		Params.WireThickness *= 5.f;
		Params.bDrawBubbles = true;
	}

	const bool bDeemphasizeUnhoveredPins = HoveredPins.Num() > 0;

	if (bDeemphasizeUnhoveredPins)
	{
		ApplyHoverDeemphasis(OutputPin, InputPin, /*inout*/ Params.WireThickness, /*inout*/ Params.WireColor);
	}
}