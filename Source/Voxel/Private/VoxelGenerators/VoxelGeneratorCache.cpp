// Copyright 2020 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorCache.h"
#include "VoxelGenerators/VoxelGeneratorTools.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"
#include "VoxelGenerators/VoxelGeneratorInstanceWrapper.h"
#include "VoxelMessages.h"

TVoxelSharedRef<FVoxelGeneratorInstance> FVoxelEmptyGeneratorCache::MakeNativeGeneratorInstance(FVoxelGeneratorPicker Picker) const
{
	const auto Instance = Picker.GetInstance(false);
	Instance->Init(GeneratorInit);
	return Instance;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> FVoxelEmptyGeneratorCache::MakeNativeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const
{
	const auto Instance = Picker.GetInstance(false);
	Instance->Init(GeneratorInit);
	return Instance;
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

	auto& Entry = Cache.FindOrAdd(Picker);
	
	if (!Entry.Instance)
	{
		ensure(!Entry.Wrapper);
		Entry.Instance = Picker.GetInstance(false);
		Entry.Instance->Init(GeneratorInit);
	}
	
	if (!Entry.Wrapper)
	{
		Entry.Wrapper = NewObject<UVoxelGeneratorInstanceWrapper>();
		Entry.Wrapper->Instance = Entry.Instance;
	}
	
	return Entry.Wrapper;
}

UVoxelTransformableGeneratorInstanceWrapper* UVoxelGeneratorCache::MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const
{
	if (!Picker.IsValid())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid generator"));
		return nullptr;
	}

	auto& Entry = TransformableCache.FindOrAdd(Picker);
	
	if (!Entry.Instance)
	{
		ensure(!Entry.Wrapper);
		Entry.Instance = Picker.GetInstance(false);
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

TVoxelSharedRef<FVoxelGeneratorInstance> UVoxelGeneratorCache::MakeNativeGeneratorInstance(FVoxelGeneratorPicker Picker) const
{
	auto& Entry = Cache.FindOrAdd(Picker);
	
	if (!Entry.Instance)
	{
		Entry.Instance = Picker.GetInstance(false);
		Entry.Instance->Init(GeneratorInit);
	}
	
	return Entry.Instance.ToSharedRef();
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelGeneratorCache::MakeNativeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const
{
	auto& Entry = TransformableCache.FindOrAdd(Picker);
	
	if (!Entry.Instance)
	{
		Entry.Instance = Picker.GetInstance(false);
		Entry.Instance->Init(GeneratorInit);
	}
	
	return Entry.Instance.ToSharedRef();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGeneratorCache::SetGeneratorInit(const FVoxelGeneratorInit& NewInit)
{
	ensure(!NewInit.GeneratorCache.IsValid());
	GeneratorInit = NewInit;
	GeneratorInit.GeneratorCache = this;
}

void UVoxelGeneratorCache::ClearCache()
{
	Cache.Reset();
}

void UVoxelGeneratorCache::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);

	auto* This = CastChecked<UVoxelGeneratorCache>(InThis);
	for (auto& It : This->Cache)
	{
		Collector.AddReferencedObject(It.Value.Wrapper);
	}
	for (auto& It : This->TransformableCache)
	{
		Collector.AddReferencedObject(It.Value.Wrapper);
	}
}