// Copyright 2020 Phyronnaz

#include "VoxelSubsystem.h"

TVoxelSharedRef<IVoxelSubsystem> UVoxelSubsystemProxy::GetSubsystem(FVoxelRuntime& Runtime) const
{
	check(false);
	return TVoxelSharedPtr<IVoxelSubsystem>().ToSharedRef();
}

IVoxelSubsystem::IVoxelSubsystem(FVoxelRuntime& Runtime)
	: Settings(Runtime.Settings)
	, DynamicSettings(Runtime.DynamicSettings)
	, RuntimeData(Runtime.RuntimeData)
	, WeakRuntime(Runtime.AsShared())
{
}

IVoxelSubsystem::~IVoxelSubsystem()
{
	ensure(IsInGameThread());
	ensure(bCreateCalled);
	ensure(bDestroyCalled);
}

void IVoxelSubsystem::Create()
{
	ensure(!bCreateCalled);
	ensure(!bDestroyCalled);
	bCreateCalled = true;
}

void IVoxelSubsystem::Destroy()
{
	ensure(bCreateCalled);
	ensure(!bDestroyCalled);
	bDestroyCalled = true;
}
