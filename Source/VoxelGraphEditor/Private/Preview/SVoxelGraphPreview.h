// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UTexture2D;
class UVoxelPlaceableItemManager;
class UVoxelGraphPreviewSettings;

class SVoxelGraphPreview : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVoxelGraphPreview) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UVoxelGraphPreviewSettings>, PreviewSettings)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& Args);
	void SetTexture(UTexture2D* Texture);
	
	void SetDebugData(const UVoxelPlaceableItemManager* Manager);

public:
	//~ Begin SCompoundWidget Interface
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

	virtual int32 OnPaint(
		const FPaintArgs& Args, 
		const FGeometry& AllottedGeometry, 
		const FSlateRect& MyCullingRect, 
		FSlateWindowElementList& OutDrawElements, 
		int32 LayerId, 
		const FWidgetStyle& InWidgetStyle, 
		bool bParentEnabled) const override;
	//~ End SCompoundWidget Interface

private:
	TWeakObjectPtr<UVoxelGraphPreviewSettings> PreviewSettings;
	FSlateBrush TextureBrush;
	FVector2D Position = FVector2D::ZeroVector;

	struct FLine
	{
		FVector2D Start;
		FVector2D End;
		FLinearColor Color;
	};
	TArray<FLine> DebugLines;

	struct FPoint
	{
		FVector2D Position;
		FLinearColor Color;
	};
	TArray<FPoint> DebugPoints;

	static constexpr float Size = 100.f;
};