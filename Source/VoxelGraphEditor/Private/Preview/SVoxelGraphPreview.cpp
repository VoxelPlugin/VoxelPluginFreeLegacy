// Copyright 2020 Phyronnaz

#include "SVoxelGraphPreview.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Engine/Texture2D.h"
#include "RenderUtils.h"

void SVoxelGraphPreview::Construct(const FArguments& InArgs)
{	
	Brush.DrawAs = ESlateBrushDrawType::NoDrawType;
	ChildSlot
	[
		SNew(SScaleBox)
		.Stretch(EStretch::ScaleToFit)
		[
			SNew(SImage)
			.Image(&Brush)
		]
	];
}

void SVoxelGraphPreview::SetTexture(UTexture2D* InTexture)
{
	Texture = InTexture;
	Texture->Filter = TextureFilter::TF_Nearest;

	Brush.SetResourceObject(Texture);
	Brush.ImageSize.X = Texture->GetSizeX();
	Brush.ImageSize.Y = Texture->GetSizeY();
	Brush.DrawAs = ESlateBrushDrawType::Image;
}