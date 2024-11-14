// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelRender/VoxelMaterialExpressions.h"
#include "VoxelContainers/VoxelStaticArray.h"

#if VOXEL_ENGINE_VERSION >= 504
#include "LandscapeUtils.h"
#include "MaterialHLSLTree.h"
#include "MaterialHLSLGenerator.h"
#endif

#include "MaterialCompiler.h"
#include "Materials/MaterialFunction.h"
#include "Materials/HLSLMaterialTranslator.h"

#if VOXEL_ENGINE_VERSION >= 506
#error "Check implementation"
#elif VOXEL_ENGINE_VERSION >= 504
#if WITH_EDITOR
namespace UE::Landscape
{
	bool IsMobileWeightmapTextureArrayEnabled()
	{
		static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("landscape.MobileWeightTextureArray"));
		return CVar->GetInt() != 0;	
	}
}

bool UMaterialExpressionLandscapeLayerBlend::GenerateHLSLExpression(FMaterialHLSLGenerator& Generator, UE::HLSLTree::FScope& Scope, int32 OutputIndex, UE::HLSLTree::FExpression const*& OutExpression) const
{
	using namespace UE::HLSLTree;

	bool bNeedsRenormalize = false;
	FTree& Tree = Generator.GetTree();
	const FExpression* ConstantOne = Tree.NewConstant(1.f);
	const FExpression* WeightSumExpression = Tree.NewConstant(0.f);
	TArray<const FExpression*> WeightExpressions;

	WeightExpressions.Empty(Layers.Num());

	for (int32 LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++)
	{
		WeightExpressions.Add(nullptr);

		const FLayerBlendInput& Layer = Layers[LayerIdx];

		// LB_AlphaBlend layers are blended last
		if (Layer.BlendType != LB_AlphaBlend)
		{
			const FExpression* WeightExpression = nullptr;
			const bool bTextureArrayEnabled = UE::Landscape::IsMobileWeightmapTextureArrayEnabled();
			verify(GenerateStaticTerrainLayerWeightExpression(Layer.LayerName, Layer.PreviewWeight, bTextureArrayEnabled, Generator, WeightExpression));

			if (WeightExpression)
			{
				switch (Layer.BlendType)
				{
				case LB_WeightBlend:
				{
					// Store the weight plus accumulate the sum of all weights so far
					WeightExpressions[LayerIdx] = WeightExpression;
					WeightSumExpression = Tree.NewAdd(WeightSumExpression, WeightExpression);
				}
				break;
				case LB_HeightBlend:
				{
					bNeedsRenormalize = true;

					// Modify weight with height
					const FExpression* HeightExpression = Layer.HeightInput.AcquireHLSLExpressionOrConstant(Generator, Scope, Layer.ConstHeightInput);
					const FExpression* ModifiedWeight = Tree.NewLerp(Tree.NewConstant(-1.f), ConstantOne, WeightExpression);
					ModifiedWeight = Tree.NewAdd(ModifiedWeight, HeightExpression);
					ModifiedWeight = Tree.NewMin(Tree.NewMax(ModifiedWeight, Tree.NewConstant(0.0001f)), ConstantOne);

					// Store the final weight plus accumulate the sum of all weights so far
					WeightExpressions[LayerIdx] = ModifiedWeight;
					WeightSumExpression = Tree.NewAdd(WeightSumExpression, ModifiedWeight);
				}
				break;
				}
			}
		}
	}

	const FExpression* InvWeightSumExpression = Tree.NewDiv(ConstantOne, WeightSumExpression);
	OutExpression = Tree.NewConstant(0.f);

	for (int32 LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++)
	{
		const FLayerBlendInput& Layer = Layers[LayerIdx];

		if (WeightExpressions[LayerIdx])
		{
			const FExpression* LayerExpression = Layer.LayerInput.AcquireHLSLExpressionOrConstant(Generator, Scope, FVector3f(Layer.ConstLayerInput));

			if (bNeedsRenormalize)
			{
				// Renormalize the weights as our height modification has made them non-uniform
				OutExpression = Tree.NewAdd(OutExpression, Tree.NewMul(LayerExpression, Tree.NewMul(InvWeightSumExpression, WeightExpressions[LayerIdx])));
			}
			else
			{
				// No renormalization is necessary, so just add the weights
				OutExpression = Tree.NewAdd(OutExpression, Tree.NewMul(LayerExpression, WeightExpressions[LayerIdx]));
			}
		}
	}

	// Blend in LB_AlphaBlend layers
	for (const FLayerBlendInput& Layer : Layers)
	{
		if (Layer.BlendType == LB_AlphaBlend)
		{
			const FExpression* WeightExpression = nullptr;
			const bool bTextureArrayEnabled = UE::Landscape::IsMobileWeightmapTextureArrayEnabled();
			verify(GenerateStaticTerrainLayerWeightExpression(Layer.LayerName, Layer.PreviewWeight, bTextureArrayEnabled, Generator, WeightExpression));

			if (WeightExpression)
			{
				const FExpression* LayerExpression = Layer.LayerInput.AcquireHLSLExpressionOrConstant(Generator, Scope, FVector3f(Layer.ConstLayerInput));

				// Blend in the layer using the alpha value
				OutExpression = Tree.NewLerp(OutExpression, LayerExpression, WeightExpression);
			}
		}
	}

	return true;
}

