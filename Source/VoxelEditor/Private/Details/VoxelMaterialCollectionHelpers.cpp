// Copyright 2019 Phyronnaz

#include "VoxelMaterialCollectionHelpers.h"
#include "Materials/MaterialFunction.h"
#include "Materials/Material.h"
#include "AssetRegistryModule.h"
#include "EditorStyleSet.h"
#include "PackageTools.h"
#include "ObjectTools.h"
#include "VoxelMaterialCollection.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "UObject/UObjectHash.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionStaticSwitchParameter.h"
#include "FileHelpers.h"

#define LOCTEXT_NAMESPACE "VoxelMaterialCollectionHelpers"

inline UPackage* CreateOrRetrievePackage(UVoxelMaterialCollection* Collection, const FString& Suffix)
{
	check(Collection);

	UPackage* Package = nullptr;
	UPackage* OuterMost = Collection->GetOutermost();

	const FString PathName = FPackageName::GetLongPackagePath(OuterMost->GetPathName());
	const FString BaseName = FPackageName::GetShortName(OuterMost->GetPathName());
	const FString PackageName = FString::Printf(TEXT("%s/MCGM/%s_%s"), *PathName, *BaseName, *Suffix);

	Package = CreatePackage(NULL, *PackageName);
	Package->FullyLoad();
	Package->Modify();

	// Target level filename
	const FString FileName = FPackageName::LongPackageNameToFilename(PackageName);
	// This is a hack to avoid save file dialog when we will be saving HLOD map package
	//Package->FileName = FName(*FileName);

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

inline bool ReplaceFunction(UMaterial* Material, const TMap<FString, UMaterialFunction*>& Functions, FString& OutError)
{
	for (auto& It : Functions)
	{
		FString Description = It.Key;
		UMaterialFunction* Function = It.Value;

		UMaterialExpression** Expression = Material->Expressions.FindByPredicate([&](UMaterialExpression* Expression)
		{
			return Expression->IsA(UMaterialExpressionMaterialFunctionCall::StaticClass()) && Expression->Desc == Description;
		});

		if (!Expression)
		{
			OutError = Material->GetName() + " is missing a material function call with description '" + Description + "'";
			return false;
		}

		UMaterialExpressionMaterialFunctionCall* FuncCall = CastChecked<UMaterialExpressionMaterialFunctionCall>(*Expression);
		FuncCall->Modify();
		FuncCall->SetMaterialFunction(Function);
	}
	Material->ForceRecompileForRendering();
	return true;
}

inline FName GetIsTessellationEnabledName(const FString& Prefix)
{
	return FName(*(Prefix + "IsTessellationEnabled"));
}

inline void AddSuffixToInfo(FMaterialParameterInfo& Info, const FString& Suffix)
{
	Info.Name = FName(*(Suffix + Info.Name.ToString()));
}

inline void CopyInstanceParameters(const TArray<UMaterialInstanceConstant*>& Froms, UMaterialInstanceConstant* To, const TArray<FString>& Suffixes, const TArray<FStaticSwitchParameter>& StaticSwitchParameters = TArray<FStaticSwitchParameter>())
{
	check(Froms.Num() == Suffixes.Num());

	struct FTmpStruct
	{
		UMaterialInstanceConstant* From;
		FString Suffix;
		FMaterialParameterInfo Info;
	};

#define IMPL(Type, InName) \
	{ \
		TArray<FTmpStruct> Infos; \
		for (int Index = 0; Index < Froms.Num(); Index++) \
		{ \
			auto* From = Froms[Index]; \
			auto& Suffix = Suffixes[Index]; \
			TArray<FMaterialParameterInfo> TmpInfos; \
			TArray<FGuid> Ids; \
			From->GetAll##InName##ParameterInfo(TmpInfos, Ids); \
			for(auto& Info : TmpInfos) \
			{ \
				Infos.Emplace(FTmpStruct{ From, Suffix, Info }); \
			} \
		} \
		for (auto& Info : Infos) \
		{ \
			Type Value{}; \
			if (Info.From->Get##InName##ParameterValue(Info.Info, Value, true)) \
			{ \
				AddSuffixToInfo(Info.Info, Info.Suffix); \
				To->Set##InName##ParameterValueEditorOnly(Info.Info, Value); \
			} \
		} \
	}

	IMPL(float, Scalar);
	IMPL(FLinearColor, Vector);
	IMPL(UTexture*, Texture);
#undef IMPL

	FStaticParameterSet Values;
	for (int Index = 0; Index < Froms.Num(); Index++)
	{
		auto* From = Froms[Index];
		auto& Suffix = Suffixes[Index];

		FStaticParameterSet TmpValues;
		From->GetStaticParameterValues(TmpValues);

		for (auto& Parameter : TmpValues.StaticSwitchParameters)
		{
			AddSuffixToInfo(Parameter.ParameterInfo, Suffix);
		}
		Values.StaticSwitchParameters.Append(TmpValues.StaticSwitchParameters);

		for (auto& Parameter : TmpValues.StaticComponentMaskParameters)
		{
			AddSuffixToInfo(Parameter.ParameterInfo, Suffix);
		}
		Values.StaticComponentMaskParameters.Append(TmpValues.StaticComponentMaskParameters);

		for (auto& Parameter : TmpValues.TerrainLayerWeightParameters)
		{
			AddSuffixToInfo(Parameter.ParameterInfo, Suffix);
		}
		Values.TerrainLayerWeightParameters.Append(TmpValues.TerrainLayerWeightParameters);

		for (auto& Parameter : TmpValues.MaterialLayersParameters)
		{
			AddSuffixToInfo(Parameter.ParameterInfo, Suffix);
		}
		Values.MaterialLayersParameters.Append(TmpValues.MaterialLayersParameters);
	}
	Values.StaticSwitchParameters.Append(StaticSwitchParameters);
	To->UpdateStaticPermutation(Values);
	To->ForceRecompileForRendering();
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
		Package->SetDirtyFlag(true);
		Collection->GetOutermost()->SetDirtyFlag(true);
		UEditorLoadingAndSavingUtils::SavePackages({ Collection->GetOutermost(), Package }, false);
	}

	bool GenerateSingleMaterials()
	{
		CreateGenerated(GetGenerated().GeneratedSingleMaterials);

		for (auto& Material : Collection->Materials)
		{
			// First duplicate the template
			UMaterial* MaterialTemplate = DuplicateTemplate(Collection->SingleMaterialTemplate, Material.MaterialFunction->GetName());
			check(MaterialTemplate);

			// Then replace the template function
			if (!ReplaceFunction(MaterialTemplate, { {"INPUT", Material.MaterialFunction } }, OutError))
			{
				return false;
			}

			if (Material.Children.Num() == 0)
			{
				MaterialTemplate->PhysMaterial = Material.PhysicalMaterial;
				AddSingleMaterial(Material.Index, MaterialTemplate);
			}
			else
			{
				for (auto& Child : Material.Children)
				{
					UMaterialInstanceConstant* Instance = CreateMaterialInstance("Single_" + Child.MaterialInstance->GetName(), MaterialTemplate);
					
					TArray<FStaticSwitchParameter> StaticSwitchParameters;
					StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo(GetIsTessellationEnabledName("")), bEnableTessellation, true, FGuid()));
					CopyInstanceParameters({ Child.MaterialInstance }, Instance, { "" }, StaticSwitchParameters);

					Instance->PhysMaterial = Child.PhysicalMaterial;
					AddSingleMaterial(Child.InstanceIndex, Instance);
				}
			}
		}
		return true;
	}

	bool GenerateDoubleMaterials()
	{
		CreateGenerated(GetGenerated().GeneratedDoubleMaterials);

		auto& Materials = Collection->Materials;
		for (int IndexA = 0; IndexA < Materials.Num(); IndexA++)
		{
			for (int IndexB = 0; IndexB <= IndexA; IndexB++)
			{
				auto& MaterialA = Materials[IndexA];
				auto& MaterialB = Materials[IndexB];

				if (IndexA == IndexB && MaterialA.Children.Num() == 0 && MaterialB.Children.Num() == 0)
				{
					// No need to create a material for that
					continue;
				}

				// First duplicate the template
				UMaterial* MaterialTemplate = DuplicateTemplate(Collection->DoubleMaterialTemplate,
					"_" + MaterialA.MaterialFunction->GetName() +
					"_" + MaterialB.MaterialFunction->GetName());
				check(MaterialTemplate);

				auto* FunctionA = DuplicateFunction(MaterialA.MaterialFunction, "INPUT0_");
				auto* FunctionB = DuplicateFunction(MaterialB.MaterialFunction, "INPUT1_");

				// Then replace the template functions
				if (!ReplaceFunction(MaterialTemplate, { {"INPUT0", FunctionA } , {"INPUT1", FunctionB} }, OutError))
				{
					return false;
				}

				// Finally save the result
				DoubleMaterials.Add({ MaterialA.MaterialFunction, MaterialB.MaterialFunction }, MaterialTemplate);
			}
		}

		for (int IndexA = 0; IndexA < IntermediateMaterials.Num(); IndexA++)
		{
			for (int IndexB = 0; IndexB < IndexA; IndexB++)
			{
				auto* MaterialAPtr = &IntermediateMaterials[IndexA];
				auto* MaterialBPtr = &IntermediateMaterials[IndexB];

				auto** MaterialPtr = DoubleMaterials.Find({ MaterialAPtr->Parent, MaterialBPtr->Parent });
				if (!MaterialPtr)
				{
					Swap(MaterialAPtr, MaterialBPtr);
					MaterialPtr = DoubleMaterials.Find({ MaterialAPtr->Parent, MaterialBPtr->Parent });
				}

				auto* Material = *MaterialPtr;
				check(Material);
				auto& MaterialA = *MaterialAPtr;
				auto& MaterialB = *MaterialBPtr;

				int I = FMath::Min(MaterialA.Index, MaterialB.Index);
				int J = FMath::Max(MaterialA.Index, MaterialB.Index);
				auto DoubleIndex = FVoxelMaterialCollectionDoubleIndex(I, J);

				UMaterialInstanceConstant* Instance = CreateMaterialInstance("Double_" + MaterialA.GetName() + "_" + MaterialB.GetName(), Material);

				TArray<UMaterialInstanceConstant*> Instances;
				TArray<FString> Suffixes;
				if (MaterialA.IsInstance())
				{
					Instances.Add(MaterialA.Instance);
					Suffixes.Add("INPUT0_");
				}
				if (MaterialB.IsInstance())
				{
					Instances.Add(MaterialB.Instance);
					Suffixes.Add("INPUT1_");
				}

				TArray<FStaticSwitchParameter> StaticSwitchParameters;
				for (auto& ParamSuffix : Suffixes)
				{
					StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo(GetIsTessellationEnabledName(ParamSuffix)), bEnableTessellation, true, FGuid()));
				}
				StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo("INPUT0IsA"), MaterialA.Index == I, true, FGuid()));
				StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo("INPUT0IsB"), MaterialA.Index == J, true, FGuid()));

				CopyInstanceParameters(Instances, Instance, Suffixes, StaticSwitchParameters);

				Instance->PhysMaterial = MaterialA.PhysicalMaterial;
				AddDoubleMaterial(DoubleIndex, Instance);
			}
		}

		return true;
	}

	bool GenerateTripleMaterials()
	{
		CreateGenerated(GetGenerated().GeneratedTripleMaterials);

		auto& Materials = Collection->Materials;
		for (int IndexA = 0; IndexA < Materials.Num(); IndexA++)
		{
			for (int IndexB = 0; IndexB <= IndexA; IndexB++)
			{
				for (int IndexC = 0; IndexC <= IndexB; IndexC++)
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
					UMaterial* MaterialTemplate = DuplicateTemplate(Collection->TripleMaterialTemplate,
						"_" + MaterialA.MaterialFunction->GetName() +
						"_" + MaterialB.MaterialFunction->GetName() +
						"_" + MaterialC.MaterialFunction->GetName());
					check(MaterialTemplate);

					auto* FunctionA = DuplicateFunction(MaterialA.MaterialFunction, "INPUT0_");
					auto* FunctionB = DuplicateFunction(MaterialB.MaterialFunction, "INPUT1_");
					auto* FunctionC = DuplicateFunction(MaterialC.MaterialFunction, "INPUT2_");

					// Then replace the template functions
					if (!ReplaceFunction(MaterialTemplate, { {"INPUT0", FunctionA } , {"INPUT1", FunctionB}, {"INPUT2", FunctionC} }, OutError))
					{
						return false;
					}

					// Finally save the result
					TripleMaterials.Add({ MaterialA.MaterialFunction, MaterialB.MaterialFunction, MaterialC.MaterialFunction }, MaterialTemplate);
				}
			}
		}

		for (int IndexA = 0; IndexA < IntermediateMaterials.Num(); IndexA++)
		{
			for (int IndexB = 0; IndexB < IndexA; IndexB++)
			{
				for (int IndexC = 0; IndexC < IndexB; IndexC++)
				{
					auto* APtr = &IntermediateMaterials[IndexA];
					auto* BPtr = &IntermediateMaterials[IndexB];
					auto* CPtr = &IntermediateMaterials[IndexC];

					auto* AOPtr = APtr;
					auto* BOPtr = BPtr;
					auto* COPtr = CPtr;
					UMaterial** MaterialPtr = nullptr;

#define IMPL(X, Y, Z) \
					if (!MaterialPtr) \
					{ \
						APtr = X##OPtr; \
						BPtr = Y##OPtr; \
						CPtr = Z##OPtr; \
						MaterialPtr = TripleMaterials.Find({ APtr->Parent, BPtr->Parent, CPtr->Parent }); \
					}
					IMPL(A, B, C);
					IMPL(A, C, B);
					IMPL(B, A, C);
					IMPL(B, C, A);
					IMPL(C, A, B);
					IMPL(C, B, A);
#undef IMPL

					auto* Material = *MaterialPtr;
					check(Material);
					auto& MaterialA = *APtr;
					auto& MaterialB = *BPtr;
					auto& MaterialC = *CPtr;

					int I = FMath::Min(MaterialA.Index, FMath::Min(MaterialB.Index, MaterialC.Index));
					int K = FMath::Max(MaterialA.Index, FMath::Max(MaterialB.Index, MaterialC.Index));
					int J = MaterialA.Index + MaterialB.Index + MaterialC.Index - I - K;

					auto TripleIndex = FVoxelMaterialCollectionTripleIndex(I, J, K);

					UMaterialInstanceConstant* Instance = CreateMaterialInstance("Triple_" + MaterialA.GetName() + "_" + MaterialB.GetName() + "_" + MaterialC.GetName(), Material);

					TArray<UMaterialInstanceConstant*> Instances;
					TArray<FString> Suffixes;
					if (MaterialA.IsInstance())
					{
						Instances.Add(MaterialA.Instance);
						Suffixes.Add("INPUT0_");
					}
					if (MaterialB.IsInstance())
					{
						Instances.Add(MaterialB.Instance);
						Suffixes.Add("INPUT1_");
					}
					if (MaterialC.IsInstance())
					{
						Instances.Add(MaterialC.Instance);
						Suffixes.Add("INPUT2_");
					}

					TArray<FStaticSwitchParameter> StaticSwitchParameters;
					for (auto& ParamSuffix : Suffixes)
					{
						StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo(GetIsTessellationEnabledName(ParamSuffix)), bEnableTessellation, true, FGuid()));
					}
					StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo("INPUT0IsA"), MaterialA.Index == I, true, FGuid()));
					StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo("INPUT0IsB"), MaterialA.Index == J, true, FGuid()));
					StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo("INPUT0IsC"), MaterialA.Index == K, true, FGuid()));
					StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo("INPUT1IsA"), MaterialB.Index == I, true, FGuid()));
					StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo("INPUT1IsB"), MaterialB.Index == J, true, FGuid()));
					StaticSwitchParameters.Add(FStaticSwitchParameter(FMaterialParameterInfo("INPUT1IsC"), MaterialB.Index == K, true, FGuid()));

					CopyInstanceParameters(Instances, Instance, Suffixes, StaticSwitchParameters);

					Instance->PhysMaterial = MaterialA.PhysicalMaterial;
					AddTripleMaterial(TripleIndex, Instance);
				}
			}
		}

		return true;
	}

