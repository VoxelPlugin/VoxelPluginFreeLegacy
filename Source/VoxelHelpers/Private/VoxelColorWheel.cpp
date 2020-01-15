// Copyright 2020 Phyronnaz

#include "VoxelColorWheel.h"
#include "Widgets/Colors/SColorWheel.h"
 
#define LOCTEXT_NAMESPACE "Voxel"

TSharedRef<SWidget> UVoxelColorWheel::RebuildWidget()
{
	ColorWheel = SNew(SColorWheel)
				.SelectedColor_UObject(this, &UVoxelColorWheel::GetColor)
				.OnValueChanged(FOnLinearColorValueChanged::CreateUObject(this, &UVoxelColorWheel::OnValueChanged));
	
	return ColorWheel.ToSharedRef();
}

void UVoxelColorWheel::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	ColorWheel.Reset();
}
 
#if WITH_EDITOR
const FText UVoxelColorWheel::GetPaletteCategory()
{
	return LOCTEXT("Voxel", "Voxel");
}
#endif

void UVoxelColorWheel::OnValueChanged(FLinearColor NewValue)
{
	Color = NewValue.HSVToLinearRGB();;
	OnColorChanged.Broadcast(Color);
}

#undef LOCTEXT_NAMESPACE
