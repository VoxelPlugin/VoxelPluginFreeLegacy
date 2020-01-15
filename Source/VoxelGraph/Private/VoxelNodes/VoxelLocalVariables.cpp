// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelLocalVariables.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphErrorReporter.h"
#include "EdGraph/EdGraphNode.h"
#include "Misc/App.h"

UVoxelLocalVariableDeclaration* UVoxelLocalVariableBase::FindDeclarationInArray(const FGuid& VariableGuid, const TArray<UVoxelNode*>& Nodes) const
{
	for (UVoxelNode* Node : Nodes)
	{
		auto* Declaration = Cast<UVoxelLocalVariableDeclaration>(Node);
		if (Declaration && this != Declaration && Declaration->VariableGuid == VariableGuid)
		{
			return Declaration;
		}
	}
	return nullptr;
}

UVoxelLocalVariableDeclaration* UVoxelLocalVariableBase::FindDeclarationInGraph(const FGuid& VariableGuid) const
{
	UVoxelLocalVariableDeclaration* Declaration = nullptr;
	if (Graph)
	{
		Declaration = FindDeclarationInArray(VariableGuid, Graph->AllNodes);
	}
	return Declaration;
}

EVoxelPinCategory UVoxelLocalVariableDeclaration::GetCategory() const
{
	switch (Category)
	{
	case EVoxelPortalNodePinCategory::Boolean:
		return EVoxelPinCategory::Boolean;
	case EVoxelPortalNodePinCategory::Int:
		return EVoxelPinCategory::Int;
	case EVoxelPortalNodePinCategory::Float:
		return EVoxelPinCategory::Float;
	case EVoxelPortalNodePinCategory::Material:
		return EVoxelPinCategory::Material;
	case EVoxelPortalNodePinCategory::Color:
		return EVoxelPinCategory::Color;
	case EVoxelPortalNodePinCategory::Seed:
		return EVoxelPinCategory::Seed;
	default:
		check(false);
		return EVoxelPinCategory::Boolean;
	}
}

void UVoxelLocalVariableDeclaration::SetCategory(EVoxelPinCategory NewCategory)
{
	switch (NewCategory)
	{
	case EVoxelPinCategory::Boolean:
		Category = EVoxelPortalNodePinCategory::Boolean;
		break;
	case EVoxelPinCategory::Int:
		Category = EVoxelPortalNodePinCategory::Int;
		break;
	case EVoxelPinCategory::Float:
		Category = EVoxelPortalNodePinCategory::Float;
		break;
	case EVoxelPinCategory::Material:
		Category = EVoxelPortalNodePinCategory::Material;
		break;
	case EVoxelPinCategory::Color:
		Category = EVoxelPortalNodePinCategory::Color;
		break;
	case EVoxelPinCategory::Seed:
		Category = EVoxelPortalNodePinCategory::Seed;
		break;
	case EVoxelPinCategory::Wildcard:
	case EVoxelPinCategory::Exec:
	default:
		ensure(false);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelLocalVariableDeclaration::PostInitProperties()
{
	Super::PostInitProperties();
	// Init the GUID
	UpdateVariableGuid(false, false);
}

void UVoxelLocalVariableDeclaration::PostLoad()
{
	Super::PostLoad();
	// Init the GUID
	UpdateVariableGuid(false, false);
}

void UVoxelLocalVariableDeclaration::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	// We do not force a guid regen here because this function is used when the Material Editor makes a copy of a material to edit.
	// If we forced a GUID regen, it would cause all of the guids for a material to change every time a material was edited.
	UpdateVariableGuid(false, true);
}

#if WITH_EDITOR
void UVoxelLocalVariableDeclaration::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty)
	{
		const FName PropertyName = PropertyChangedEvent.MemberProperty->GetFName();
		if (PropertyName == GET_MEMBER_NAME_STATIC(UVoxelLocalVariableDeclaration, Name))
		{
			MakeNameUnique();
		}
		else if (PropertyName == GET_MEMBER_NAME_STATIC(UVoxelLocalVariableDeclaration, Category))
		{
			if (Graph && GraphNode)
			{
				GraphNode->ReconstructNode();
				for (auto* Node : Graph->AllNodes)
				{
					auto* Usage = Cast<UVoxelLocalVariableUsage>(Node);
					if (Usage && Usage->Declaration == this && Usage->GraphNode)
					{
						Usage->GraphNode->ReconstructNode();
					}
				}
			}
		}
	}
}
#endif // WITH_EDITOR

EVoxelPinCategory UVoxelLocalVariableDeclaration::GetInputPinCategory(int32 PinIndex) const
{
	return GetCategory();
}