private:
	inline FVoxelMaterialCollectionGenerated& GetGenerated()
	{
		return bEnableTessellation ? Collection->GeneratedMaterialsTess : Collection->GeneratedMaterials;
	}

	inline void AddSingleMaterial(uint8 Index, UMaterialInterface* Material)
	{
		GetGenerated().GeneratedSingleMaterials->Map.Add(Index, Material);
	}
	inline void AddDoubleMaterial(FVoxelMaterialCollectionDoubleIndex Index, UMaterialInterface* Material)
	{
		GetGenerated().GeneratedDoubleMaterials->Map.Add(Index, Material);
	}
	inline void AddTripleMaterial(FVoxelMaterialCollectionTripleIndex Index, UMaterialInterface* Material)
	{
		GetGenerated().GeneratedTripleMaterials->Map.Add(Index, Material);
	}

	inline FString GetTessellationSuffix() const { return bEnableTessellation ? "_TESS" : ""; }

	template<typename T>
	inline void CreateGenerated(T*& Generated)
	{
		check(!Generated);
		Generated = NewObject<T>(Package, FName(*(Suffix + "RefHolder" + GetTessellationSuffix())), RF_Public | RF_Standalone);
		FAssetRegistryModule::AssetCreated(Generated);
	}

	inline UMaterialInstanceConstant* CreateMaterialInstance(FString AssetName, UMaterialInterface* Parent)
	{
		auto* Factory = NewObject<UMaterialInstanceConstantFactoryNew>(GetTransientPackage());
		check(Factory);
		Factory->InitialParent = Parent;
		auto* Result = Factory->FactoryCreateNew(UMaterialInstanceConstant::StaticClass(), Package, FName(*(AssetName + GetTessellationSuffix())), RF_NoFlags, NULL, GWarn);
		FAssetRegistryModule::AssetCreated(Result);

		return CastChecked<UMaterialInstanceConstant>(Result);
	}

	inline UMaterial* DuplicateTemplate(UMaterial* Template, const FString& InSuffix)
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

	inline UMaterialFunction* DuplicateFunction(UMaterialFunction* MaterialFunctionToDuplicate, const FString& Prefix)
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
				ParameterA->ParameterName = FName(*(Prefix + ParameterA->ParameterName.ToString()));
			}
			else if (auto* ParameterB = Cast<UMaterialExpressionTextureSampleParameter>(Expression))
			{
				ParameterB->ParameterName = FName(*(Prefix + ParameterB->ParameterName.ToString()));
			}
			else if (auto* ParameterC = Cast<UMaterialExpressionFontSampleParameter>(Expression))
			{
				ParameterC->ParameterName = FName(*(Prefix + ParameterC->ParameterName.ToString()));
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