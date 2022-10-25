// Copyright 2021 Phyronnaz

#include "VoxelRender/VoxelMaterialExpressions.h"
#include "VoxelContainers/VoxelStaticArray.h"

#include "MaterialCompiler.h"
#include "Materials/HLSLMaterialTranslator.h"

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

	virtual int32 StaticTerrainLayerWeight(FName ParameterName, int32 Default) override
	{
		FHLSLCompilerChild* HLSLCompiler = static_cast<FHLSLCompilerChild*>(GetBaseCompiler());
		auto& TerrainLayerWeightParameters = HLSLCompiler->GetStaticParameters().TerrainLayerWeightParameters;

		TArray<int32> VoxelIndices;
		int32 ParameterVoxelIndex = -1;
		for (auto& Parameter : TerrainLayerWeightParameters)
		{
			const int32 WeightmapIndex = Parameter.WeightmapIndex - 1000000;
			if (WeightmapIndex >= 0)
			{
				VoxelIndices.Add(WeightmapIndex);
				
				if (Parameter.UE_5_SWITCH(ParameterInfo.Name, ParameterInfo_DEPRECATED.Name) == ParameterName)
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
		
		if (!ensure(ParameterVoxelIndex != -1))
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
		if (!ensure(ParameterVoxelIndex >= 0 && ParameterVoxelIndex < 6))
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

#if VOXEL_ENGINE_VERSION >= 426 && VOXEL_ENGINE_VERSION < 500
	virtual int32 PreSkinVertexOffset() override
	{
		return Compiler->PreSkinVertexOffset();
	}
	virtual int32 PostSkinVertexOffset() override
	{
		return Compiler->PostSkinVertexOffset();
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
				if (NeedsToBeConvertedToVoxelImp(Function->FunctionExpressions, VisitedFunctions))
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