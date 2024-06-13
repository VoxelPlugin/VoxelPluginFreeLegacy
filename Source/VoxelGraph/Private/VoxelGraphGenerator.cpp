// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelGraphGenerator.h"
#include "IVoxelGraphEditor.h"
#include "VoxelGraphGlobals.h"
#include "VoxelGraphOutputs.h"
#include "VoxelGraphOutputsConfig.h"
#include "VoxelGraphConstants.h"
#include "VoxelGraphErrorReporter.h"

#include "VoxelNodes/VoxelExecNodes.h"
#include "VoxelNodes/VoxelSeedNodes.h"

#include "VoxelMessages.h"
#include "VoxelNode.h"
#include "VoxelGenerators/VoxelEmptyGenerator.h"
#include "VoxelGenerators/VoxelGeneratorParameters.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "Engine/Texture2D.h"
#include "Misc/ScopeExit.h"
#include "Misc/MessageDialog.h"

#define VOXEL_GRAPH_THUMBNAIL_RES 128

#if WITH_EDITOR
void IVoxelGraphEditor::SetVoxelGraphEditor(TSharedPtr<IVoxelGraphEditor> InVoxelGraphEditor)
{
	ensure(!VoxelGraphEditor.IsValid());
	VoxelGraphEditor = InVoxelGraphEditor;
}

TSharedPtr<IVoxelGraphEditor> IVoxelGraphEditor::VoxelGraphEditor = nullptr;
#endif

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

TMap<uint32, FVoxelGraphOutput> UVoxelGraphGenerator::GetOutputs() const
{
	TMap<uint32, FVoxelGraphOutput> Result;
	for (int32 Index = 0; Index < FVoxelGraphOutput::DefaultOutputs.Num(); Index++)
	{
		Result.Add(Index, FVoxelGraphOutput::DefaultOutputs[Index]);
	}
	if (Outputs)
	{
		const auto& AdditionalOutputs = Outputs->Outputs;
		for (int32 Index = 0; Index < AdditionalOutputs.Num(); Index++)
		{
			Result.Add(FVoxelGraphOutputsIndices::DefaultOutputsMax + Index, AdditionalOutputs[Index]);
		}
	}
	for (auto& It : Result)
	{
		It.Value.Index = It.Key;
	}
	return Result;
}

TArray<FVoxelGraphPermutationArray> UVoxelGraphGenerator::GetPermutations() const
{
	TArray<FVoxelGraphPermutationArray> Result;
	Result.Append(FVoxelGraphOutput::DefaultOutputsPermutations);
	if (Outputs)
	{
		const auto& GraphOutputs = Outputs->Outputs;
		for (int32 Index = 0; Index < GraphOutputs.Num(); Index++)
		{
			FVoxelGraphPermutationArray NewElement;
			NewElement.Add(FVoxelGraphOutputsIndices::DefaultOutputsMax + Index);
			Result.Add(NewElement);

			if (GraphOutputs[Index].Category == EVoxelDataPinCategory::Float)
			{
				FVoxelGraphPermutationArray NewRangeElement;
				NewRangeElement.Add(FVoxelGraphOutputsIndices::DefaultOutputsMax + Index);
				NewRangeElement.Add(FVoxelGraphOutputsIndices::RangeAnalysisIndex);
				Result.Add(NewRangeElement);
			}
		}
	}
	return Result;
}

/////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITORONLY_DATA
UTexture2D* UVoxelGraphGenerator::GetPreviewTexture()
{
	if (!PreviewTexture)
	{
		PreviewTexture = UTexture2D::CreateTransient(VOXEL_GRAPH_THUMBNAIL_RES, VOXEL_GRAPH_THUMBNAIL_RES);
		PreviewTexture->CompressionSettings = TC_HDR;
		PreviewTexture->SRGB = false;

		PreviewTextureSave.SetNumZeroed(VOXEL_GRAPH_THUMBNAIL_RES * VOXEL_GRAPH_THUMBNAIL_RES);

		FTexture2DMipMap& Mip = PreviewTexture->GetPlatformData()->Mips[0];
		
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(Data, PreviewTextureSave.GetData(), PreviewTextureSave.Num() * sizeof(FColor));

		Mip.BulkData.Unlock();
		PreviewTexture->UpdateResource();
	}

	return PreviewTexture;
}

void UVoxelGraphGenerator::SetPreviewTexture(const TArray<FColor>& Colors, int32 Size)
{
	// Do not save thumbnails in the free version, as they can't be right since we can't run graphs
}
#endif

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void UVoxelGraphGenerator::ApplyParameters(const TMap<FName, FString>& Parameters)
{
	for (UVoxelNode* Node : AllNodes)
	{
		Node->ApplyParameters(Parameters);
	}
}

