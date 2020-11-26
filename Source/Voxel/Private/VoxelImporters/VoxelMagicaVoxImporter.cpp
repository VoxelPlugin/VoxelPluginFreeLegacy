// Copyright 2020 Phyronnaz

#include "VoxelImporters/VoxelMagicaVoxImporter.h"
#include "VoxelImporters/ogt_vox.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.inl"
#include "VoxelPlaceableItems/Actors/VoxelAssetActor.h"
#include "VoxelUtilities/VoxelExampleUtilities.h"
#include "VoxelGenerators/VoxelEmptyGenerator.h"
#include "VoxelWorld.h"
#include "VoxelMessages.h"

#include "Engine/World.h"
#include "UObject/Package.h"
#include "Misc/FileHelper.h"

FVoxelMagicaVoxScene::FVoxelMagicaVoxScene(const ogt_vox_scene& Scene)
	: Scene(Scene)
	, Models(Scene.models, Scene.num_models)
	, Instances(Scene.instances, Scene.num_instances)
	, Layers(Scene.layers, Scene.num_layers)
	, Groups(Scene.groups, Scene.num_groups)
{
}

FVoxelMagicaVoxScene::~FVoxelMagicaVoxScene()
{
	VOXEL_FUNCTION_COUNTER();
	ogt_vox_destroy_scene(&Scene);
}

UVoxelMagicaVoxScene* FVoxelMagicaVoxScene::Import(
	UObject* Parent,
	FName NamePrefix,
	EObjectFlags Flags,
	bool bUsePalette,
	TArray<UVoxelDataAsset*>& OutAssets) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Parent)
	{
		Parent = GetTransientPackage();
	}

	TSet<FName> UsedNames;

	for (int32 ModelIndex = 0; ModelIndex < Models.Num(); ModelIndex++)
	{
		const ogt_vox_model* Model = Models[ModelIndex];
		UVoxelDataAsset*& Asset = OutAssets.Emplace_GetRef(nullptr);
		if (!ensure(Model))
		{
			continue;
		}

		FName ModelName = *FString::Printf(TEXT("%s_model%d"), *NamePrefix.ToString(), ModelIndex);
		// Try to find a nicer name by finding an instance using this
		for (const ogt_vox_instance& Instance : Instances)
		{
			if (Instance.model_index != ModelIndex)
			{
				continue;
			}
		
			// Remove any number at the end that would have been added when duplicating the instance in magica voxel
			FString InstanceName = Instance.name;
			while (InstanceName.Len() > 0 && FChar::IsDigit(InstanceName[InstanceName.Len() - 1]))
			{
				InstanceName.RemoveAt(InstanceName.Len() - 1);
			}

			if (InstanceName.IsEmpty())
			{
				continue;
			}

			int32 Number = 0;
			do
			{
				ModelName = *FString::Printf(TEXT("%s_%s"), *NamePrefix.ToString(), *InstanceName);
				ModelName.SetNumber(Number++);
			}
			while (UsedNames.Contains(ModelName));

			UsedNames.Add(ModelName);
			break;
		}
		
		Asset = NewObject<UVoxelDataAsset>(Parent, NamePrefix.IsNone() ? NAME_None : ModelName, Flags);
		
		const auto Data = MakeVoxelShared<FVoxelDataAssetData>();
		ImportModel(*Data, *Model, bUsePalette, Scene.palette);
		Asset->SetData(Data);
	}

	
	const FName SceneName = *FString::Printf(TEXT("%s_scene"), *NamePrefix.ToString());
	auto* SceneObject = NewObject<UVoxelMagicaVoxScene>(Parent, NamePrefix.IsNone() ? NAME_None : SceneName, Flags);

	for (const ogt_vox_instance& Instance : Instances)
	{
		FVoxelMagicaVoxSceneEntry& Entry = SceneObject->Entries.Emplace_GetRef();
		Entry.Name = Instance.name;
		Entry.Asset = ensure(OutAssets.IsValidIndex(Instance.model_index)) ? OutAssets[Instance.model_index] : nullptr;
		Entry.Transform = ConvertTransform(Instance.transform);
		Entry.Layer = ensure(Layers.IsValidIndex(Instance.layer_index)) ? Layers[Instance.layer_index].name : nullptr;

		int32 GroupIndex = Instance.group_index;
		while (GroupIndex != k_invalid_group_index && ensure(Groups.IsValidIndex(GroupIndex)))
		{
			const ogt_vox_group& Group = Groups[GroupIndex];
			Entry.Transform *= ConvertTransform(Group.transform);
			GroupIndex = Group.parent_group_index;
		}
	}

	return SceneObject;
}

