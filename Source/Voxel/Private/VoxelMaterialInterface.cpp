// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelMaterialInterface.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

FAutoConsoleCommandWithWorldAndArgs ClearInstancePoolCmd(
	TEXT("voxel.renderer.ClearMaterialInstancePool"),
	TEXT("Clear material instance pool"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const auto&, auto*) { FVoxelMaterialInstance::ClearPool(); }));

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Material Instances Pool"), STAT_VoxelMaterialInstancesPool, STATGROUP_VoxelMemory);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Material Instances Used"), STAT_VoxelMaterialInstancesUsed, STATGROUP_VoxelMemory);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<IVoxelMaterialInterface> IVoxelMaterialInterface::Default()
{
	return MakeShareable(new IVoxelMaterialInterface(DefaultMaterial()));
}

IVoxelMaterialInterface::IVoxelMaterialInterface(UMaterialInterface* MaterialInterface)
	: MaterialInterface(MaterialInterface)
	, MaterialName_Debug(MaterialInterface->GetFName())
{
	check(MaterialInterface);
}

UMaterialInterface* IVoxelMaterialInterface::DefaultMaterial()
{
	return UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);
}

IVoxelMaterialInterface::~IVoxelMaterialInterface()
{
	// Note: MaterialInterface is null if the material is recompiled
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMaterialInterface::FVoxelMaterialInterface(UMaterialInterface* MaterialInterface)
	: IVoxelMaterialInterface(MaterialInterface)
	, bWasAlreadyRooted(MaterialInterface->IsRooted())
{
	if (/*ensure*/(!bWasAlreadyRooted)) // Any object in a static constructor is rooted, so happens quite often
	{
		MaterialInterface->AddToRoot();
	}
}

FVoxelMaterialInterface::~FVoxelMaterialInterface()
{
	// Happens if the material is recompiled
	if (!MaterialInterface.IsValid())
	{
		return;
	}
	
	// Tricky case if bWasAlreadyRooted but not rooted anymore
	if (ensure(MaterialInterface->IsRooted()) && !bWasAlreadyRooted)
	{
		MaterialInterface->RemoveFromRoot();
	}
}

TVoxelSharedRef<IVoxelMaterialInterface> FVoxelMaterialInterface::Create(UMaterialInterface* MaterialInterface)
{
	VOXEL_FUNCTION_COUNTER();

	check(IsInGameThread());
	if (!MaterialInterface ||
		!ensure(TWeakObjectPtr<UMaterialInterface>(MaterialInterface).IsValid()))
	{
		return IVoxelMaterialInterface::Default();
	}
	if (MaterialInterface->GetMaterial()->IsDefaultMaterial() && ensure(MaterialInterface->IsRooted()))
	{
		return MakeShareable(new IVoxelMaterialInterface(MaterialInterface));
	}
	
	static TMap<TWeakObjectPtr<UMaterialInterface>, TVoxelWeakPtr<FVoxelMaterialInterface>> Map;
	auto& WeakPtr = Map.FindOrAdd(MaterialInterface);
	auto PinnedPtr = WeakPtr.Pin();
	if (!PinnedPtr.IsValid())
	{
		PinnedPtr = MakeShareable(new FVoxelMaterialInterface(MaterialInterface));
		WeakPtr = PinnedPtr;
	}
	return PinnedPtr.ToSharedRef();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMaterialInstance::FVoxelMaterialInstance(UMaterialInstanceDynamic* MaterialInstance)
	: IVoxelMaterialInterface(MaterialInstance)
{
	ensure(MaterialInterface->IsRooted());
	INC_DWORD_STAT(STAT_VoxelMaterialInstancesUsed);
}

FVoxelMaterialInstance::~FVoxelMaterialInstance()
{
	DEC_DWORD_STAT(STAT_VoxelMaterialInstancesUsed);

	if (!MaterialInterface.IsValid())
	{
		return;
	}

	ensure(MaterialInterface->IsRooted());
	InstancePool.Add(CastChecked<UMaterialInstanceDynamic>(MaterialInterface));
	INC_DWORD_STAT(STAT_VoxelMaterialInstancesPool);
}

void FVoxelMaterialInstance::ClearPool()
{
	VOXEL_FUNCTION_COUNTER();

	for (auto& Instance : InstancePool)
	{
		if (ensure(Instance.IsValid()))
		{
			Instance->RemoveFromRoot();
		}
	}
	UE_LOG(LogVoxel, Log, TEXT("Clear Material Instance Pool: %d instances removed"), InstancePool.Num());
	DEC_DWORD_STAT_BY(STAT_VoxelMaterialInstancesPool, InstancePool.Num());
	InstancePool.Reset();
}

// To access SetParentInternal
class FMaterialInstanceResource
{
public:
	static void SetParent(UMaterialInstanceDynamic& Instance, UMaterialInterface* Parent)
	{
		VOXEL_FUNCTION_COUNTER();
		Instance.SetParentInternal(Parent, false);
	}
};

TVoxelSharedRef<IVoxelMaterialInterface> FVoxelMaterialInstance::Create(UMaterialInterface* Parent)
{
	VOXEL_FUNCTION_COUNTER();

	check(IsInGameThread());
	if (!ensure(Parent) ||
		!ensure(TWeakObjectPtr<UMaterialInterface>(Parent).IsValid()))
	{
		return IVoxelMaterialInterface::Default();
	}

	UMaterialInstanceDynamic* Instance = nullptr;
	while (!Instance && InstancePool.Num() > 0)
	{
		Instance = InstancePool.Pop(false).Get();
		ensure(Instance);
		DEC_DWORD_STAT(STAT_VoxelMaterialInstancesPool);
	}

	if (Instance)
	{
		ensure(Instance->IsRooted());
		VOXEL_SCOPE_COUNTER("ClearParameterValues");
		Instance->ClearParameterValues();
	}
	else
	{
		Instance = NewObject<UMaterialInstanceDynamic>(GetTransientPackage());
		Instance->AddToRoot();
	}
	
	FMaterialInstanceResource::SetParent(*Instance, Parent);

	return MakeShareable(new FVoxelMaterialInstance(Instance));
}

TArray<TWeakObjectPtr<UMaterialInstanceDynamic>> FVoxelMaterialInstance::InstancePool;