bool UMaterialExpressionLandscapeLayerSample::GenerateHLSLExpression(FMaterialHLSLGenerator& Generator, UE::HLSLTree::FScope& Scope, int32 OutputIndex, UE::HLSLTree::FExpression const*& OutExpression) const
{
	const bool bTextureArrayEnabled = UE::Landscape::IsMobileWeightmapTextureArrayEnabled();
	return GenerateStaticTerrainLayerWeightExpression(ParameterName, PreviewWeight, bTextureArrayEnabled, Generator, OutExpression);
}

bool UMaterialExpressionLandscapeLayerSwitch::GenerateHLSLExpression(FMaterialHLSLGenerator& Generator, UE::HLSLTree::FScope& Scope, int32 OutputIndex, UE::HLSLTree::FExpression const*& OutExpression) const
{
	using namespace UE::HLSLTree;

	const FExpression* Inputs[] = {
		LayerNotUsed.TryAcquireHLSLExpression(Generator, Scope),
		LayerUsed.TryAcquireHLSLExpression(Generator, Scope)
	};

	OutExpression = Generator.GetTree().NewExpression<Material::FExpressionLandscapeLayerSwitch>(Inputs, ParameterName, PreviewUsed!=0);
	return OutExpression != nullptr;
}

bool UMaterialExpressionLandscapeVisibilityMask::GenerateHLSLExpression(FMaterialHLSLGenerator& Generator, UE::HLSLTree::FScope& Scope, int32 OutputIndex, UE::HLSLTree::FExpression const*& OutExpression) const
{
	using namespace UE::HLSLTree;

	const FExpression* MaskLayerExpression = nullptr;
	const bool bTextureArrayEnabled = UE::Landscape::IsMobileWeightmapTextureArrayEnabled();
	verify(GenerateStaticTerrainLayerWeightExpression(ParameterName, 0.f, bTextureArrayEnabled, Generator, MaskLayerExpression));

	FTree& Tree = Generator.GetTree();
	const FExpression* ConstantOne = Tree.NewConstant(1.f);
	OutExpression = MaskLayerExpression ? Tree.NewSub(ConstantOne, MaskLayerExpression) : ConstantOne;
	return true;
}
#endif
#endif

#if WITH_EDITOR
class FHLSLCompilerChild : public FHLSLMaterialTranslator
{
public:
	auto& GetStaticParameters() const
	{
		return StaticParameters;
	}
};

