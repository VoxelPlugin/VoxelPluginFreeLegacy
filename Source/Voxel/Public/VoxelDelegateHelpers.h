// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSharedPtr.h"
#include "VoxelEngineVersionHelpers.h"

/**
 * Delegate impl for weak lambda on shared pointers
 */

#if ENGINE_MINOR_VERSION < 26
template <class UserClass, ESPMode SPMode, typename FuncType, typename FunctorType, typename... VarTypes>
class TBaseSPFunctorDelegateInstance;

template <class UserClass, ESPMode SPMode, typename WrappedRetValType, typename... ParamTypes, typename FunctorType, typename... VarTypes>
class TBaseSPFunctorDelegateInstance<UserClass, SPMode, WrappedRetValType (ParamTypes...), FunctorType, VarTypes...> : public IBaseDelegateInstance<typename TUnwrapType<WrappedRetValType>::Type (ParamTypes...)>
{
public:
	typedef typename TUnwrapType<WrappedRetValType>::Type RetValType;

private:
	typedef IBaseDelegateInstance<RetValType (ParamTypes...)> Super;
	typedef TBaseSPFunctorDelegateInstance<UserClass, SPMode, RetValType (ParamTypes...), FunctorType, VarTypes...> UnwrappedThisType;

public:
	TBaseSPFunctorDelegateInstance(const TSharedPtr<UserClass, SPMode>& InUserObject, FunctorType&& InFunctor)
		: UserObject(InUserObject)
		, Functor(MoveTemp(InFunctor))
		, Handle(FDelegateHandle::GenerateNewHandle)
	{
		// NOTE: Shared pointer delegates are allowed to have a null incoming object pointer.  Weak pointers can expire,
		//       an it is possible for a copy of a delegate instance to end up with a null pointer.
	}

	// IDelegateInstance interface

#if USE_DELEGATE_TRYGETBOUNDFUNCTIONNAME
	virtual FName TryGetBoundFunctionName() const override final
	{
		return {};
	}
#endif
	virtual UObject* GetUObject() const override final
	{
		return nullptr;
	}

	virtual const void* GetObjectForTimerManager() const override final
	{
		return UserObject.Pin().Get();
	}

	virtual uint64 GetBoundProgramCounterForTimerManager() const override final
	{
		return 0;
	}

	// Deprecated
	virtual bool HasSameObject(const void* InUserObject) const override final
	{
		return UserObject.HasSameObject(InUserObject);
	}

	virtual bool IsSafeToExecute() const override final
	{
		return UserObject.IsValid();
	}

public:

	// IBaseDelegateInstance interface

	virtual void CreateCopy(FDelegateBase& Base) override final
	{
		new (Base) UnwrappedThisType(*(UnwrappedThisType*)this);
	}

	virtual RetValType Execute(ParamTypes... Params) const override final
	{
		typedef typename TRemoveConst<UserClass>::Type MutableUserClass;

		// Verify that the user object is still valid.  We only have a weak reference to it.
		auto SharedUserObject = UserObject.Pin();
		check(SharedUserObject.IsValid());

		return Functor(Forward<ParamTypes>(Params)...);
	}

	virtual FDelegateHandle GetHandle() const override final
	{
		return Handle;
	}

public:

