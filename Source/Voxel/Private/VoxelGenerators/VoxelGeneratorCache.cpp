// Copyright 2020 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorCache.h"
#include "VoxelGenerators/VoxelGeneratorTools.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"
#include "VoxelGenerators/VoxelGeneratorInstanceWrapper.h"
#include "VoxelMessages.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"

DEFINE_VOXEL_SUBSYSTEM_PROXY(UVoxelGeneratorCacheSubsystemProxy);

void FVoxelGeneratorCacheSubsystem::Create()
{
	Super::Create();

	GeneratorCache = FVoxelGeneratorCache::Create(Settings.GetGeneratorInit());
}

void FVoxelGeneratorCacheSubsystem::PreDestructor()
{
	Super::PreDestructor();

	GeneratorCache.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelGeneratorCache> FVoxelGeneratorCache::Create(const FVoxelGeneratorInit& Init)
{
	ensure(!Init.GeneratorCache.IsValid());
	const TVoxelSharedRef<FVoxelGeneratorCache> Result = MakeShareable(new FVoxelGeneratorCache());
	Result->GeneratorInit = Init;
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

UVoxelGeneratorInstanceWrapper* FVoxelGeneratorCache::K2_MakeGeneratorInstance(FVoxelGeneratorPicker Picker)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	
	auto& Entry = Cache.FindOrAdd(Picker);
	
	if (!Entry.Instance)
	{
		ensure(!Entry.Wrapper);
		Entry.Instance = Picker.GetInstance();
		Entry.Instance->Init(GeneratorInit);
	}
	
	if (!Entry.Wrapper)
	{
		Entry.Wrapper = NewObject<UVoxelGeneratorInstanceWrapper>();
		Entry.Wrapper->Instance = Entry.Instance;
	}
	
	return Entry.Wrapper;
}

UVoxelTransformableGeneratorInstanceWrapper* FVoxelGeneratorCache::K2_MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	
	auto& Entry = TransformableCache.FindOrAdd(Picker);
	
	if (!Entry.Instance)
	{
		ensure(!Entry.Wrapper);
		Entry.Instance = Picker.GetInstance();
		Entry.Instance->Init(GeneratorInit);
	}
	
	if (!Entry.Wrapper)
	{
		Entry.Wrapper = NewObject<UVoxelTransformableGeneratorInstanceWrapper>();
		Entry.Wrapper->Instance = Entry.Instance;
	}
	
	return Entry.Wrapper;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelGeneratorInstance> FVoxelGeneratorCache::MakeGeneratorInstance(FVoxelGeneratorPicker Picker)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	
	auto& Entry = Cache.FindOrAdd(Picker);
	
	if (!Entry.Instance)
	{
		Entry.Instance = Picker.GetInstance();
		Entry.Instance->Init(GeneratorInit);
	}
	
	return Entry.Instance.ToSharedRef();
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> FVoxelGeneratorCache::MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	
	auto& Entry = TransformableCache.FindOrAdd(Picker);
	
	if (!Entry.Instance)
	{
		Entry.Instance = Picker.GetInstance();
		Entry.Instance->Init(GeneratorInit);
	}
	
	return Entry.Instance.ToSharedRef();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGeneratorCache::AddReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	GeneratorInit.AddReferencedObjects(Collector);
	
	for (auto& It : Cache)
	{
		Collector.AddReferencedObject(It.Value.Wrapper);
	}
	for (auto& It : TransformableCache)
	{
		Collector.AddReferencedObject(It.Value.Wrapper);
	}
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