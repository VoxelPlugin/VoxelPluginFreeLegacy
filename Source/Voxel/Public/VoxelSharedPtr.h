// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

template<typename T>
using TVoxelSharedRef = TSharedRef<T, ESPMode::ThreadSafe>;
template<typename T>
using TVoxelSharedPtr = TSharedPtr<T, ESPMode::ThreadSafe>;
template<typename T>
using TVoxelWeakPtr = TWeakPtr<T, ESPMode::ThreadSafe>;
template<typename T>
using TVoxelSharedFromThis = TSharedFromThis<T, ESPMode::ThreadSafe>;

template<typename CastToType, typename CastFromType>
inline TVoxelSharedRef<CastToType> StaticCastVoxelSharedRef(const TVoxelSharedRef<CastFromType>& InSharedRef)
{
	return StaticCastSharedRef<CastToType, CastFromType, ESPMode::ThreadSafe>(InSharedRef);
}

template<typename CastToType, typename CastFromType>
inline TVoxelSharedPtr<CastToType> StaticCastVoxelSharedPtr(const TVoxelSharedPtr<CastFromType>& InSharedPtr)
{
	return StaticCastSharedPtr<CastToType, CastFromType, ESPMode::ThreadSafe>(InSharedPtr);
}

template<typename T, typename... InArgTypes>
inline TVoxelSharedRef<T> MakeVoxelShared(InArgTypes&&... Args)
{
	return MakeShared<T, ESPMode::ThreadSafe>(Forward<InArgTypes>(Args)...);
}

template<typename T, template<class, ESPMode> class TPtr>
inline TVoxelWeakPtr<T> MakeVoxelWeakPtr(const TPtr<T, ESPMode::ThreadSafe>& Ptr)
{
	return TVoxelWeakPtr<T>(Ptr);
}
template<typename T>
inline TVoxelWeakPtr<T> MakeVoxelWeakPtr(T* Ptr)
{
	return TVoxelWeakPtr<T>(StaticCastVoxelSharedRef<T>(Ptr->AsShared()));
}

template<typename T, template<class, ESPMode> class TPtr>
inline TWeakPtr<T> MakeWeakPtr(const TPtr<T, ESPMode::Fast>& Ptr)
{
	return TWeakPtr<T>(Ptr);
}
template<typename T>
inline TWeakPtr<T> MakeWeakPtr(T* Ptr)
{
	return TWeakPtr<T>(StaticCastSharedRef<T>(Ptr->AsShared()));
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
	template<typename TLambda>
	static auto Create(TLambda Lambda)
	{
		return TBaseDelegate<TReturnType, TArgs...>::CreateLambda(Lambda);
	}
};

template<typename TLambda>
inline auto MakeLambdaDelegate(TLambda Lambda)
{
	return TDelegateFromLambda<TLambda>::Create(Lambda);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T, typename TLambda>
inline auto MakeWeakPtrDelegate(const T& Ptr, TLambda Lambda)
{
	return MakeLambdaDelegate(MakeWeakPtrLambda(Ptr, Lambda));
}