bool FVoxelMagicaVoxScene::ImportModel(FVoxelDataAssetData& Asset, int32 ModelIndex, bool bUsePalette) const
{
	if (!Models.IsValidIndex(ModelIndex))
	{
		return false;
	}
	auto* Model = Models[ModelIndex];
	if (!Model)
	{
		return false;
	}

	ImportModel(Asset, *Model, bUsePalette, Scene.palette);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedPtr<FVoxelMagicaVoxScene> FVoxelMagicaVoxScene::LoadScene(const FString& Filename, FString& OutError)
{
	VOXEL_FUNCTION_COUNTER();
	
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *Filename))
	{
		OutError = "Error when opening the file";
		return nullptr;
	}

	const ogt_vox_scene* Scene = ogt_vox_read_scene(Bytes.GetData(), Bytes.Num());
	if (!Scene)
	{
		OutError = "Error when decoding the scene";
		return nullptr;
	}

	return MakeVoxelShared<FVoxelMagicaVoxScene>(*Scene);
}

void FVoxelMagicaVoxScene::ImportModel(FVoxelDataAssetData& Asset, const ogt_vox_model& Model, bool bUsePalette, const ogt_vox_palette& Palette)
{
	VOXEL_FUNCTION_COUNTER();
	
	check(Model.voxel_data);

	Asset.SetSize(FIntVector(Model.size_y, Model.size_x, Model.size_z), true);

	for (uint32 Z = 0; Z < Model.size_z; Z++)
	{
		for (uint32 Y = 0; Y < Model.size_y; Y++)
		{
			for (uint32 X = 0; X < Model.size_x; X++)
			{
				const uint32 Index = X + Model.size_x * Y + Model.size_x * Model.size_y * Z;
				const uint8 Voxel = Model.voxel_data[Index];

				Asset.SetValue(Y, X, Z, Voxel > 0 ? FVoxelValue::Full() : FVoxelValue::Empty());

				FVoxelMaterial Material(ForceInit);
				if (bUsePalette)
				{
					const auto MagicaColor = Palette.color[Voxel];
					// Store the color as a linear color, so edits can be in linear space
					const FColor Color = FLinearColor(FColor(MagicaColor.r, MagicaColor.g, MagicaColor.b, MagicaColor.a)).ToFColor(false);
					Material.SetColor(Color);
				}
				else
				{
					Material.SetSingleIndex(Voxel);
				}

				Asset.SetMaterial(Y, X, Z, Material);
			}
		}
	}
}

FTransform FVoxelMagicaVoxScene::ConvertTransform(const ogt_vox_transform& Transform)
{
	// Tricky: swap X and Y axes
	
	FMatrix Matrix;
	
	Matrix.M[1][1] = Transform.m00;
	Matrix.M[1][0] = Transform.m01;
	Matrix.M[1][2] = Transform.m02;
	Matrix.M[1][3] = Transform.m03;
	
	Matrix.M[0][1] = Transform.m10;
	Matrix.M[0][0] = Transform.m11;
	Matrix.M[0][2] = Transform.m12;
	Matrix.M[0][3] = Transform.m13;
	
	Matrix.M[2][1] = Transform.m20;
	Matrix.M[2][0] = Transform.m21;
	Matrix.M[2][2] = Transform.m22;
	Matrix.M[2][3] = Transform.m23;
	
	Matrix.M[3][1] = Transform.m30;
	Matrix.M[3][0] = Transform.m31;
	Matrix.M[3][2] = Transform.m32;
	Matrix.M[3][3] = Transform.m33;

	return FTransform(Matrix);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelMagicaVoxSceneActor::AVoxelMagicaVoxSceneActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AVoxelMagicaVoxSceneActor::ApplyVoxelSize()
{
	VOXEL_FUNCTION_COUNTER();

	for (auto It = ActorTransforms.CreateIterator(); It; ++It)
	{
		AVoxelAssetActor* Actor = It.Key();
		FTransform Transform = It.Value();
		if (!Actor)
		{
			It.RemoveCurrent();
			continue;
		}

		if (auto* Asset = Cast<UVoxelDataAsset>(Actor->Generator.GetObject()))
		{
			// Transforms in magica voxel are centered
			// Make sure to do it in local space (hence the TransformVector), as else it will do the wrong thing when there's a rotation!
			Transform.AddToTranslation(Transform.TransformVector(-FVector(FVoxelUtilities::DivideFloor(Asset->GetSize(), 2))));
		}
		Transform.SetTranslation(Transform.GetTranslation() * VoxelSize);

		Actor->SetActorTransform(Transform * GetTransform());
		Actor->ClampTransform();
		Actor->MarkPackageDirty();
	}

	// Make sure to do that AFTER fixing up the transforms
	// Else updates are triggered with the wrong settings
	if (VoxelWorld && VoxelWorld->VoxelSize != VoxelSize)
	{
		VoxelWorld->VoxelSize = VoxelSize;
#if WITH_EDITOR
		FPropertyChangedEvent PropertyChangedEvent(AVoxelWorld::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_STATIC(AVoxelWorld, VoxelSize)));
		VoxelWorld->PostEditChangeProperty(PropertyChangedEvent);
#endif
	}
}

