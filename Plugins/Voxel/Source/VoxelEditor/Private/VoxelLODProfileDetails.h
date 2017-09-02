// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "SCurveEditor.h"
#include "VoxelLODProfile.h"

struct FLODCurveEditor : public FCurveOwnerInterface
{
	FLODCurveEditor(UVoxelLODProfile* Owner = nullptr) : Owner(Owner)
	{
	};
	/** FCurveOwnerInterface interface */
	virtual TArray<FRichCurveEditInfoConst> GetCurves() const override;
	virtual TArray<FRichCurveEditInfo> GetCurves() override;
	virtual void ModifyOwner() override;
	virtual TArray<const UObject*> GetOwners() const override;
	virtual void MakeTransactional() override;
	virtual void OnCurveChanged(const TArray<FRichCurveEditInfo>& ChangedCurveEditInfos) override;
	virtual bool IsValidCurve(FRichCurveEditInfo CurveInfo) override;
private:
	UVoxelLODProfile* Owner;
};

class FVoxelLODDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	TWeakObjectPtr<UVoxelLODProfile> LODProfile;

	TSharedPtr<SCurveEditor> LODCurveWidget;

	FLODCurveEditor LODCurveEditor;
};