	/**
	 * Creates a new shared pointer delegate binding for the given user object and lambda.
	 *
	 * @param InUserObjectRef Shared reference to the user's object that contains the class method.
	 * @param InFunc  Lambda
	 * @return The new delegate.
	 */
	FORCEINLINE static void Create(FDelegateBase& Base, const TSharedPtr<UserClass, SPMode>& InUserObjectRef, FunctorType&& InFunc)
	{
		new (Base) UnwrappedThisType(InUserObjectRef, MoveTemp(InFunc));
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
	FORCEINLINE static void Create(FDelegateBase& Base, UserClass* InUserObject, FunctorType&& InFunc)
	{
		// We expect the incoming InUserObject to derived from TSharedFromThis.
		auto UserObjectRef = StaticCastSharedRef<UserClass>(InUserObject->AsShared());
		Create(Base, UserObjectRef, MoveTemp(InFunc));
	}

protected:

	// Weak reference to an instance of the user's class which contains a method we would like to call.
	TWeakPtr<UserClass, SPMode> UserObject;

	// Functor
	FunctorType Functor;

	// The handle of this delegate
	FDelegateHandle Handle;
};

template <class UserClass, ESPMode SPMode, typename... ParamTypes, typename FunctorType, typename... VarTypes>
class TBaseSPFunctorDelegateInstance<UserClass, SPMode, void (ParamTypes...), FunctorType, VarTypes...> : public TBaseSPFunctorDelegateInstance<UserClass, SPMode, TTypeWrapper<void> (ParamTypes...), FunctorType, VarTypes...>
{
	typedef TBaseSPFunctorDelegateInstance<UserClass, SPMode, TTypeWrapper<void> (ParamTypes...), FunctorType, VarTypes...> Super;

public:
	TBaseSPFunctorDelegateInstance(const TSharedPtr<UserClass, SPMode>& InUserObject, FunctorType&& InFunctor)
		: Super(InUserObject, MoveTemp(InFunctor))
	{
	}
	
	virtual bool ExecuteIfSafe(ParamTypes... Params) const override final
	{
		// Verify that the user object is still valid.  We only have a weak reference to it.
		auto SharedUserObject = Super::UserObject.Pin();
		if (SharedUserObject.IsValid())
		{
			Super::Execute(Params...);

			return true;
		}

		return false;
	}
};
#else

template <typename UserClass, ESPMode SPMode, typename FuncType, typename UserPolicy, typename FunctorType>
class TBaseSPFunctorDelegateInstance;

template <typename UserClass, ESPMode SPMode, typename WrappedRetValType, typename... ParamTypes, typename UserPolicy, typename FunctorType>
class TBaseSPFunctorDelegateInstance<UserClass, SPMode, WrappedRetValType(ParamTypes...), UserPolicy, FunctorType> : public TCommonDelegateInstanceState<WrappedRetValType(ParamTypes...), UserPolicy>
{
private:
	static_assert(TAreTypesEqual<FunctorType, typename TRemoveReference<FunctorType>::Type>::Value, "FunctorType cannot be a reference");

	using Super             = TCommonDelegateInstanceState<WrappedRetValType(ParamTypes...), UserPolicy>;
	using RetValType        = typename Super::RetValType;
	using UnwrappedThisType = TBaseSPFunctorDelegateInstance<UserClass, SPMode, RetValType(ParamTypes...), UserPolicy, FunctorType>;

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
	void CreateCopy(FDelegateBase& Base) final
	{
		new (Base) UnwrappedThisType(*(UnwrappedThisType*)this);
	}

	virtual RetValType Execute(ParamTypes... Params) const override final
	{
		typedef typename TRemoveConst<UserClass>::Type MutableUserClass;

		// Verify that the user object is still valid.  We only have a weak reference to it.
		auto SharedUserObject = UserObject.Pin();
		check(SharedUserObject.IsValid());

		return Functor(Forward<ParamTypes>(Params)...);
	}
	
	virtual bool ExecuteIfSafe(ParamTypes... Params) const override final
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
	FORCEINLINE static void Create(FDelegateBase& Base, const TSharedPtr<UserClass, SPMode>& InUserObjectRef, FunctorType&& InFunc)
	{
		new (Base) UnwrappedThisType(InUserObjectRef, MoveTemp(InFunc));
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
	FORCEINLINE static void Create(FDelegateBase& Base, UserClass* InUserObject, FunctorType&& InFunc)
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
	mutable typename TRemoveConst<FunctorType>::Type Functor;
};

#endif

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
#if ENGINE_MINOR_VERSION < 26
	using Type = TBaseDelegate<TReturnType, TArgs...>;
#else
	using Type = TDelegate<TReturnType(TArgs...)>;
#endif
	
