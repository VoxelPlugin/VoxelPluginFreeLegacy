// Copyright 2020 Phyronnaz

#include "VoxelSubsystem.h"

TVoxelSharedRef<IVoxelSubsystem> UVoxelSubsystemProxy::GetSubsystem(FVoxelRuntime& Runtime, const FVoxelRuntimeSettings& Settings) const
{
	check(false);
	return TVoxelSharedPtr<IVoxelSubsystem>().ToSharedRef();
}

IVoxelSubsystem::IVoxelSubsystem(FVoxelRuntime& Runtime, const FVoxelRuntimeSettings& Settings)
	: Settings(Settings)
	, RuntimeData(Runtime.RuntimeData)
	, DynamicSettings(Runtime.DynamicSettings)
	, WeakRuntime(Runtime.AsShared())
{
}

IVoxelSubsystem::~IVoxelSubsystem()
{
	ensure(IsInGameThread());
	ensure(State == EState::Last);
}

void IVoxelSubsystem::Create()
{
	ensure(State == EState::Before_Create);
	State = EState::Create;
}

void IVoxelSubsystem::Destroy()
{
	ensure(State == EState::Before_Destroy);
	State = EState::Destroy;
}

void IVoxelSubsystem::PostCreate(const IVoxelSubsystem* OldSubsystem)
{
	ensure(State == EState::Before_PostCreate);
	State = EState::PostCreate;
}

void IVoxelSubsystem::PreDestructor()
{
	ensure(State == EState::Before_PreDestructor);
	State = EState::PreDestructor;
}