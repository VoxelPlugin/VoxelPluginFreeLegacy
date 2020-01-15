// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Styling/SlateBrush.h"

class UTexture2D;

class SVoxelGraphPreview : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVoxelGraphPreview) {};
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	inline UTexture2D* GetTexture() const { return Texture; }
	void SetTexture(UTexture2D* Texture);

private:
	UTexture2D* Texture = nullptr;
	FSlateBrush Brush;
};