	template<typename UserClass, ESPMode Mode, typename TFunctor>
	using TDelegateImpl = TBaseSPFunctorDelegateInstance<UserClass, Mode, TReturnType(TArgs...), UE_26_ONLY(FDefaultDelegateUserPolicy,) TFunctor>;
};

template<typename TLambda>
inline auto MakeLambdaDelegate(TLambda Lambda)
{
	return TDelegateFromLambda<TLambda>::Type::CreateLambda(MoveTemp(Lambda));
}

template<typename T, typename TLambda>
inline auto MakeWeakObjectPtrDelegate(T* Ptr, TLambda Lambda)
{
	return TDelegateFromLambda<TLambda>::Type::CreateWeakLambda(const_cast<typename TRemoveConst<T>::Type*>(Ptr), MoveTemp(Lambda));
}

template<typename TClass, ESPMode Mode, typename TLambda>
inline auto MakeWeakPtrDelegate(const TSharedRef<TClass, Mode>& Object, TLambda Lambda)
{
	typename TDelegateFromLambda<TLambda>::Type Delegate;
	TDelegateFromLambda<TLambda>::template TDelegateImpl<TClass, Mode, TLambda>::Create(Delegate, Object, MoveTemp(Lambda));
	return Delegate;
}

template<typename TClass, typename TLambda>
inline auto MakeWeakPtrDelegate(TClass* Object, TLambda Lambda)
{
	typename TDelegateFromLambda<TLambda>::Type Delegate;
	TDelegateFromLambda<TLambda>::template TDelegateImpl<TClass, ESPMode::Fast, TLambda>::Create(Delegate, Object, MoveTemp(Lambda));
	return Delegate;
}

template<typename TClass, typename TLambda>
inline auto MakeVoxelWeakPtrDelegate(TClass* Object, TLambda Lambda)
{
	typename TDelegateFromLambda<TLambda>::Type Delegate;
	TDelegateFromLambda<TLambda>::template TDelegateImpl<TClass, ESPMode::ThreadSafe, TLambda>::Create(Delegate, Object, MoveTemp(Lambda));
	return Delegate;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class Lambda>
struct TLambdaConditionalForward : TLambdaConditionalForward<decltype(&Lambda::operator())>
{
};

template<typename TReturnType, typename TClass, typename... TArgs>
struct TLambdaConditionalForward<TReturnType(TClass::*)(TArgs...) const> : TLambdaConditionalForward<TReturnType(TClass::*)(TArgs...)>
{

};

template<typename TReturnType, typename TClass, typename... TArgs>
struct TLambdaConditionalForward<TReturnType(TClass::*)(TArgs...)>
{
	template<typename TLambda, typename TGetCondition, typename TCheckCondition>
	static auto Create(TLambda Lambda, TGetCondition GetCondition, TCheckCondition CheckCondition)
	{
		return [=](TArgs... Args)
		{
			// Could be a shared pointer, or a bool
			auto&& Condition = GetCondition();
			if (CheckCondition(Condition))
			{
				Lambda(Forward<TArgs>(Args)...);
			}
		};
	}
};

template<typename T, typename TLambda>
inline auto MakeWeakPtrLambda(const T& Ptr, TLambda Lambda)
{
	return TLambdaConditionalForward<TLambda>::Create(Lambda, [WeakPtr = MakeWeakPtr(Ptr)]() { return WeakPtr.Pin(); }, [](const auto& InPtr) { return InPtr.IsValid(); });
}

template<typename T, typename TLambda>
inline auto MakeVoxelWeakPtrLambda(const T& Ptr, TLambda Lambda)
{
	return TLambdaConditionalForward<TLambda>::Create(Lambda, [WeakPtr = MakeVoxelWeakPtr(Ptr)]() { return WeakPtr.Pin(); }, [](const auto& InPtr) { return InPtr.IsValid(); });
}