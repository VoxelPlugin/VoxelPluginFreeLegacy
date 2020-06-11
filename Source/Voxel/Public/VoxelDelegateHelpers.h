// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSharedPtr.h"

/**
 * Delegate impl for weak lambda on shared pointers
 */

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
	using Type = TBaseDelegate<TReturnType, TArgs...>;
	
	template<typename UserClass, ESPMode Mode, typename TFunctor>
	using TDelegateImpl = TBaseSPFunctorDelegateInstance<UserClass, Mode, TReturnType(TArgs...), TFunctor, TArgs...>;
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
	return TLambdaConditionalForward<TLambda>::Create(Lambda, [WeakPtr = MakeWeakPtr(Ptr)]() { return WeakPtr.Pin(); }, [](const auto& Ptr) { return Ptr.IsValid(); });
}

template<typename T, typename TLambda>
inline auto MakeWeakVoxelPtrLambda(const T& Ptr, TLambda Lambda)
{
	return TLambdaConditionalForward<TLambda>::Create(Lambda, [WeakPtr = MakeWeakVoxelPtr(Ptr)]() { return WeakPtr.Pin(); }, [](const auto& Ptr) { return Ptr.IsValid(); });
}