void AVoxelMagicaVoxSceneActor::SetScene(UVoxelMagicaVoxScene* Scene)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!Scene)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Scene is null"), this);
		return;
	}

	ActorTransforms.Empty();

	if (!VoxelWorld)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.bDeferConstruction = true;
		
		VoxelWorld = GetWorld()->SpawnActor<AVoxelWorld>(SpawnParameters);
		if (!ensure(VoxelWorld)) return;
		
#if WITH_EDITOR
		VoxelWorld->SetActorLabel(Scene->GetName() + "_VoxelWorld");
#endif
		VoxelWorld->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		VoxelWorld->bCreateWorldAutomatically = true;
		VoxelWorld->VoxelSize = VoxelSize;
		VoxelWorld->RenderType = EVoxelRenderType::Cubic;
		VoxelWorld->bGreedyCubicMesher = true;
		VoxelWorld->bMergeChunks = true;
		VoxelWorld->ChunksClustersSize = 256;
		VoxelWorld->VoxelMaterial = FVoxelExampleUtilities::LoadExampleObject<UMaterialInterface>(TEXT("/Voxel/Examples/Materials/RGB/M_VoxelMaterial_Colors_Greedy"));
		VoxelWorld->Generator = UVoxelEmptyGenerator::StaticClass();
		VoxelWorld->FinishSpawning({}, true);
	}

	for (const FVoxelMagicaVoxSceneEntry& Entry : Scene->Entries)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.bDeferConstruction = true;

		auto* Actor = GetWorld()->SpawnActor<AVoxelAssetActor>(SpawnParameters);
		if (!ensure(Actor)) continue;
		
#if WITH_EDITOR
		Actor->SetActorLabel(Scene->GetName() + "_" + Entry.Name);
#endif
		Actor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		Actor->Generator = Entry.Asset;
		Actor->PreviewWorld = VoxelWorld;
		Actor->bOnlyImportIntoPreviewWorld = true;
		Actor->FinishSpawning({}, true);

		ActorTransforms.Add(Actor, Entry.Transform);
	}

	ApplyVoxelSize();

	FVoxelIntBoxWithValidity SceneBounds;
	for (auto& It : ActorTransforms)
	{
		 SceneBounds += It.Key->AddItemToData(VoxelWorld, nullptr);
	}

	if (SceneBounds.IsValid())
	{
		VoxelWorld->SetRenderOctreeDepth(FVoxelUtilities::GetOctreeDepthContainingBounds<RENDER_CHUNK_SIZE>(SceneBounds.GetBox()));
		if (FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(VoxelWorld->RenderOctreeDepth) <= 2048)
		{
			VoxelWorld->MaxLOD = 0;
			VoxelWorld->bConstantLOD = true;
		}
#if WITH_EDITOR
		VoxelWorld->Toggle();
		ensure(VoxelWorld->IsCreated());
#endif
	}
}

#if WITH_EDITOR
void AVoxelMagicaVoxSceneActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && 
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STATIC(AVoxelMagicaVoxSceneActor, VoxelSize))
	{
		ApplyVoxelSize();
	}
}
#endif