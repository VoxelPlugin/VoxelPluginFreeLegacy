// Copyright 2020 Phyronnaz

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
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "Engine/Texture2D.h"
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
		for (int32 Index = 0; Index < Outputs->Outputs.Num(); Index++)
		{
			FVoxelGraphPermutationArray NewElement;
			NewElement.Add(FVoxelGraphOutputsIndices::DefaultOutputsMax + Index);
			Result.Add(NewElement);

			FVoxelGraphPermutationArray NewRangeElement;
			NewRangeElement.Add(FVoxelGraphOutputsIndices::DefaultOutputsMax + Index);
			NewRangeElement.Add(FVoxelGraphOutputsIndices::RangeAnalysisIndex);
			Result.Add(NewRangeElement);
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

		FTexture2DMipMap& Mip = PreviewTexture->PlatformData->Mips[0];

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

void UVoxelGraphGenerator::ClearParametersOverrides()
{
	FloatParameters.Reset();
	IntParameters.Reset();
	BoolParameters.Reset();
	ColorParameters.Reset();
	VoxelTextureParameters.Reset();
}

float UVoxelGraphGenerator::GetFloatParameter(const FName& Name, float DefaultValue) const
{
	const auto* Value = FloatParameters.Find(Name);
	return Value ? *Value : DefaultValue;
}

int32 UVoxelGraphGenerator::GetIntParameter(const FName& Name, int32 DefaultValue) const
{
	const auto* Value = IntParameters.Find(Name);
	return Value ? *Value : DefaultValue;
}

bool UVoxelGraphGenerator::GetBoolParameter(const FName& Name, bool DefaultValue) const
{
	const auto* Value = BoolParameters.Find(Name);
	return Value ? *Value : DefaultValue;
}

FLinearColor UVoxelGraphGenerator::GetColorParameter(const FName& Name, FLinearColor DefaultValue) const
{
	const auto* Value = ColorParameters.Find(Name);
	return Value ? *Value : DefaultValue;
}

FVoxelFloatTexture UVoxelGraphGenerator::GetTextureParameter(const FName& Name) const
{
	const auto* Value = VoxelTextureParameters.Find(Name);
	return Value ? *Value : FVoxelFloatTexture();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

TMap<FName, int32> UVoxelGraphGenerator::GetDefaultSeeds() const
{
	TMap<FName, TArray<UVoxelNode_Seed*>> NameToSeedNodes;

	for(auto* Node : AllNodes)
	{
		if (auto* SeedNode = Cast<UVoxelNode_Seed>(Node))
		{
			NameToSeedNodes.FindOrAdd(SeedNode->Name).Add(SeedNode);
		}
	}
	
	TMap<FName, int32> Result;
	for (auto& It : NameToSeedNodes)
	{
		check(It.Value.Num() > 0);
		const FName Name = It.Key;
		const auto& SeedNodes = It.Value;
		const int32 Seed = SeedNodes[0]->DefaultValue;
		for (auto* SeedNode : SeedNodes)
		{
			if (SeedNode->DefaultValue != Seed)
			{
				FVoxelGraphErrorReporter ErrorReporter(this);
				ErrorReporter.AddError("Seeds have the same name, but different default values! Name: " + Name.ToString());
				for (auto* Node : SeedNodes)
				{
					ErrorReporter.AddMessageToNode(Node, "Seed: " + LexToString(Node->DefaultValue), EVoxelGraphNodeMessageType::Info);
				}
				ErrorReporter.Apply(false);
				FVoxelMessages::Error("Voxel Graph Error: GetDefaultSeeds failed!", this);
				return {};
			}
		}
		Result.Add(Name, Seed);
	}
	return Result;
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelGraphGenerator::GetTransformableInstance()
{
	FVoxelMessages::ShowVoxelPluginProError("Voxel Graphs require Voxel Plugin Pro");
	return MakeVoxelShared<FVoxelTransformableEmptyWorldGeneratorInstance>();
}

void UVoxelGraphGenerator::SaveInstance(const FVoxelTransformableWorldGeneratorInstance& Instance, FArchive& Ar) const
{
	FString Path;
	Ar << Path;
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelGraphGenerator::LoadInstance(FArchive& Ar) const
{
	FString Path;
	Ar << Path;

	FVoxelMessages::ShowVoxelPluginProError("Voxel Graphs require Voxel Plugin Pro");
	return MakeVoxelShared<FVoxelTransformableEmptyWorldGeneratorInstance>();
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
	if (SaveLocation.FilePath.IsEmpty())
	{
		SaveLocation.FilePath = FPaths::GameSourceDir() + "GeneratedWorldGenerators/" + GetName() + ".h";
	}
}

void UVoxelGraphGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelGraphGenerator, Outputs))
		{
			BindUpdateSetterNodes();
			UpdateSetterNodes();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////

void UVoxelGraphGenerator::OnPreBeginPIE(bool bIsSimulating)
{
	ClearParametersOverrides();
}

void UVoxelGraphGenerator::OnEndPIE(bool bIsSimulating)
{
	ClearParametersOverrides();
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