void UVoxelLocalVariableDeclaration::PostCopyNode(const TArray<UVoxelNode*>& CopiedNodes)
{
	Super::PostCopyNode(CopiedNodes);

	// Only force regeneration of Guid if there's already a variable with the same one
	if (FindDeclarationInGraph(VariableGuid))
	{
		// Update Guid, and update the copied usages accordingly
		const FGuid OldGuid = VariableGuid;
		UpdateVariableGuid(true, true);
		for (UVoxelNode* Node : CopiedNodes)
		{
			auto* Usage = Cast<UVoxelLocalVariableUsage>(Node);
			if (Usage && Usage->DeclarationGuid == OldGuid)
			{
				Usage->Declaration = this;
				Usage->DeclarationGuid = VariableGuid;
			}
		}

		// Find a new name
		MakeNameUnique();
	}
	else
	{
		// If there's no existing variable with this GUID, only create it if needed
		UpdateVariableGuid(false, true);
	}
}

FString UVoxelLocalVariableDeclaration::GetEditableName() const
{
	return Name.ToString();
}

void UVoxelLocalVariableDeclaration::SetEditableName(const FString& NewName)
{
	Name = *NewName;
	MakeNameUnique();
#if WITH_EDITOR
	for (UVoxelNode* Node : Graph->AllNodes)
	{
		auto* Usage = Cast<UVoxelLocalVariableUsage>(Node);
		if (Usage && Usage->Declaration == this && Usage->GraphNode)
		{
			Usage->GraphNode->ReconstructNode();
		}
	}
#endif
}

void UVoxelLocalVariableDeclaration::UpdateVariableGuid(bool bForceGeneration, bool bAllowMarkingPackageDirty)
{
	// If we are in the editor, and we don't have a valid GUID yet, generate one.
	if (GIsEditor && !FApp::IsGame())
	{
		if (bForceGeneration || !VariableGuid.IsValid())
		{
			VariableGuid = FGuid::NewGuid();

			if (bAllowMarkingPackageDirty)
			{
				MarkPackageDirty();
			}
		}
	}
}

void UVoxelLocalVariableDeclaration::MakeNameUnique()
{
	if (Graph)
	{
		int32 NameIndex = 1;
		bool bResultNameIndexValid;
		FName PotentialName;

		// Find an available unique name
		do
		{
			PotentialName = Name;
			if (NameIndex != 1)
			{
				PotentialName.SetNumber(NameIndex);
			}

			bResultNameIndexValid = true;
			for (UVoxelNode* Node : Graph->AllNodes)
			{
				auto* OtherDeclaration = Cast<UVoxelLocalVariableDeclaration>(Node);
				if (OtherDeclaration && OtherDeclaration != this && OtherDeclaration->Name == PotentialName)
				{
					bResultNameIndexValid = false;
					break;
				}
			}

			NameIndex++;
		} while (!bResultNameIndexValid);

		Name = PotentialName;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelLocalVariableUsage::PostLoad()
{
	Super::PostLoad();
	if (Selector_DEPRECATED.Input.IsValid())
	{
		MarkPackageDirty();
		Declaration = Selector_DEPRECATED.Input.Get();
		if (Declaration && !DeclarationGuid.IsValid())
		{
			if (!Declaration->VariableGuid.IsValid())
			{
				Declaration->MarkPackageDirty();
				Declaration->VariableGuid = FGuid::NewGuid();
			}
			DeclarationGuid = Declaration->VariableGuid;
		}
	}
}

int32 UVoxelLocalVariableUsage::GetOutputPinsCount() const
{
	return 1;
}

EVoxelPinCategory UVoxelLocalVariableUsage::GetOutputPinCategory(int32 PinIndex) const
{
	return Declaration ? Declaration->GetCategory() : EVoxelPinCategory::Float;
}

FText UVoxelLocalVariableUsage::GetTitle() const
{
	return Declaration ? FText::FromName(Declaration->Name) : FText();
}


void UVoxelLocalVariableUsage::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	if (!IsDeclarationValid())
	{
		ErrorReporter.AddMessageToNode(this, "invalid variable", EVoxelGraphNodeMessageType::FatalError);
	}
}

void UVoxelLocalVariableUsage::PostCopyNode(const TArray<UVoxelNode*>& CopiedNodes)
{
	Super::PostCopyNode(CopiedNodes);

	ensure(!Declaration || Declaration->VariableGuid == DeclarationGuid);

	if (!Declaration)
	{
		// First try to find the declaration in the copied nodes
		Declaration = FindDeclarationInArray(DeclarationGuid, CopiedNodes);
		if (!Declaration)
		{
			// If unsuccessful, try to find it in the whole graph
			Declaration = FindDeclarationInGraph(DeclarationGuid);
		}
		if (Declaration)
		{
			// Save that Declaration change
			MarkPackageDirty();
		}
	}
}

bool UVoxelLocalVariableUsage::IsDeclarationValid() const
{
	// Deleted expressions are marked as pending kill (see FVoxelGraphEditorToolkit::DeleteSelectedNodes)
	return Declaration && !Declaration->IsPendingKill();
}