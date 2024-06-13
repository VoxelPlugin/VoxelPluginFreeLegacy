// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelSharedPtr.h"
#include "VoxelEngineVersionHelpers.h"

#if VOXEL_ENGINE_VERSION >= 503
struct FVoxelDelegateUtilities
	: public TDelegateBase<FThreadSafeDelegateMode>
	, public TDelegateBase<FNotThreadSafeDelegateMode>
	, public TDelegateBase<FNotThreadSafeNotCheckedDelegateMode>
{
	template<typename Mode>
	struct THack : TDelegateBase<Mode>
	{
		using TDelegateBase<Mode>::CreateDelegateInstance;
	};

	template<typename Mode, typename DelegateInstanceType>
	static void CreateDelegateInstance(TDelegateBase<Mode>& Base, DelegateInstanceType& DelegateInstance)
	{
		static_cast<THack<Mode>&>(Base).template CreateDelegateInstance<DelegateInstanceType>(DelegateInstance);
	}
	template<typename DelegateInstanceType, typename Mode, typename... DelegateInstanceParams>
	static void CreateDelegateInstance(TDelegateBase<Mode>& Base, DelegateInstanceParams&&... Params)
	{
		static_cast<THack<Mode>&>(Base).template CreateDelegateInstance<DelegateInstanceType>(Forward<DelegateInstanceParams>(Params)...);
	}
};
#endif

/**
 * Delegate impl for weak lambda on shared pointers
 */

template <typename UserClass, ESPMode SPMode, typename FuncType, typename UserPolicy, typename FunctorType>
class TBaseSPFunctorDelegateInstance;

template <typename UserClass, ESPMode SPMode, typename RetValType, typename... ParamTypes, typename UserPolicy, typename FunctorType>
class TBaseSPFunctorDelegateInstance<UserClass, SPMode, RetValType(ParamTypes...), UserPolicy, FunctorType> : public TCommonDelegateInstanceState<RetValType(ParamTypes...), UserPolicy>
{
private:
	static_assert(std::is_same_v<FunctorType, typename TRemoveReference<FunctorType>::Type>, "FunctorType cannot be a reference");

	using Super = TCommonDelegateInstanceState<RetValType(ParamTypes...), UserPolicy>;
	using ThisType = TBaseSPFunctorDelegateInstance<UserClass, SPMode, RetValType(ParamTypes...), UserPolicy, FunctorType>;

public:
	TBaseSPFunctorDelegateInstance(const TSharedPtr<UserClass, SPMode>& InUserObject, const FunctorType& InFunctor)
		: Super     ()
		, UserObject(InUserObject)
		, Functor   (InFunctor)
	{
	}

	TBaseSPFunctorDelegateInstance(const TSharedPtr<UserClass, SPMode>& InUserObject, FunctorType&& InFunctor)
		: Super     ()
		, UserObject(InUserObject)
		, Functor   (MoveTemp(InFunctor))
	{
	}

	// IDelegateInstance interface

#if USE_DELEGATE_TRYGETBOUNDFUNCTIONNAME

	FName TryGetBoundFunctionName() const final
	{
		return NAME_None;
	}

#endif

	UObject* GetUObject() const final
	{
		return nullptr;
	}

	const void* GetObjectForTimerManager() const final
	{
		return UserObject.Pin().Get();
	}

	uint64 GetBoundProgramCounterForTimerManager() const final
	{
		return 0;
	}

	// Deprecated
	bool HasSameObject(const void* InUserObject) const final
	{
		return UserObject.HasSameObject(InUserObject);
	}
	
	bool IsSafeToExecute() const final
	{
		return UserObject.IsValid();
	}

public:
	// IBaseDelegateInstance interface
#if VOXEL_ENGINE_VERSION >= 503
	virtual void CreateCopy(TDelegateBase<FThreadSafeDelegateMode>& Base) const final override
	{
		FVoxelDelegateUtilities::CreateDelegateInstance(Base, *this);
	}
	virtual void CreateCopy(TDelegateBase<FNotThreadSafeDelegateMode>& Base) const override
	{
		FVoxelDelegateUtilities::CreateDelegateInstance(Base, *this);
	}
	virtual void CreateCopy(TDelegateBase<FNotThreadSafeNotCheckedDelegateMode>& Base) const override
	{
		FVoxelDelegateUtilities::CreateDelegateInstance(Base, *this);
	}
#else
	virtual void CreateCopy(FDelegateBase& Base) const final override
	{
		new (Base) ThisType(*(ThisType*)this);
	}
#endif

	virtual RetValType Execute(ParamTypes... Params) const final override
	{
		typedef typename UE_503_SWITCH(TRemoveConst<UserClass>::Type, std::remove_const_t<UserClass>) MutableUserClass;

		// Verify that the user object is still valid.  We only have a weak reference to it.
		auto SharedUserObject = UserObject.Pin();
		check(SharedUserObject.IsValid());

		return Functor(Forward<ParamTypes>(Params)...);
	}
	
	virtual bool ExecuteIfSafe(ParamTypes... Params) const final override
	{
		// Verify that the user object is still valid.  We only have a weak reference to it.
		if (TSharedPtr<UserClass, SPMode> SharedUserObject = this->UserObject.Pin())
		{
			(void)Functor(Forward<ParamTypes>(Params)...);

			return true;
		}

		return false;
	}

public:
	/**
	 * Creates a new shared pointer delegate binding for the given user object and lambda.
	 *
	 * @param InUserObjectRef Shared reference to the user's object that contains the class method.
	 * @param InFunc  Lambda
	 * @return The new delegate.
	 */
	FORCEINLINE static void Create(UE_503_SWITCH(FDelegateBase, TDelegateBase<FDefaultDelegateUserPolicy::FThreadSafetyMode>)& Base, const TSharedPtr<UserClass, SPMode>& InUserObjectRef, FunctorType&& InFunc)
	{
#if VOXEL_ENGINE_VERSION >= 503
		FVoxelDelegateUtilities::CreateDelegateInstance<ThisType>(Base, InUserObjectRef, MoveTemp(InFunc));
#else
		new (Base) ThisType(InUserObjectRef, MoveTemp(InFunc));
#endif
	}

