// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "VoxelColorWheel.generated.h"

class SColorWheel;
 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoxelColorWheelColorChangedEvent, const FLinearColor&, NewColor);

UCLASS()
class VOXELHELPERS_API UVoxelColorWheel : public UWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Voxel Color Wheel")
	FLinearColor Color = FLinearColor::Red;
	
	UPROPERTY(BlueprintAssignable, Category="Voxel Color Wheel")
	FOnVoxelColorWheelColorChangedEvent OnColorChanged;
	
protected:
	//~ Begin UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	//~ End UWidget interface
	 
private:
	TSharedPtr<SColorWheel> ColorWheel;

	void OnValueChanged(FLinearColor NewValue);
	inline FLinearColor GetColor() const { return Color.LinearRGBToHSV(); }
};
