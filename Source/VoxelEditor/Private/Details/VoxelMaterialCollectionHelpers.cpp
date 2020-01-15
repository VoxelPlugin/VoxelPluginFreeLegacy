// Copyright 2020 Phyronnaz

#include "VoxelMaterialCollectionHelpers.h"

#include "Materials/MaterialFunction.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionStaticSwitchParameter.h"

#include "AssetRegistryModule.h"
#include "EditorStyleSet.h"
#include "PackageTools.h"
#include "ObjectTools.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "UObject/UObjectHash.h"
#include "FileHelpers.h"

#include "VoxelRender/VoxelMaterialCollection.h"

#define LOCTEXT_NAMESPACE "Voxel"

inline UPackage* CreateOrRetrievePackage(UVoxelMaterialCollection* Collection, const FString& Suffix)
{
	check(Collection);

	UPackage* Package = nullptr;
	UPackage* OuterMost = Collection->GetOutermost();

	const FString PathName = FPackageName::GetLongPackagePath(OuterMost->GetPathName());
	const FString BaseName = FPackageName::GetShortName(OuterMost->GetPathName());
	const FString PackageName = FString::Printf(TEXT("%s/%s_GeneratedMaterials/%s_%s"), *PathName, *BaseName, *BaseName, *Suffix);

	Package = CreatePackage(NULL, *PackageName);
	Package->FullyLoad();
	Package->Modify();

	return Package;
}

inline void CleanPackage(UPackage* Package, TArray<UObject*>& OutObjectsToDelete)
{
	TArray<UObject*> Objects;
	GetObjectsWithOuter(Package, Objects);
	for(UObject* PackageObject : Objects)
	{
		PackageObject->Rename(*FString::Printf(TEXT("PENDING_DELETE_%d_%s"), FMath::Rand(), *PackageObject->GetName()), nullptr, REN_DontCreateRedirectors | REN_NonTransactional);
		OutObjectsToDelete.Add(PackageObject);
	}
}

inline UPackage* GetPackage(UVoxelMaterialCollection* Collection, const FString& Suffix)
{
	UPackage* Package = CreateOrRetrievePackage(Collection, Suffix);

	TArray<UObject*> ObjectsToDelete;
	CleanPackage(Package, ObjectsToDelete);

	if (ObjectsToDelete.Num() > 0)
	{
		UEditorLoadingAndSavingUtils::SavePackages({ Package, Collection->GetOutermost() }, false);

		for (auto& Object : ObjectsToDelete)
		{
			if (auto* Material = Cast<UMaterial>(Object))
			{
				Material->CancelOutstandingCompilation();
			}
		}

		if (ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete) != ObjectsToDelete.Num())
		{
			ObjectsToDelete.Reset();
			CleanPackage(Package, ObjectsToDelete);
			ObjectTools::DeleteObjects(ObjectsToDelete, true); // Ask the user this time
		}

		// In case the package was deleted
		Package = CreateOrRetrievePackage(Collection, Suffix);
	}

	return Package;
}

inline bool ReplaceFunction(const FString& TemplateName, UMaterial* Material, const TMap<FString, UMaterialFunction*>& Functions, FString& OutError)
{
	for (auto& It : Functions)
	{
		FString Description = It.Key;
		UMaterialFunction* Function = It.Value;

		UMaterialExpression** Expression = Material->Expressions.FindByPredicate([&](UMaterialExpression* Expr)
		{
			return Expr->IsA(UMaterialExpressionMaterialFunctionCall::StaticClass()) && Expr->Desc == Description;
		});

		if (!Expression)
		{
			OutError = TemplateName + " is missing a material function call node with description '" + Description + "'";
			return false;
		}

		UMaterialExpressionMaterialFunctionCall* FuncCall = CastChecked<UMaterialExpressionMaterialFunctionCall>(*Expression);
		FuncCall->Modify();
		FuncCall->SetMaterialFunction(Function);
	}
	Material->RebuildExpressionTextureReferences();
	return true;
}
inline void AddPrefixToParameter(FName& Name, const FString& Suffix)
{
	if (Name.ToString().StartsWith("VOXELPARAM_"))
	{
		Name = FName(*(Suffix + Name.ToString()));
	}
}

inline void AddPrefixToInfo(FMaterialParameterInfo& Info, const FString& Suffix)
{
	AddPrefixToParameter(Info.Name, Suffix);
}

