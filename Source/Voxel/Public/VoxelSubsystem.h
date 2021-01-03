// Copyright 2021 Phyronnaz

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

	virtual TVoxelSharedRef<IVoxelSubsystem> GetSubsystem(FVoxelRuntime& Runtime, const FVoxelRuntimeSettings& Settings) const;
};

// Any class deriving from this will be created if ShouldCreateSubsystem returns true
UCLASS(Abstract)
class VOXEL_API UVoxelStaticSubsystemProxy : public UVoxelSubsystemProxy
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(const FVoxelRuntime& Runtime, const FVoxelRuntimeSettings& Settings) const
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
	virtual TVoxelSharedRef<IVoxelSubsystem> GetSubsystem(FVoxelRuntime& Runtime, const FVoxelRuntimeSettings& Settings) const override;

#define DEFINE_VOXEL_SUBSYSTEM_PROXY(Class) \
	TVoxelSharedRef<IVoxelSubsystem> Class::GetSubsystem(FVoxelRuntime& Runtime, const FVoxelRuntimeSettings& Settings) const \
	{ \
		static_assert(TIsSame<Class, SubsystemClass::ProxyClass>::Value, ""); \
		return FVoxelUtilities::MakeGameThreadTickableDeleterPtr<SubsystemClass>(Runtime, Settings); \
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
	FORCEINLINE auto AsShared() { return StaticCastSharedRef<VOXEL_THIS_TYPE>(this->Super::AsShared()); } \
	FORCEINLINE auto AsShared() const { return StaticCastSharedRef<const VOXEL_THIS_TYPE>(this->Super::AsShared()); } \
	FORCEINLINE static UClass* StaticClass() { return ProxyClass::StaticClass(); }

#define VOXEL_SUBSYSTEM_FWD(InSubsystemClass, Getter) \
	using SubsystemClass = InSubsystemClass; \
	FORCEINLINE static decltype(auto) GetFromSubsystem(const SubsystemClass& Subsystem) \
	{ \
		return Subsystem.Getter(); \
	}
	
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
	const TVoxelSharedRef<FVoxelRuntimeData> RuntimeData;
	const TVoxelSharedRef<FVoxelRuntimeDynamicSettings> DynamicSettings;
	const TVoxelWeakPtr<FVoxelRuntime> WeakRuntime;

	// This constructor should only be used to use the subsystem as a placeholder/empty subsystem
	IVoxelSubsystem(FVoxelRuntime& Runtime, const FVoxelRuntimeSettings& Settings);
	virtual ~IVoxelSubsystem();
	UE_NONCOPYABLE(IVoxelSubsystem);
	
	void OnDeleteTickable() { PreDestructor(); }

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
	T& GetSubsystemChecked() const
	{
		return WeakRuntime.Pin()->GetSubsystemChecked<T>();
	}

protected:
	//~ Begin IVoxelSubsystem Interface
	virtual void Create();
	virtual void Destroy();

	// Called once every subsystem is created
	virtual void PostCreate(const IVoxelSubsystem* OldSubsystem);
	// Called before the destructor is called. Used to clean up GC objects, as the destructor is delayed due to the tickable bug
	virtual void PreDestructor();
	
	virtual UClass* GetProxyClass() const = 0;
	//~ End IVoxelSubsystem Interface

private:
	enum class EState
	{
		Init,
		Create,
		PostCreate,
		Destroy,
		PreDestructor,
		
		Before_Create = Init,
		Before_PostCreate = Create,
		Before_Destroy = PostCreate,
		Before_PreDestructor = Destroy,

		Last = PreDestructor,
	};
	EState State = EState::Init;
	
	template<typename T>
	friend struct TDeleter;
	friend class FVoxelRuntime;
};