// Copyright 2021 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorCache.h"
#include "VoxelGenerators/VoxelGeneratorTools.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"
#include "VoxelGenerators/VoxelGeneratorInstanceWrapper.h"
#include "VoxelMessages.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"
#include "VoxelFoliage/VoxelFoliageInterface.h"

DEFINE_VOXEL_SUBSYSTEM_PROXY(UVoxelGeneratorCacheSubsystemProxy);

void FVoxelGeneratorCacheSubsystem::Create()
{
	Super::Create();

	// Not GetChecked because the subsystem won't be created in free
	GeneratorCache = FVoxelGeneratorCache::Create(Settings.GetGeneratorInit(), WeakRuntime);
}

void FVoxelGeneratorCacheSubsystem::PreDestructor()
{
	Super::PreDestructor();

	GeneratorCache.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelGeneratorCache> FVoxelGeneratorCache::Create(const FVoxelGeneratorInit& Init, TVoxelWeakPtr<FVoxelRuntime> Runtime)
{
	ensure(!Init.Runtime.IsValid());
	const TVoxelSharedRef<FVoxelGeneratorCache> Result = MakeShareable(new FVoxelGeneratorCache());
	Result->GeneratorInit = Init;
	Result->GeneratorInit.Runtime = Runtime;
	Result->GeneratorInit.GeneratorCache = Result;
	OnGeneratorRecompiled.AddThreadSafeSP(Result, &FVoxelGeneratorCache::OnGeneratorRecompiledImpl);
	return Result;
}

FVoxelGeneratorCache::FOnGeneratorRecompiled FVoxelGeneratorCache::OnGeneratorRecompiled;

void FVoxelGeneratorCache::OnGeneratorRecompiledImpl(UVoxelGenerator* Generator)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	for (auto It = Cache.CreateIterator(); It; ++It)
	{
		if (It.Key().GetGenerator() == Generator)
		{
			It.RemoveCurrent();
		}
	}
	for (auto It = TransformableCache.CreateIterator(); It; ++It)
	{
		if (It.Key().GetGenerator() == Generator)
		{
			It.RemoveCurrent();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelGeneratorInstance> FVoxelGeneratorCache::MakeGeneratorInstance(FVoxelGeneratorPicker Picker)
{
	return MakeGeneratorInstanceImpl(Picker);
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> FVoxelGeneratorCache::MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker)
{
	return MakeGeneratorInstanceImpl(Picker);
}

UVoxelGeneratorInstanceWrapper* FVoxelGeneratorCache::K2_MakeGeneratorInstance(FVoxelGeneratorPicker Picker)
{
	return K2_MakeGeneratorInstanceImpl(Picker);
}

UVoxelTransformableGeneratorInstanceWrapper* FVoxelGeneratorCache::K2_MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker)
{
	return K2_MakeGeneratorInstanceImpl(Picker);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGeneratorCache::AddReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	GeneratorInit.AddReferencedObjects(Collector);

	WrapperCache.Remove(nullptr);
	WrapperTransformableCache.Remove(nullptr);
	
	for (auto& It : WrapperCache)
	{
		Collector.AddReferencedObject(It.Value);
	}
	for (auto& It : WrapperTransformableCache)
	{
		Collector.AddReferencedObject(It.Value);
	}
}

template<typename T>
TVoxelSharedRef<typename T::FInstance> FVoxelGeneratorCache::MakeGeneratorInstanceImpl(T Picker)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (auto* Instance = GetCache<T>().Find(Picker))
	{
		return Instance->ToSharedRef();
	}
	
	const auto Instance = Picker.GetInstance();
	Instance->Init(GeneratorInit);
	GetCache<T>().Add(Picker, Instance);
	
	return Instance;
}

template<typename T>
typename T::UWrapper* FVoxelGeneratorCache::K2_MakeGeneratorInstanceImpl(T Picker)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const auto Instance = MakeGeneratorInstanceImpl(Picker);

	auto*& Wrapper = GetWrapperCache<T>().FindOrAdd(Instance);
	
	if (!Wrapper)
	{
		Wrapper = NewObject<typename T::UWrapper>();
		Wrapper->Instance = Instance;
	}
	
	return Wrapper;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelGeneratorInstanceWrapper* UVoxelGeneratorCache::MakeGeneratorInstance(FVoxelGeneratorPicker Picker) const
{
	if (!Picker.IsValid())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid generator"));
		return nullptr;
	}
	if (!ensure(GeneratorCache))
	{
		return nullptr;
	}

	return GeneratorCache->K2_MakeGeneratorInstance(Picker);
}

UVoxelTransformableGeneratorInstanceWrapper* UVoxelGeneratorCache::MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const
{
	if (!Picker.IsValid())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid generator"));
		return nullptr;
	}
	if (!ensure(GeneratorCache))
	{
		return nullptr;
	}

	return GeneratorCache->K2_MakeTransformableGeneratorInstance(Picker);
}