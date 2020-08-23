// Copyright 2020 Phyronnaz

#include "SVoxelGraphPreview.h"
#include "VoxelGraphPreviewSettings.h"
#include "VoxelPlaceableItems/VoxelPlaceableItemManager.h"

#include "Widgets/SCanvas.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Engine/Texture2D.h"
#include "Rendering/DrawElements.h"
#include "Brushes/SlateColorBrush.h"

void SVoxelGraphPreview::Construct(const FArguments& Args)
{
	PreviewSettings = Args._PreviewSettings;
	check(PreviewSettings.IsValid());
	
	TextureBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
	
	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(Size)
		.HeightOverride(Size)
		[
			SNew(SCanvas)
			+ SCanvas::Slot()
			.Size(FVector2D(Size, Size))
			.Position(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateLambda([=]() { return Position; })))
			[
				SNew(SImage)
				.Image(&TextureBrush)
			]
		]
	];
}

void SVoxelGraphPreview::SetTexture(UTexture2D* Texture)
{
	check(Texture);
	
	Texture->Filter = TextureFilter::TF_Nearest;

	TextureBrush.SetResourceObject(Texture);
	TextureBrush.ImageSize.X = Texture->GetSizeX();
	TextureBrush.ImageSize.Y = Texture->GetSizeY();
	TextureBrush.DrawAs = ESlateBrushDrawType::Image;
}

void SVoxelGraphPreview::SetDebugData(const UVoxelPlaceableItemManager* Manager)
{
	if (!Manager)
	{
		DebugLines.Reset();
		DebugPoints.Reset();
		return;
	}
	
	const FVoxelGraphPreviewSettingsWrapper Wrapper(*PreviewSettings);

	const auto GetScreenPosition = [&](const FVector& Point)
	{
		FVector2D ScreenPoint = Wrapper.GetScreenPosition(Point) / Wrapper.Resolution * Size;
		ScreenPoint.Y = Size - ScreenPoint.Y;
		return ScreenPoint;
	};
	
	DebugLines.Reset(Manager->GetDebugLines().Num());
	for (auto& Line : Manager->GetDebugLines())
	{
		DebugLines.Add(FLine{
			GetScreenPosition(Line.Start),
			GetScreenPosition(Line.End),
			Line.Color });
	}
	
	DebugPoints.Reset(Manager->GetDebugPoints().Num());
	for (auto& Point : Manager->GetDebugPoints())
	{
		DebugPoints.Add(FPoint{
			GetScreenPosition(Point.Position),
			Point.Color });
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FReply SVoxelGraphPreview::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const int32 Delta = FMath::Clamp(FMath::RoundToInt(MouseEvent.GetWheelDelta()), -1, 1);

	if (Delta != 0)
	{
		PreviewSettings->ResolutionMultiplierLog -= Delta;
		FPropertyChangedEvent PropertyChangedEvent(UVoxelGraphPreviewSettings::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UVoxelGraphPreviewSettings, ResolutionMultiplierLog)));
		PreviewSettings->PostEditChangeProperty(PropertyChangedEvent);
	}

	return FReply::Handled();
}

FReply SVoxelGraphPreview::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		Position += TransformVector(Inverse(GetCachedGeometry().GetAccumulatedRenderTransform()), MouseEvent.GetCursorDelta());
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SVoxelGraphPreview::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const FVector2D LocalClickPosition = GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		const FVector2D RelativePosition = LocalClickPosition / Size;

		const FVoxelGraphPreviewSettingsWrapper Wrapper(*PreviewSettings);
		PreviewSettings->PreviewedVoxel = Wrapper.Start + FVoxelUtilities::RoundToInt(Wrapper.GetRelativePosition(RelativePosition.X, 1 - RelativePosition.Y) * FVector(Wrapper.Size * Wrapper.Step));

		if (!PreviewSettings->bShowStats && !PreviewSettings->bShowValues)
		{
			// Make sure one of them is toggled if we click the preview
			PreviewSettings->bShowValues = true;
		}
		
		FPropertyChangedEvent PropertyChangedEvent(UVoxelGraphPreviewSettings::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UVoxelGraphPreviewSettings, PreviewedVoxel)));
		PreviewSettings->PostEditChangeProperty(PropertyChangedEvent);
	}
	
	return FReply::Unhandled();
}

FReply SVoxelGraphPreview::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (Position.IsZero())
		{
			return FReply::Unhandled();
		}

		const FVector2D RelativePosition = Position / Size;
		
		const FVoxelGraphPreviewSettingsWrapper Wrapper(*PreviewSettings);
		PreviewSettings->Center -= FVoxelUtilities::RoundToInt(Wrapper.GetRelativePosition(RelativePosition.X, -RelativePosition.Y) * FVector(Wrapper.Size * Wrapper.Step));

		FPropertyChangedEvent PropertyChangedEvent(UVoxelGraphPreviewSettings::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UVoxelGraphPreviewSettings, Center)));
		PreviewSettings->PostEditChangeProperty(PropertyChangedEvent);

		Position = FVector2D::ZeroVector;

		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

FCursorReply SVoxelGraphPreview::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	return FCursorReply::Cursor(EMouseCursor::Crosshairs);
}

int32 SVoxelGraphPreview::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	VOXEL_FUNCTION_COUNTER();
	
	LayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	LayerId++;

	ensure(GetCachedGeometry().Size.X == Size);
	ensure(GetCachedGeometry().Size.Y == Size);

	TArray<FVector2D> Points;
	Points.SetNum(2);
	for (auto& Line : DebugLines)
	{
		Points[0] = Position + Line.Start;
		Points[1] = Position + Line.End;

		if (!MyCullingRect.ContainsPoint(GetCachedGeometry().LocalToAbsolute(Points[0])) && 
			!MyCullingRect.ContainsPoint(GetCachedGeometry().LocalToAbsolute(Points[1])))
		{
			continue;
		}
		
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			Points,
			ESlateDrawEffect::None,
			Line.Color);
	}
	
	FSlateColorBrush Brush(FLinearColor::White);
	for (auto& Point : DebugPoints)
	{
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(Position + Point.Position, FVector2D(2, 2)),
			&Brush,
			ESlateDrawEffect::None,
			Point.Color);
	}

	return LayerId;	
}