class FVoxelMaterialCompiler : public FProxyMaterialCompiler
{
public:
	static TSet<FMaterialCompiler*>& GetVoxelCompilers()
	{
		static TSet<FMaterialCompiler*> Set;
		return Set;
	}
	
	FVoxelMaterialCompiler(FMaterialCompiler* InCompiler)
		: FProxyMaterialCompiler(InCompiler)
	{
		GetVoxelCompilers().Add(this);
	}
	~FVoxelMaterialCompiler()
	{
		ensure(GetVoxelCompilers().Contains(this));
		GetVoxelCompilers().Remove(this);
	}

	FMaterialCompiler* GetBaseCompiler() const
	{
		if (GetVoxelCompilers().Contains(Compiler))
		{
			// Can happen in recursive calls
			return static_cast<FVoxelMaterialCompiler*>(Compiler)->GetBaseCompiler();
		}
		else
		{
			return Compiler;
		}
	}

	virtual int32 StaticTerrainLayerWeight(FName ParameterName, int32 Default UE_504_ONLY(, bool bTextureArray = false)) override
	{
		FHLSLCompilerChild* HLSLCompiler = static_cast<FHLSLCompilerChild*>(GetBaseCompiler());
		auto& TerrainLayerWeightParameters = HLSLCompiler->GetStaticParameters().EditorOnly.TerrainLayerWeightParameters;

		TArray<int32> VoxelIndices;
		int32 ParameterVoxelIndex = -1;
		for (auto& Parameter : TerrainLayerWeightParameters)
		{
			const int32 WeightmapIndex = Parameter.WeightmapIndex - 1000000;
			if (WeightmapIndex >= 0)
			{
				VoxelIndices.Add(WeightmapIndex);
				
				if (Parameter.LayerName == ParameterName)
				{
					ParameterVoxelIndex = WeightmapIndex;
				}
			}
		}

		if (VoxelIndices.Num() == 0)
		{
			// Default to regular behavior in case we're compiling a real landscape material
			return Compiler->StaticTerrainLayerWeight(ParameterName, Default);
		}

		if (ParameterName == UMaterialExpressionLandscapeVisibilityMask::ParameterName)
		{
			// No support for visibility mask
			return INDEX_NONE;
		}
		
      // DDW - This ensure breaks builds
		// if (!ensureVoxelSlow(ParameterVoxelIndex != -1))
      if ( !( ParameterVoxelIndex != -1 ) )
		{
			// We should have all the parameters overriden
			return INDEX_NONE;
		}
		if (ParameterVoxelIndex == 6)
		{
			// Valid index: means it's not used
			// Make sure to return 0 to avoid inconsistencies
			return Constant(0.f);
		}
		if (!ensureVoxelSlow(ParameterVoxelIndex >= 0 && ParameterVoxelIndex < 6))
		{
			// Invalid index
			return INDEX_NONE;
		}

		// Remove all unused layers
		VoxelIndices.Remove(6);

		const int32 NumBlends = VoxelIndices.Num();
		ensure(NumBlends > 0 && NumBlends <= 6);
		
		const int32 R = ComponentMask(VertexColor(), true, false, false, false);
		const int32 G = ComponentMask(VertexColor(), false, true, false, false);
		const int32 B = ComponentMask(VertexColor(), false, false, true, false);
		const int32 U = ComponentMask(TextureCoordinate(1, false, false), true, false, false, false);
		const int32 V = ComponentMask(TextureCoordinate(1, false, false), false, true, false, false);

		TVoxelStaticArray<int32, 6> Inputs;
		for (int32& Input : Inputs)
		{
			Input = Constant(0.f);
		}
		Inputs[ParameterVoxelIndex] = Constant(1.f);

		int32 Output = Inputs[0];

		if (NumBlends > 1) Output = Lerp(Output, Inputs[1], R);
		if (NumBlends > 2) Output = Lerp(Output, Inputs[2], G);
		if (NumBlends > 3) Output = Lerp(Output, Inputs[3], B);
		if (NumBlends > 4) Output = Lerp(Output, Inputs[4], U);
		if (NumBlends > 5) Output = Lerp(Output, Inputs[5], V);
		
		return Output;
	}

public:
	virtual int32 ReflectionAboutCustomWorldNormal(int32 CustomWorldNormal, int32 bNormalizeCustomWorldNormal) override
	{
		return Compiler->ReflectionAboutCustomWorldNormal(CustomWorldNormal, bNormalizeCustomWorldNormal);
	}
	virtual int32 ParticleRelativeTime() override
	{
		return Compiler->ParticleRelativeTime();
	}
	virtual int32 ParticleMotionBlurFade() override
	{
		return Compiler->ParticleMotionBlurFade();
	}
	virtual int32 ParticleRandom() override
	{
		return Compiler->ParticleRandom();
	}
	virtual int32 ParticleDirection() override
	{
		return Compiler->ParticleDirection();
	}
	virtual int32 ParticleSpeed() override
	{
		return Compiler->ParticleSpeed();
	}
	virtual int32 ParticleSize() override
	{
		return Compiler->ParticleSize();
	}
	virtual int32 VertexInterpolator(uint32 InterpolatorIndex) override
	{
		return Compiler->VertexInterpolator(InterpolatorIndex);
	}
	virtual int32 MaterialBakingWorldPosition() override
	{
		return Compiler->MaterialBakingWorldPosition();
	}
#if VOXEL_ENGINE_VERSION >= 503
	virtual int32 ParticleSpriteRotation() override
	{
		return Compiler->ParticleSpriteRotation();
	}
#endif
};