struct FInstanceWithSuffix
{
	UMaterialInstanceConstant* Instance;
	FString Suffix;
};

inline void CopyInstancesParameters(const TArray<FInstanceWithSuffix>& InstancesToCopyFrom, UMaterialInstanceConstant* To)
{
	struct FInstanceParameterInfo
	{
		FInstanceWithSuffix Instance;
		FMaterialParameterInfo ParameterInfo;
	};

#define IMPL(Type, InName) \
	{ \
		TArray<FInstanceParameterInfo> InstancesParameterInfo; \
		for (auto& Instance : InstancesToCopyFrom) \
		{ \
			TArray<FMaterialParameterInfo> ParametersInfo; \
			TArray<FGuid> Ids; \
			Instance.Instance->GetAll##InName##ParameterInfo(ParametersInfo, Ids); \
			for(auto& ParameterInfo : ParametersInfo) \
			{ \
				InstancesParameterInfo.Emplace(FInstanceParameterInfo{ Instance, ParameterInfo }); \
			} \
		} \
		for (auto& InstanceParameterInfo : InstancesParameterInfo) \
		{ \
			Type Value{}; \
			if (InstanceParameterInfo.Instance.Instance->Get##InName##ParameterValue(InstanceParameterInfo.ParameterInfo, Value, true)) \
			{ \
				AddPrefixToInfo(InstanceParameterInfo.ParameterInfo, InstanceParameterInfo.Instance.Suffix); \
				To->Set##InName##ParameterValueEditorOnly(InstanceParameterInfo.ParameterInfo, Value); \
			} \
		} \
	}

	IMPL(float, Scalar);
	IMPL(FLinearColor, Vector);
	IMPL(UTexture*, Texture);
#undef IMPL

	FStaticParameterSet Values;
	for (auto& Instance : InstancesToCopyFrom)
	{
		FStaticParameterSet TmpValues;
		Instance.Instance->GetStaticParameterValues(TmpValues);

		for (auto& Parameter : TmpValues.StaticSwitchParameters)
		{
			AddPrefixToInfo(Parameter.ParameterInfo, Instance.Suffix);
		}
		Values.StaticSwitchParameters.Append(TmpValues.StaticSwitchParameters);

		for (auto& Parameter : TmpValues.StaticComponentMaskParameters)
		{
			AddPrefixToInfo(Parameter.ParameterInfo, Instance.Suffix);
		}
		Values.StaticComponentMaskParameters.Append(TmpValues.StaticComponentMaskParameters);

		for (auto& Parameter : TmpValues.TerrainLayerWeightParameters)
		{
			AddPrefixToInfo(Parameter.ParameterInfo, Instance.Suffix);
		}
		Values.TerrainLayerWeightParameters.Append(TmpValues.TerrainLayerWeightParameters);

		for (auto& Parameter : TmpValues.MaterialLayersParameters)
		{
			AddPrefixToInfo(Parameter.ParameterInfo, Instance.Suffix);
		}
		Values.MaterialLayersParameters.Append(TmpValues.MaterialLayersParameters);
	}

	To->UpdateStaticPermutation(Values);
}

inline TArray<uint8> GetIndices(UVoxelMaterialCollection* Collection)
{
	TArray<uint8> Indices;
	for (auto& Material : Collection->Materials)
	{
		if (Material.Children.Num() == 0)
		{
			Indices.Add(Material.Index);
		}
		else
		{
			for (auto& Child : Material.Children)
			{
				Indices.Add(Child.InstanceIndex);
			}
		}
	}

	return Indices;
}

inline bool CheckCollectionForDuplicateIndicesAndNullRefs(UVoxelMaterialCollection* Collection, FString& OutError)
{
	TSet<uint8> AddedIndices;
	const TArray<uint8> Indices = GetIndices(Collection);
	for (auto Index : Indices)
	{
		if (AddedIndices.Contains(Index))
		{
			OutError = "Index " + FString::FromInt(Index) + " is used multiple times";
			return false;
		}
		AddedIndices.Add(Index);
	}
	
	for (auto& Material : Collection->Materials)
	{
		if (!Material.MaterialFunction)
		{
			OutError = "Invalid material function";
			return false;
		}
		for (auto& Child : Material.Children)
		{
			if (!Child.MaterialInstance)
			{
				OutError = "Invalid material instance";
				return false;
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelIntermediateMaterial
{
	enum EKind
	{
		E_MaterialFunction,
		E_MaterialInstance
	};

	EKind const Kind;
	// For both
	UMaterialFunction* const Parent;
	// Only for instance
	UMaterialInstanceConstant* const Instance;
	
	uint8 const Index;
	UPhysicalMaterial* const PhysicalMaterial;

	inline bool IsInstance() const { return Kind == E_MaterialInstance; }
	inline bool IsFunction() const { return Kind == E_MaterialFunction; }

	inline FString GetName() const
	{
		return FString::FromInt(Index);
	}

	FVoxelIntermediateMaterial(UMaterialFunction* Material, uint8 Index, UPhysicalMaterial* PhysicalMaterial)
		: Kind(E_MaterialFunction)
		, Parent(Material)
		, Instance(nullptr)
		, Index(Index)
		, PhysicalMaterial(PhysicalMaterial)
	{
	}
	
	FVoxelIntermediateMaterial(UMaterialFunction* Parent, UMaterialInstanceConstant* Instance, uint8 Index, UPhysicalMaterial* PhysicalMaterial)
		: Kind(E_MaterialInstance)
		, Parent(Parent)
		, Instance(Instance)
		, Index(Index)
		, PhysicalMaterial(PhysicalMaterial)
	{
	}
};

struct FDoubleMaterial
{
	UMaterialFunction* A;
	UMaterialFunction* B;

	FDoubleMaterial(UMaterialFunction* X, UMaterialFunction* Y)
	{
		TArray<UMaterialFunction*, TFixedAllocator<2>> Array = { X, Y };
		Array.Sort([](UMaterialFunction& U, UMaterialFunction& V) { return &U < &V; });
		A = Array[0];
		B = Array[1];
	}
	
	inline bool operator==(const FDoubleMaterial& Other) const
	{
		return A == Other.A && B == Other.B;
	}
};

struct FTripleMaterial
{
	UMaterialFunction* A;
	UMaterialFunction* B;
	UMaterialFunction* C;

	FTripleMaterial(UMaterialFunction* X, UMaterialFunction* Y, UMaterialFunction* Z)
	{
		TArray<UMaterialFunction*, TFixedAllocator<3>> Array = { X, Y, Z };
		Array.Sort([](UMaterialFunction& U, UMaterialFunction& V) { return &U < &V; });
		A = Array[0];
		B = Array[1];
		C = Array[2];
	}
	
	inline bool operator==(const FTripleMaterial& Other) const
	{
		return A == Other.A && B == Other.B && C == Other.C;
	}
};

inline uint32 GetTypeHash(const FDoubleMaterial& O)
{
	return GetTypeHash(O.A) ^ GetTypeHash(O.B);
}

inline uint32 GetTypeHash(const FTripleMaterial& O)
{
	return GetTypeHash(O.A) ^ GetTypeHash(O.B) ^ GetTypeHash(O.C);
}

struct FVoxelIntermediateMaterialsHolder
{
	TMap<FDoubleMaterial, UMaterial*> DoubleMaterials;
	TMap<FTripleMaterial, UMaterial*> TripleMaterials;

	TArray<FVoxelIntermediateMaterial> IntermediateMaterials;

	TArray<UMaterial*> GeneratedMaterials;

	UVoxelMaterialCollection* const Collection;
	FString const Suffix;
	bool const bEnableTessellation;
	UPackage* const Package;
	FString& OutError;

	FVoxelIntermediateMaterialsHolder(UVoxelMaterialCollection* Collection, const FString& Suffix, bool bEnableTessellation, FString& OutError)
		: Collection(Collection)
		, Suffix(Suffix)
		, bEnableTessellation(bEnableTessellation)
		, Package(GetPackage(Collection, Suffix + GetTessellationSuffix()))
		, OutError(OutError)
	{
		for (auto& Material : Collection->Materials)
		{
			if (Material.Children.Num() == 0)
			{
				IntermediateMaterials.Emplace(Material.MaterialFunction, Material.Index, Material.PhysicalMaterial);
			}
			else
			{
				for (auto& Child : Material.Children)
				{
					IntermediateMaterials.Emplace(Material.MaterialFunction, Child.MaterialInstance, Child.InstanceIndex, Child.PhysicalMaterial);
				}
			}
		}
	}
	~FVoxelIntermediateMaterialsHolder()
	{
		FMaterialUpdateContext Context;
		for (auto* Material : GeneratedMaterials)
		{
			Context.AddMaterial(Material);
			Material->PostEditChange();
			Material->ForceRecompileForRendering();
		}
		Package->SetDirtyFlag(true);
		Collection->GetOutermost()->SetDirtyFlag(true);
		UEditorLoadingAndSavingUtils::SavePackages({ Collection->GetOutermost(), Package }, false);
	}

	bool GenerateSingleMaterials()
	{
		GetGenerated().GeneratedSingleMaterials = CreateRefHolder<UVoxelMaterialCollectionSingleRefHolder>();

		for (auto& Material : Collection->Materials)
		{
			// First duplicate the template
			UMaterial* MaterialTemplate = DuplicateTemplate(Collection->SingleMaterialTemplate, Material.MaterialFunction->GetName());
			check(MaterialTemplate);
			GeneratedMaterials.Add(MaterialTemplate);

			// Then replace the template function
			// Duplicate to keep all refs inside the generated materials (w/e)
			UMaterialFunction* const Function = DuplicateFunction(Material.MaterialFunction, "");
			if (!ReplaceFunction("Single Template", MaterialTemplate, { {"INPUT", Function } }, OutError))
			{
				return false;
			}

			if (Material.Children.Num() == 0)
			{
				// Always create an instance to copy parameters
				UMaterialInstanceConstant* Instance = CreateMaterialInstance("Single_" + Material.MaterialFunction->GetName(), MaterialTemplate);
				Instance->PhysMaterial = Material.PhysicalMaterial;
				AddSingleMaterial(Material.Index, Instance);
			}
			else
			{
				for (auto& Child : Material.Children)
				{
					UMaterialInstanceConstant* Instance = CreateMaterialInstance("Single_" + Child.MaterialInstance->GetName(), MaterialTemplate);

					CopyInstancesParameters({ { Child.MaterialInstance, "" } }, Instance);

					Instance->PhysMaterial = Child.PhysicalMaterial;
					AddSingleMaterial(Child.InstanceIndex, Instance);
				}
			}
		}
		return true;
	}

	bool GenerateDoubleMaterials()
	{
		GetGenerated().GeneratedDoubleMaterials = CreateRefHolder<UVoxelMaterialCollectionDoubleRefHolder>();

		// Create the master materials
		auto& Materials = Collection->Materials;
		for (int32 IndexA = 0; IndexA < Materials.Num(); IndexA++)
		{
			for (int32 IndexB = 0; IndexB <= IndexA; IndexB++)
			{
				auto& MaterialA = Materials[IndexA];
				auto& MaterialB = Materials[IndexB];

				if (IndexA == IndexB && MaterialA.Children.Num() == 0 && MaterialB.Children.Num() == 0)
				{
					// No need to create a material for that
					continue;
				}

				// First duplicate the template
				const FString TemplateSuffix = 
					"_" + MaterialA.MaterialFunction->GetName() +
					"_" + MaterialB.MaterialFunction->GetName();
				UMaterial* const MaterialTemplate = DuplicateTemplate(Collection->DoubleMaterialTemplate, TemplateSuffix);
				check(MaterialTemplate);
				GeneratedMaterials.Add(MaterialTemplate);

				UMaterialFunction* const FunctionA = DuplicateFunction(MaterialA.MaterialFunction, "INPUT0_");
				UMaterialFunction* const FunctionB = DuplicateFunction(MaterialB.MaterialFunction, "INPUT1_");

				// Then replace the template functions
				if (!ReplaceFunction("Double Template", MaterialTemplate, { {"INPUT0", FunctionA } , {"INPUT1", FunctionB } }, OutError))
				{
					return false;
				}

				// Finally save the result
				DoubleMaterials.Add({ MaterialA.MaterialFunction, MaterialB.MaterialFunction }, MaterialTemplate);
			}
		}

		// Create instances
		for (int32 IndexA = 0; IndexA < IntermediateMaterials.Num(); IndexA++)
		{
			for (int32 IndexB = 0; IndexB < IndexA; IndexB++)
			{
				const auto& MaterialA = IntermediateMaterials[IndexA];
				const auto& MaterialB = IntermediateMaterials[IndexB];
				UMaterialInterface* const Material = DoubleMaterials.FindChecked({ MaterialA.Parent, MaterialB.Parent });

				const FString Name = "Double_" + MaterialA.GetName() + "_" + MaterialB.GetName();
				UMaterialInstanceConstant* const Instance = CreateMaterialInstance(Name, Material);

				TArray<FInstanceWithSuffix> Instances;
				if (MaterialA.IsInstance())
				{
					Instances.Add({ MaterialA.Instance, "INPUT0_" });
				}
				if (MaterialB.IsInstance())
				{
					Instances.Add({ MaterialB.Instance, "INPUT1_" });
				}

				CopyInstancesParameters(Instances, Instance);

				Instance->PhysMaterial = MaterialA.PhysicalMaterial;
				AddDoubleMaterial({ MaterialA.Index, MaterialB.Index }, Instance);
			}
		}

		return true;
	}

	bool GenerateTripleMaterials()
	{
		GetGenerated().GeneratedTripleMaterials = CreateRefHolder<UVoxelMaterialCollectionTripleRefHolder>();

		// Create the master materials
		auto& Materials = Collection->Materials;
		for (int32 IndexA = 0; IndexA < Materials.Num(); IndexA++)
		{
			for (int32 IndexB = 0; IndexB <= IndexA; IndexB++)
			{
				for (int32 IndexC = 0; IndexC <= IndexB; IndexC++)
				{
					auto& MaterialA = Materials[IndexA];
					auto& MaterialB = Materials[IndexB];
					auto& MaterialC = Materials[IndexC];

					if (IndexA == IndexB && IndexB == IndexC && MaterialA.Children.Num() == 0 && MaterialB.Children.Num() == 0 && MaterialC.Children.Num() == 0)
					{
						// No need to create a material for that
						continue;
					}

					// First duplicate the template
					const FString TemplateSuffix  = 
						"_" + MaterialA.MaterialFunction->GetName() +
						"_" + MaterialB.MaterialFunction->GetName() +
						"_" + MaterialC.MaterialFunction->GetName();
					UMaterial* MaterialTemplate = DuplicateTemplate(Collection->TripleMaterialTemplate, TemplateSuffix);
					check(MaterialTemplate);
					GeneratedMaterials.Add(MaterialTemplate);

					auto* FunctionA = DuplicateFunction(MaterialA.MaterialFunction, "INPUT0_");
					auto* FunctionB = DuplicateFunction(MaterialB.MaterialFunction, "INPUT1_");
					auto* FunctionC = DuplicateFunction(MaterialC.MaterialFunction, "INPUT2_");

					// Then replace the template functions
					if (!ReplaceFunction(
						"Triple Template", 
						MaterialTemplate, 
						{
							{"INPUT0", FunctionA },
							{"INPUT1", FunctionB },
							{"INPUT2", FunctionC }
						}, OutError))
					{
						return false;
					}

					// Finally save the result
					TripleMaterials.Add({ MaterialA.MaterialFunction, MaterialB.MaterialFunction, MaterialC.MaterialFunction }, MaterialTemplate);
				}
			}
		}

		// Create instances
		for (int32 IndexA = 0; IndexA < IntermediateMaterials.Num(); IndexA++)
		{
			for (int32 IndexB = 0; IndexB < IndexA; IndexB++)
			{
				for (int32 IndexC = 0; IndexC < IndexB; IndexC++)
				{
					auto& MaterialA = IntermediateMaterials[IndexA];
					auto& MaterialB = IntermediateMaterials[IndexB];
					auto& MaterialC = IntermediateMaterials[IndexC];
					
					UMaterial* Material = TripleMaterials.FindChecked({ MaterialA.Parent, MaterialB.Parent, MaterialC.Parent });
					check(Material);

					const FString Name = "Triple_" + MaterialA.GetName() + "_" + MaterialB.GetName() + "_" + MaterialC.GetName();
					UMaterialInstanceConstant* Instance = CreateMaterialInstance(Name, Material);

					TArray<FInstanceWithSuffix> Instances;
					if (MaterialA.IsInstance())
					{
						Instances.Add({ MaterialA.Instance, "INPUT0_" });
					}
					if (MaterialB.IsInstance())
					{
						Instances.Add({ MaterialB.Instance, "INPUT1_" });
					}
					if (MaterialC.IsInstance())
					{
						Instances.Add({ MaterialC.Instance, "INPUT2_" });
					}

					CopyInstancesParameters(Instances, Instance);

					Instance->PhysMaterial = MaterialA.PhysicalMaterial;
					AddTripleMaterial({ MaterialA.Index, MaterialB.Index, MaterialC.Index }, Instance);
				}
			}
		}

		return true;
	}

private:
	inline FVoxelMaterialCollectionGenerated& GetGenerated() const
	{
		return bEnableTessellation ? Collection->GeneratedMaterialsTess : Collection->GeneratedMaterials;
	}

	inline void AddSingleMaterial(uint8 Index, UMaterialInterface* Material)
	{
		FMaterialUpdateContext Context;
		Context.AddMaterialInterface(Material);
		Material->PostEditChange();
		Material->ForceRecompileForRendering();
		GetGenerated().GeneratedSingleMaterials->Map.Add(Index, Material);
	}
	inline void AddDoubleMaterial(FVoxelMaterialCollectionDoubleIndex Index, UMaterialInterface* Material)
	{
		FMaterialUpdateContext Context;
		Context.AddMaterialInterface(Material);
		Material->PostEditChange();
		Material->ForceRecompileForRendering();

		const int32 I = FMath::Min(Index.I, Index.J);
		const int32 J = FMath::Max(Index.I, Index.J);

		GetGenerated().GeneratedDoubleMaterials->SortedIndexMap.Add({ I, J }, Index);
		GetGenerated().GeneratedDoubleMaterials->Map.Add(Index, Material);
	}
	inline void AddTripleMaterial(FVoxelMaterialCollectionTripleIndex Index, UMaterialInterface* Material)
	{
		FMaterialUpdateContext Context;
		Context.AddMaterialInterface(Material);
		Material->PostEditChange();
		Material->ForceRecompileForRendering();

		const int32 I = FMath::Min3(Index.I, Index.J, Index.K);
		const int32 K = FMath::Max3(Index.I, Index.J, Index.K);
		const int32 J = Index.I + Index.J + Index.K - I - K;

		GetGenerated().GeneratedTripleMaterials->SortedIndexMap.Add({ I, J, K }, Index);
		GetGenerated().GeneratedTripleMaterials->Map.Add(Index, Material);
	}

	inline FString GetTessellationSuffix() const { return bEnableTessellation ? "_TESS" : ""; }

	template<typename T>
	inline T* CreateRefHolder()
	{
		auto* Generated = NewObject<T>(Package, FName(*(Suffix + "RefHolder" + GetTessellationSuffix())), RF_Public | RF_Standalone);
		FAssetRegistryModule::AssetCreated(Generated);
		return Generated;
	}

	inline UMaterialInstanceConstant* CreateMaterialInstance(FString AssetName, UMaterialInterface* Parent) const
	{
		auto* Factory = NewObject<UMaterialInstanceConstantFactoryNew>(GetTransientPackage());
		check(Factory);
		Factory->InitialParent = Parent;
		auto* Result = CastChecked<UMaterialInstanceConstant>(Factory->FactoryCreateNew(UMaterialInstanceConstant::StaticClass(), Package, FName(*(AssetName + GetTessellationSuffix())), RF_NoFlags, NULL, GWarn));
		FAssetRegistryModule::AssetCreated(Result);

		// Copy template parameters
		if (Collection->TemplateInstanceParameters)
		{
			CopyInstancesParameters({ { Collection->TemplateInstanceParameters, "" } }, Result);
		}
		
		return Result;
	}

	inline UMaterial* DuplicateTemplate(UMaterial* Template, const FString& InSuffix) const
	{
		FObjectDuplicationParameters Parameters(Template, Package);
		Parameters.DestName = FName(*(Template->GetName() + "_" + InSuffix + GetTessellationSuffix()));
		Parameters.FlagMask &= ~(RF_Public | RF_Standalone);
		UMaterial* NewTemplate = CastChecked<UMaterial>(StaticDuplicateObjectEx(Parameters));

		FAssetRegistryModule::AssetCreated(NewTemplate);
		
		FPlatformMisc::CreateGuid(NewTemplate->StateId); // Clears the shader map, can lead to crashes if we don't do that
		NewTemplate->D3D11TessellationMode = bEnableTessellation ? EMaterialTessellationMode::MTM_PNTriangles : EMaterialTessellationMode::MTM_NoTessellation;
		
		return NewTemplate;
	}

	inline UMaterialFunction* DuplicateFunction(UMaterialFunction* MaterialFunctionToDuplicate, const FString& Prefix) const
	{
		FObjectDuplicationParameters Parameters(MaterialFunctionToDuplicate, Package);
		Parameters.DestName = FName(*(Prefix + MaterialFunctionToDuplicate->GetName() + GetTessellationSuffix()));
		Parameters.FlagMask &= ~(RF_Public | RF_Standalone);
		UMaterialFunction* NewFunction = CastChecked<UMaterialFunction>(StaticDuplicateObjectEx(Parameters));

		FAssetRegistryModule::AssetCreated(NewFunction);

		for (auto* Expression : NewFunction->FunctionExpressions)
		{
			if (auto* ParameterA = Cast<UMaterialExpressionParameter>(Expression))
			{
				AddPrefixToParameter(ParameterA->ParameterName, Prefix);
			}
			else if (auto* ParameterB = Cast<UMaterialExpressionTextureSampleParameter>(Expression))
			{
				AddPrefixToParameter(ParameterB->ParameterName, Prefix);
			}
			else if (auto* ParameterC = Cast<UMaterialExpressionFontSampleParameter>(Expression))
			{
				AddPrefixToParameter(ParameterC->ParameterName, Prefix);
			}
		}

		return NewFunction;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelMaterialCollectionHelpers::GenerateSingleMaterials(UVoxelMaterialCollection* Collection, FString& OutError)
{
	if (!Collection->SingleMaterialTemplate)
	{
		OutError = "SingleMaterialTemplate is invalid";
		return false;
	}
	if (!CheckCollectionForDuplicateIndicesAndNullRefs(Collection, OutError))
	{
		return false;
	}
	Collection->GeneratedMaterials.GeneratedSingleMaterials = nullptr;
	Collection->GeneratedMaterialsTess.GeneratedSingleMaterials = nullptr;
	UEditorLoadingAndSavingUtils::SavePackages({ Collection->GetOutermost() }, false);

	if (!FVoxelIntermediateMaterialsHolder(Collection, "Single", false, OutError).GenerateSingleMaterials())
	{
		return false;
	}
	if (Collection->bEnableTessellation)
	{
		if (!FVoxelIntermediateMaterialsHolder(Collection, "Single", true, OutError).GenerateSingleMaterials())
		{
			return false;
		}
	}
	return true;
}

bool FVoxelMaterialCollectionHelpers::GenerateDoubleMaterials(UVoxelMaterialCollection* Collection, FString& OutError)
{
	if (!Collection->DoubleMaterialTemplate)
	{
		OutError = "DoubleMaterialTemplate is invalid";
		return false;
	}
	if (!CheckCollectionForDuplicateIndicesAndNullRefs(Collection, OutError))
	{
		return false;
	}
	Collection->GeneratedMaterials.GeneratedDoubleMaterials = nullptr;
	Collection->GeneratedMaterialsTess.GeneratedDoubleMaterials= nullptr;
	UEditorLoadingAndSavingUtils::SavePackages({ Collection->GetOutermost() }, false);

	if (!FVoxelIntermediateMaterialsHolder(Collection, "Double", false, OutError).GenerateDoubleMaterials())
	{
		return false;
	}
	if (Collection->bEnableTessellation)
	{
		if (!FVoxelIntermediateMaterialsHolder(Collection, "Double", true, OutError).GenerateDoubleMaterials())
		{
			return false;
		}
	}
	return true;
}

bool FVoxelMaterialCollectionHelpers::GenerateTripleMaterials(UVoxelMaterialCollection* Collection, FString& OutError)
{
	if (!Collection->TripleMaterialTemplate)
	{
		OutError = "TripleMaterialTemplate is invalid";
		return false;
	}
	if (!CheckCollectionForDuplicateIndicesAndNullRefs(Collection, OutError))
	{
		return false;
	}
	Collection->GeneratedMaterials.GeneratedTripleMaterials = nullptr;
	Collection->GeneratedMaterialsTess.GeneratedTripleMaterials= nullptr;
	UEditorLoadingAndSavingUtils::SavePackages({ Collection->GetOutermost() }, false);

	if (!FVoxelIntermediateMaterialsHolder(Collection, "Triple", false, OutError).GenerateTripleMaterials())
	{
		return false;
	}
	if (Collection->bEnableTessellation)
	{
		if (!FVoxelIntermediateMaterialsHolder(Collection, "Triple", true, OutError).GenerateTripleMaterials())
		{
			return false;
		}
	}
	return true;
}
#undef LOCTEXT_NAMESPACE