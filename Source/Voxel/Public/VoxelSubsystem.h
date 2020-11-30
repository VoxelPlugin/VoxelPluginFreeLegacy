// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelRuntime.h"
#include "VoxelSubsystem.generated.h"

class IVoxelSubsystem;

UCLASS(Abstract)
class VOXEL_API UVoxelSubsystemProxy : public UObject
{
	GENERATED_BODY()

public:
	using SubsystemClass = IVoxelSubsystem;

	virtual TVoxelSharedRef<IVoxelSubsystem> GetSubsystem(FVoxelRuntime& Runtime) const;
};

// Any class deriving from this will be created if ShouldCreateSubsystem returns true
UCLASS(Abstract)
class VOXEL_API UVoxelStaticSubsystemProxy : public UVoxelSubsystemProxy
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(const FVoxelRuntime& Runtime) const
	{
		return true;
	}
};

// Only one of these will be created per subclass, according to what's picked in the UI
// eg you could define multiple renderers, but only one will be created
UCLASS(Abstract)
class VOXEL_API UVoxelDynamicSubsystemProxy : public UVoxelSubsystemProxy
{
	GENERATED_BODY()
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GENERATED_VOXEL_SUBSYSTEM_PROXY_BODY(InSubsystemClass) \
	public: \
	VOXEL_FWD_DECLARE_CLASS(InSubsystemClass) \
	using SubsystemClass = InSubsystemClass; \
	virtual TVoxelSharedRef<IVoxelSubsystem> GetSubsystem(FVoxelRuntime& Runtime) const override;

#define DEFINE_VOXEL_SUBSYSTEM_PROXY(Class) \
	TVoxelSharedRef<IVoxelSubsystem> Class::GetSubsystem(FVoxelRuntime& Runtime) const \
	{ \
		static_assert(TIsSame<Class, SubsystemClass::ProxyClass>::Value, ""); \
		return FVoxelUtilities::MakeGameThreadTickableDeleterPtr<SubsystemClass>(Runtime); \
	}

#define GENERATED_ABSTRACT_VOXEL_SUBSYSTEM_PROXY_BODY(InSubsystemClass) \
	public: \
	VOXEL_FWD_DECLARE_CLASS(InSubsystemClass) \
	using SubsystemClass = InSubsystemClass;

#define GENERATED_VOXEL_SUBSYSTEM_BODY(InProxyClass) \
	using Super = InProxyClass::Super::SubsystemClass; \
	using ProxyClass = InProxyClass; \
	using Super::Super; \
	virtual UClass* GetProxyClass() const override { static_assert(TIsSame<VOXEL_THIS_TYPE, ProxyClass::SubsystemClass>::Value, ""); return ProxyClass::StaticClass(); } \
	auto AsShared() { return StaticCastSharedRef<VOXEL_THIS_TYPE>(this->Super::AsShared()); } \
	auto AsShared() const { return StaticCastSharedRef<const VOXEL_THIS_TYPE>(this->Super::AsShared()); } \
	static UClass* StaticClass() { return ProxyClass::StaticClass(); }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class VOXEL_API UVoxelRendererSubsystemProxy : public UVoxelDynamicSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_ABSTRACT_VOXEL_SUBSYSTEM_PROXY_BODY(IVoxelRenderer);
};

UCLASS(Abstract)
class VOXEL_API UVoxelLODSubsystemProxy : public UVoxelDynamicSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_ABSTRACT_VOXEL_SUBSYSTEM_PROXY_BODY(IVoxelLODManager);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Subsystems will always be deleted on the game thread
class VOXEL_API IVoxelSubsystem : public TVoxelSharedFromThis<IVoxelSubsystem>
{
public:
	const FVoxelRuntimeSettings Settings;
	const TVoxelSharedRef<FVoxelRuntimeDynamicSettings> DynamicSettings;
	const TVoxelSharedRef<FVoxelRuntimeData> RuntimeData;
	const TVoxelWeakPtr<FVoxelRuntime> WeakRuntime;

	explicit IVoxelSubsystem(FVoxelRuntime& Runtime);
	virtual ~IVoxelSubsystem();
	UE_NONCOPYABLE(IVoxelSubsystem);

	template<typename T>
	TVoxelSharedPtr<T> GetSubsystem() const
	{
		const auto PinnedRuntime = WeakRuntime.Pin();
		if (PinnedRuntime)
		{
			return PinnedRuntime->GetSubsystem<T>();
		}
		else
		{
			return nullptr;
		}
	}
	template<typename T>
	TVoxelSharedRef<T> GetSubsystemChecked() const
	{
		return WeakRuntime.Pin()->GetSubsystemChecked<T>();
	}
	
	template<typename T>
	TVoxelSharedRef<T> InitializeDependency()
	{
		return WeakRuntime.Pin()->InitializeDependency<T>();
	}

protected:
	//~ Begin IVoxelSubsystem Interface
	virtual void Create();
	virtual void Destroy();
	virtual UClass* GetProxyClass() const = 0;
	//~ End IVoxelSubsystem Interface

private:
	bool bCreateCalled = false;
	bool bDestroyCalled = false;
	
	template<typename T>
	friend struct TDeleter;
	friend class FVoxelRuntime;
};