void UVoxelGraphGenerator::GetParameters(TArray<FVoxelGeneratorParameter>& OutParameters) const
{
	for (UVoxelNode* Node : AllNodes)
	{
		Node->GetParameters(OutParameters);
	}
	
	TMap<FName, FVoxelGeneratorParameter> NamesToParameters;
	for (auto& Parameter : OutParameters)
	{
		auto* ExistingParameter = NamesToParameters.Find(Parameter.Id);
		if (!ExistingParameter)
		{
			NamesToParameters.Add(Parameter.Id, Parameter);
			continue;
		}

		if (ExistingParameter->Name != Parameter.Name)
		{
			FVoxelMessages::Error(FString::Printf(
				TEXT("Parameters with same Unique Name but different Display Name: %s vs %s for %s"),
				*Parameter.Name,
				*ExistingParameter->Name,
				*Parameter.Id.ToString()));
		}
		if (ExistingParameter->Type != Parameter.Type)
		{
			FVoxelMessages::Error(FString::Printf(
				TEXT("Parameters with same Unique Name but different type: %s vs %s for %s"),
				*Parameter.Type.ToString(),
				*ExistingParameter->Type.ToString(),
				*Parameter.Id.ToString()));
		}
	}
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelGraphGenerator::GetTransformableInstance()
{
	return GetTransformableInstance({});
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelGraphGenerator::GetTransformableInstance(const TMap<FName, FString>& Parameters)
{
	FVoxelMessages::Info("Running Voxel Graphs require Voxel Plugin Pro");
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelGraphGenerator::PostInitProperties()
{
	Super::PostInitProperties();
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		CreateGraphs();
	}
}

void UVoxelGraphGenerator::PostLoad()
{
	Super::PostLoad();

	CreateGraphs();
	BindUpdateSetterNodes();
}

void UVoxelGraphGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelGraphGenerator, SaveLocation))
		{
			if (!SaveLocation.FilePath.IsEmpty())
			{
				SaveLocation.FilePath = FPaths::ConvertRelativePathToFull(SaveLocation.FilePath);
				FPaths::MakePathRelativeTo(SaveLocation.FilePath, *FPaths::ProjectDir());
			}
		}
		if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelGraphGenerator, Outputs))
		{
			BindUpdateSetterNodes();
			UpdateSetterNodes();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////

UVoxelNode* UVoxelGraphGenerator::ConstructNewNode(UClass* NewNodeClass, const FVector2D& Position, bool bSelectNewNode)
{
	Modify();
	VoxelGraph->Modify();

	UVoxelNode* VoxelNode = NewObject<UVoxelNode>(this, NewNodeClass, NAME_None, RF_Transactional);
	AllNodes.Add(VoxelNode); // To have valid list even without compiling
	MarkPackageDirty();

#if WITH_EDITOR
	VoxelNode->Graph = this;

	// Create the graph node
	check(!VoxelNode->GraphNode);
	IVoxelGraphEditor::GetVoxelGraphEditor()->CreateVoxelGraphNode(VoxelGraph, VoxelNode, bSelectNewNode);
	VoxelNode->GraphNode->NodePosX = Position.X;
	VoxelNode->GraphNode->NodePosY = Position.Y;
#endif // WITH_EDITOR

	return VoxelNode;
}

void UVoxelGraphGenerator::CreateGraphs()
{
	if (auto* VoxelGraphEditor = IVoxelGraphEditor::GetVoxelGraphEditor())
	{
		if (!VoxelGraph)
		{
			VoxelGraph = VoxelGraphEditor->CreateNewVoxelGraph(this);
			VoxelGraph->bAllowDeletion = false;

			// Give the schema a chance to fill out any required nodes (like the results node)
			const UEdGraphSchema* Schema = VoxelGraph->GetSchema();
			Schema->CreateDefaultNodesForGraph(*VoxelGraph);
		}
		if (!VoxelDebugGraph)
		{
			VoxelDebugGraph = VoxelGraphEditor->CreateNewVoxelGraph(this);
			VoxelDebugGraph->bAllowDeletion = false;
		}
	}
}

void UVoxelGraphGenerator::CompileVoxelNodesFromGraphNodes()
{
	if (!ensure(this))
	{
		return;
	}
	if (auto* VoxelGraphEditor = IVoxelGraphEditor::GetVoxelGraphEditor())
	{
		VoxelGraphEditor->CompileVoxelNodesFromGraphNodes(this);
	}
}

void UVoxelGraphGenerator::UpdateSetterNodes()
{
	for (auto& Node : AllNodes)
	{
		if (IsValid(Node))
		{
			if (auto* SetNode = Cast<UVoxelNode_SetNode>(Node))
			{
				SetNode->UpdateSetterNode();
			}
		}
	}
}

void UVoxelGraphGenerator::BindUpdateSetterNodes()
{
	if (Outputs && !Outputs->OnPropertyChanged.IsBoundToObject(this))
	{
		Outputs->OnPropertyChanged.AddUObject(this, &UVoxelGraphGenerator::UpdateSetterNodes);
	}
}

#endif