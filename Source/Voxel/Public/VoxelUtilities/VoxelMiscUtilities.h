// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"

namespace FVoxelUtilities
{
	template<typename T>
	struct TValuesMaterialsSelector;
	
	template<>
	struct TValuesMaterialsSelector<FVoxelValue>
	{
		template<typename U> static inline auto& Get(U& Object)
		{
			return Object.Values;
		}
	};
	template<>
	struct TValuesMaterialsSelector<FVoxelMaterial>
	{
		template<typename U> static inline auto& Get(U& Object)
		{
			return Object.Materials;
		}
	};
	
	template<>
	struct TValuesMaterialsSelector<const FVoxelValue>
	{
		template<typename U> static inline auto& Get(U& Object)
		{
			return Object.Values;
		}
	};
	template<>
	struct TValuesMaterialsSelector<const FVoxelMaterial>
	{
		template<typename U> static inline auto& Get(U& Object)
		{
			return Object.Materials;
		}
	};

	struct FTrueType
	{
		FORCEINLINE constexpr operator bool() const
		{
			return true;
		}
	};
	struct FFalseType
	{
		FORCEINLINE constexpr operator bool() const
		{
			return false;
		}
	};

	template<typename T>
	void StaticBranch(bool bA, T Lambda)
	{
		if (bA)
			Lambda(FTrueType());
		else
			Lambda(FFalseType());
	}
	template<typename TLambda>
	void StaticBranch(bool bA, bool bB, TLambda Lambda)
	{
		const int32 Value = bA + 2 * bB;
		using T = FTrueType;
		using F = FFalseType;
		switch (Value)
		{
		case 0: Lambda(F(), F()); return;
		case 1: Lambda(T(), F()); return;
		case 2: Lambda(F(), T()); return;
		case 3: Lambda(T(), T()); return;
		default: checkVoxelSlow(false); return;
		}
	}
	template<typename TLambda>
	void StaticBranch(bool bA, bool bB, bool bC, TLambda Lambda)
	{
		const int32 Value = bA + 2 * bB + 4 * bC;
		using T = FTrueType;
		using F = FFalseType;
		switch (Value)
		{
		case 0: Lambda(F(), F(), F()); return;
		case 1: Lambda(T(), F(), F()); return;
		case 2: Lambda(F(), T(), F()); return;
		case 3: Lambda(T(), T(), F()); return;
		case 4: Lambda(F(), F(), T()); return;
		case 5: Lambda(T(), F(), T()); return;
		case 6: Lambda(F(), T(), T()); return;
		case 7: Lambda(T(), T(), T()); return;
		default: checkVoxelSlow(false); return;
		}
	}

	template<typename T>
	struct THasForceInitConstructor
	{
		template<typename X, typename = decltype(T(ForceInit))>
		static uint8 Test(X*);
		
		template<typename X>
		static uint32 Test(...);

		static constexpr bool Value = sizeof(Test<T>(nullptr)) == sizeof(uint8);
	};

	// Works with void
	template<typename T>
	typename TEnableIf<!THasForceInitConstructor<T>::Value, T>::Type GetDefaultValue()
	{
		return T();
	}
	template<typename T>
	typename TEnableIf<THasForceInitConstructor<T>::Value, T>::Type GetDefaultValue()
	{
		return T(ForceInit);
	}
}