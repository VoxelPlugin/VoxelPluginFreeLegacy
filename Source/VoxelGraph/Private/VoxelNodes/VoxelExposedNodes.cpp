// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelExposedNodes.h"
#include "VoxelNodes/VoxelNodeColors.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelGraphGenerator.h"
#include "EdGraph/EdGraphNode.h"

#if WITH_EDITOR
bool UVoxelExposedNode::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	ensure(false);
	return false;
}
#endif

FLinearColor UVoxelExposedNode::GetColor() const
{
	return FVoxelNodeColors::ExposedNode;
}

FText UVoxelExposedNode::GetTitle() const
{
	return FText::FromName(UniqueName);
}

bool UVoxelExposedNode::CanRenameNode() const
{
	return bCanBeRenamed;
}

FString UVoxelExposedNode::GetEditableName() const
{
	return UniqueName.IsNone() ? "" : UniqueName.ToString();
}

void UVoxelExposedNode::SetEditableName(const FString& NewName)
{
	bCanBeRenamed = false;
	UniqueName = *NewName;
	MakeNameUnique();
	MarkPackageDirty();
#if WITH_EDITOR
	if (GraphNode)
	{
		GraphNode->bCanRenameNode = bCanBeRenamed;
		GraphNode->ReconstructNode();
	}
#endif
}

#if WITH_EDITOR
void UVoxelExposedNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		UniqueName = *FVoxelVariable::SanitizeName(UniqueName.ToString());
		MakeNameUnique();
	}
}
#endif

void UVoxelExposedNode::PostEditImport()
{
	Super::PostEditImport();

	MakeNameUnique();
}

void UVoxelExposedNode::MakeNameUnique()
{
	if (Graph)
	{
		TSet<FName> Names;
		for (auto* Node : Graph->AllNodes)
		{
			auto* ExposedNode = Cast<UVoxelExposedNode>(Node);
			if (ExposedNode && ExposedNode != this && ExposedNode->GetClass() != GetClass())
			{
				Names.Add(ExposedNode->UniqueName);
			}
		}

		int32 Number = UniqueName.GetNumber();
		bool bDirty = false;
		while (Names.Contains(UniqueName))
		{
			UniqueName.SetNumber(++Number);
			bDirty = true;
		}
		if (bDirty)
		{
			MarkPackageDirty();
		}
	}
}


FText UVoxelOptionallyExposedNode::GetTitle() const
{
	const FString ValueString = GetValueString();
	if (bExposeToBP)
	{
		FString Text = UniqueName.ToString().Replace(TEXT("_"), TEXT(" "));
		if (!ValueString.IsEmpty())
		{
			Text += " = " + ValueString;
		}
		return FText::FromString(Text);
	}
	else
	{
		return FText::FromString(ValueString);
	}
}

FLinearColor UVoxelOptionallyExposedNode::GetColor() const
{
	return bExposeToBP ? Super::GetColor() : GetNotExposedColor();
}

void UVoxelOptionallyExposedNode::SetEditableName(const FString& NewName)
{
	if (!NewName.IsEmpty())
	{
		bExposeToBP = true;
	}
	Super::SetEditableName(NewName);
}