	/**
	 * Creates a new shared pointer delegate binding for the given user object and lambda.
	 *
	 * This overload requires that the supplied object derives from TSharedFromThis.
	 *
	 * @param InUserObject  The user's object that contains the class method.  Must derive from TSharedFromThis.
	 * @param InFunc  Lambda
	 * @return The new delegate.
	 */
	FORCEINLINE static void Create(UE_503_SWITCH(FDelegateBase, TDelegateBase<FDefaultDelegateUserPolicy::FThreadSafetyMode>)& Base, UserClass* InUserObject, FunctorType&& InFunc)
	{
		// We expect the incoming InUserObject to derived from TSharedFromThis.
		auto UserObjectRef = StaticCastSharedRef<UserClass>(InUserObject->AsShared());
		Create(Base, UserObjectRef, MoveTemp(InFunc));
	}

private:
	// Context object - the validity of this object controls the validity of the lambda
	TWeakPtr<UserClass, SPMode> UserObject;

	// C++ functor
	// We make this mutable to allow mutable lambdas to be bound and executed.  We don't really want to
	// model the Functor as being a direct subobject of the delegate (which would maintain transivity of
	// const - because the binding doesn't affect the substitutability of a copied delegate.
	mutable typename UE_503_SWITCH(TRemoveConst<FunctorType>::Type, std::remove_const_t<FunctorType>) Functor;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class Lambda>
struct TDelegateFromLambda : TDelegateFromLambda<decltype(&Lambda::operator())>
{
};

template<typename TReturnType, typename TClass, typename... TArgs>
struct TDelegateFromLambda<TReturnType(TClass::*)(TArgs...) const> : TDelegateFromLambda<TReturnType(TClass::*)(TArgs...)>
{

};

template<typename TReturnType, typename TClass, typename... TArgs>
struct TDelegateFromLambda<TReturnType(TClass::*)(TArgs...)>
{
	using Type = TDelegate<TReturnType(TArgs...)>;
	
	template<typename UserClass, ESPMode Mode, typename TFunctor>
	using TDelegateImpl = TBaseSPFunctorDelegateInstance<UserClass, Mode, TReturnType(TArgs...), FDefaultDelegateUserPolicy, TFunctor>;
};

template<typename TLambda>
auto MakeLambdaDelegate(TLambda Lambda)
{
	return TDelegateFromLambda<TLambda>::Type::CreateLambda(MoveTemp(Lambda));
}

template<typename T, typename TLambda>
auto MakeWeakObjectPtrDelegate(T* Ptr, TLambda Lambda)
{
	return TDelegateFromLambda<TLambda>::Type::CreateWeakLambda(VOXEL_CONST_CAST(Ptr), MoveTemp(Lambda));
}

template<typename TClass, ESPMode Mode, typename TLambda>
auto MakeWeakPtrDelegate(const TSharedPtr<TClass, Mode>& Object, TLambda Lambda)
{
	typename TDelegateFromLambda<TLambda>::Type Delegate;
	TDelegateFromLambda<TLambda>::template TDelegateImpl<TClass, Mode, TLambda>::Create(Delegate, Object, MoveTemp(Lambda));
	return Delegate;
}

template<typename TClass, ESPMode Mode, typename TLambda>
auto MakeWeakPtrDelegate(const TSharedRef<TClass, Mode>& Object, TLambda Lambda)
{
	typename TDelegateFromLambda<TLambda>::Type Delegate;
	TDelegateFromLambda<TLambda>::template TDelegateImpl<TClass, Mode, TLambda>::Create(Delegate, Object, MoveTemp(Lambda));
	return Delegate;
}

template<typename TClass, typename TLambda>
auto MakeWeakPtrDelegate(TClass* Object, TLambda Lambda)
{
	typename TDelegateFromLambda<TLambda>::Type Delegate;
	TDelegateFromLambda<TLambda>::template TDelegateImpl<TClass, ESPMode::ThreadSafe, TLambda>::Create(Delegate, Object, MoveTemp(Lambda));
	return Delegate;
}

template<typename DelegateType, typename TClass, typename TLambda>
DelegateType MakeWeakPtrTypedDelegate(TClass* Object, TLambda Lambda)
{
	using DelegateImpl = TBaseSPFunctorDelegateInstance<TClass, ESPMode::ThreadSafe, typename DelegateType::TFuncType, FDefaultDelegateUserPolicy, TLambda>;

	DelegateType Delegate;
	DelegateImpl::Create(Delegate, Object, MoveTemp(Lambda));
	return Delegate;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T, typename LambdaType>
auto MakeWeakPtrLambda(const T& Ptr, LambdaType Lambda)
{
	return [WeakPtr = MakeWeakPtr(Ptr), Lambda = MoveTemp(Lambda)](auto&&... Args)
	{
		if (const auto Pinned = WeakPtr.Pin())
		{
			Lambda(Forward<decltype(Args)>(Args)...);
		}
	};
}

template<typename T, typename LambdaType>
auto MakeWeakObjectPtrLambda(const T& Ptr, LambdaType Lambda)
{
	return [WeakPtr = MakeWeakObjectPtr(Ptr), Lambda = MoveTemp(Lambda)](auto&&... Args)
	{
		if (WeakPtr.IsValid())
		{
			Lambda(Forward<decltype(Args)>(Args)...);
		}
	};
}