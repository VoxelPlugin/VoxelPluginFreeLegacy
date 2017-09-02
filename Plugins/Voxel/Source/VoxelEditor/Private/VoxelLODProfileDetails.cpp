#include "VoxelLODProfileDetails.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor.h"
#include "IDetailPropertyRow.h"


TArray<FRichCurveEditInfoConst> FLODCurveEditor::GetCurves() const
{
	TArray<FRichCurveEditInfoConst> Curves;
	Curves.Add(&Owner->LODCurve.EditorCurveData);

	return Curves;
}

TArray<FRichCurveEditInfo> FLODCurveEditor::GetCurves()
{
	TArray<FRichCurveEditInfo> Curves;
	Curves.Add(&Owner->LODCurve.EditorCurveData);

	return Curves;
}

void FLODCurveEditor::ModifyOwner()
{
	if (Owner)
	{
		Owner->Modify();
	}
}

TArray<const UObject*> FLODCurveEditor::GetOwners() const
{
	TArray<const UObject*> Owners;
	if (Owner)
	{
		Owners.Add(Owner);
	}

	return Owners;
}

void FLODCurveEditor::MakeTransactional()
{
	if (Owner)
	{
		Owner->SetFlags(Owner->GetFlags() | RF_Transactional);
	}
}

void FLODCurveEditor::OnCurveChanged(const TArray<FRichCurveEditInfo>& ChangedCurveEditInfos)
{
}

bool FLODCurveEditor::IsValidCurve(FRichCurveEditInfo CurveInfo)
{
	return CurveInfo.CurveToEdit == &Owner->LODCurve.EditorCurveData;
}


TSharedRef<IDetailCustomization> FVoxelLODDetails::MakeInstance()
{
	return MakeShareable(new FVoxelLODDetails());
}

void FVoxelLODDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			UVoxelLODProfile* CurrentCaptureActor = Cast<UVoxelLODProfile>(CurrentObject.Get());
			if (CurrentCaptureActor != NULL)
			{
				LODProfile = CurrentCaptureActor;
				break;
			}
		}
	}

	LODCurveEditor = FLODCurveEditor(LODProfile.Get());
	IDetailCategoryBuilder& VoxelCategory = DetailLayout.EditCategory("Voxel");
	TSharedRef<IPropertyHandle> LODCurveHandle = DetailLayout.GetProperty("LODCurve");

	VoxelCategory.AddProperty(LODCurveHandle).CustomWidget()
		.NameContent()
		[
			LODCurveHandle->CreatePropertyNameWidget()
		]
	.ValueContent()
		.MinDesiredWidth(125.f * 3.f)
		[
			SAssignNew(LODCurveWidget, SCurveEditor)
			.ViewMinInput(0.f)
		.ViewMaxInput(70000.f)
		.ViewMinOutput(0)
		.ViewMaxOutput(10)
		.TimelineLength(7000.f)
		.HideUI(false)
		.DesiredSize(FVector2D(1024, 1024))
		.XAxisName(FString(TEXT("Distance")))
		.YAxisName(FString(TEXT("LOD")))
		.DataMinInput(0)
		];

	LODCurveWidget->SetCurveOwner(&LODCurveEditor);
}