#define FORWARD_CLASS(Name) \
	void Name::GetCaption(TArray<FString>& OutCaptions) const \
	{ \
		Super::GetCaption(OutCaptions); \
		OutCaptions[0] += " (voxel)"; \
	} \
	\
	int32 Name::Compile(FMaterialCompiler* Compiler, int32 OutputIndex) \
	{ \
		FVoxelMaterialCompiler VoxelCompiler(Compiler); \
		return Super::Compile(&VoxelCompiler, OutputIndex); \
	}

FORWARD_CLASS(UMaterialExpressionVoxelLandscapeLayerBlend)
FORWARD_CLASS(UMaterialExpressionVoxelLandscapeLayerSwitch)
FORWARD_CLASS(UMaterialExpressionVoxelLandscapeLayerWeight)
FORWARD_CLASS(UMaterialExpressionVoxelLandscapeLayerSample)
FORWARD_CLASS(UMaterialExpressionVoxelLandscapeVisibilityMask)

#undef FORWARD_CLASS

bool NeedsToBeConvertedToVoxelImp(const TArray<UMaterialExpression*>& Expressions, TSet<UMaterialFunction*>& VisitedFunctions)
{
	for (auto* Expression : Expressions)
	{
		if (FVoxelMaterialExpressionUtilities::GetVoxelExpression(Expression->GetClass()))
		{
			return true;
		}
		if (auto* FunctionCall = Cast<UMaterialExpressionMaterialFunctionCall>(Expression))
		{
			auto* Function = Cast<UMaterialFunction>(FunctionCall->MaterialFunction);
			if (Function && !VisitedFunctions.Contains(Function))
			{
				VisitedFunctions.Add(Function);
				if (NeedsToBeConvertedToVoxelImp(Function->GetEditorOnlyData()->ExpressionCollection.Expressions, VisitedFunctions))
				{
					return true;
				}
			}
		}
	}
	
	return false;
}

bool FVoxelMaterialExpressionUtilities::NeedsToBeConvertedToVoxel(const TArray<UMaterialExpression*>& Expressions)
{
	TSet<UMaterialFunction*> VisitedFunctions;
	return NeedsToBeConvertedToVoxelImp(Expressions, VisitedFunctions);
}
#endif