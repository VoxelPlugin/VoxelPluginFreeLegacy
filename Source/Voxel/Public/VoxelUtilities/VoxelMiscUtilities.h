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

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	template<int32 N>
	struct TInteger
	{
		FORCEINLINE constexpr operator int32() const
		{
			return N;
		}
	};

	template<int32 Num>
	struct TStaticSwitch;
	
#define VOXEL_STATIC_SWITCH_CASE(Index) case Index: Lambda(TInteger<Index>()); return;
#define VOXEL_STATIC_SWITCH_CASE_1 VOXEL_STATIC_SWITCH_CASE(0)
#define VOXEL_STATIC_SWITCH_CASE_2 VOXEL_STATIC_SWITCH_CASE_1 VOXEL_STATIC_SWITCH_CASE(1)
#define VOXEL_STATIC_SWITCH_CASE_3 VOXEL_STATIC_SWITCH_CASE_2 VOXEL_STATIC_SWITCH_CASE(2)
#define VOXEL_STATIC_SWITCH_CASE_4 VOXEL_STATIC_SWITCH_CASE_3 VOXEL_STATIC_SWITCH_CASE(3)
#define VOXEL_STATIC_SWITCH_CASE_5 VOXEL_STATIC_SWITCH_CASE_4 VOXEL_STATIC_SWITCH_CASE(4)
#define VOXEL_STATIC_SWITCH_CASE_6 VOXEL_STATIC_SWITCH_CASE_5 VOXEL_STATIC_SWITCH_CASE(5)
#define VOXEL_STATIC_SWITCH_CASE_7 VOXEL_STATIC_SWITCH_CASE_6 VOXEL_STATIC_SWITCH_CASE(6)
#define VOXEL_STATIC_SWITCH_CASE_8 VOXEL_STATIC_SWITCH_CASE_7 VOXEL_STATIC_SWITCH_CASE(7)
#define VOXEL_STATIC_SWITCH_CASE_9 VOXEL_STATIC_SWITCH_CASE_8 VOXEL_STATIC_SWITCH_CASE(8)
#define VOXEL_STATIC_SWITCH_CASE_10 VOXEL_STATIC_SWITCH_CASE_9 VOXEL_STATIC_SWITCH_CASE(9)
#define VOXEL_STATIC_SWITCH_CASE_11 VOXEL_STATIC_SWITCH_CASE_10 VOXEL_STATIC_SWITCH_CASE(10)
#define VOXEL_STATIC_SWITCH_CASE_12 VOXEL_STATIC_SWITCH_CASE_11 VOXEL_STATIC_SWITCH_CASE(11)
#define VOXEL_STATIC_SWITCH_CASE_13 VOXEL_STATIC_SWITCH_CASE_12 VOXEL_STATIC_SWITCH_CASE(12)
#define VOXEL_STATIC_SWITCH_CASE_14 VOXEL_STATIC_SWITCH_CASE_13 VOXEL_STATIC_SWITCH_CASE(13)
#define VOXEL_STATIC_SWITCH_CASE_15 VOXEL_STATIC_SWITCH_CASE_14 VOXEL_STATIC_SWITCH_CASE(14)
#define VOXEL_STATIC_SWITCH_CASE_16 VOXEL_STATIC_SWITCH_CASE_15 VOXEL_STATIC_SWITCH_CASE(15)
#define VOXEL_STATIC_SWITCH_CASE_17 VOXEL_STATIC_SWITCH_CASE_16 VOXEL_STATIC_SWITCH_CASE(16)
#define VOXEL_STATIC_SWITCH_CASE_18 VOXEL_STATIC_SWITCH_CASE_17 VOXEL_STATIC_SWITCH_CASE(17)
#define VOXEL_STATIC_SWITCH_CASE_19 VOXEL_STATIC_SWITCH_CASE_18 VOXEL_STATIC_SWITCH_CASE(18)
#define VOXEL_STATIC_SWITCH_CASE_20 VOXEL_STATIC_SWITCH_CASE_19 VOXEL_STATIC_SWITCH_CASE(19)
#define VOXEL_STATIC_SWITCH_CASE_21 VOXEL_STATIC_SWITCH_CASE_20 VOXEL_STATIC_SWITCH_CASE(20)
#define VOXEL_STATIC_SWITCH_CASE_22 VOXEL_STATIC_SWITCH_CASE_21 VOXEL_STATIC_SWITCH_CASE(21)
#define VOXEL_STATIC_SWITCH_CASE_23 VOXEL_STATIC_SWITCH_CASE_22 VOXEL_STATIC_SWITCH_CASE(22)
#define VOXEL_STATIC_SWITCH_CASE_24 VOXEL_STATIC_SWITCH_CASE_23 VOXEL_STATIC_SWITCH_CASE(23)
#define VOXEL_STATIC_SWITCH_CASE_25 VOXEL_STATIC_SWITCH_CASE_24 VOXEL_STATIC_SWITCH_CASE(24)
#define VOXEL_STATIC_SWITCH_CASE_26 VOXEL_STATIC_SWITCH_CASE_25 VOXEL_STATIC_SWITCH_CASE(25)
#define VOXEL_STATIC_SWITCH_CASE_27 VOXEL_STATIC_SWITCH_CASE_26 VOXEL_STATIC_SWITCH_CASE(26)
#define VOXEL_STATIC_SWITCH_CASE_28 VOXEL_STATIC_SWITCH_CASE_27 VOXEL_STATIC_SWITCH_CASE(27)
#define VOXEL_STATIC_SWITCH_CASE_29 VOXEL_STATIC_SWITCH_CASE_28 VOXEL_STATIC_SWITCH_CASE(28)
#define VOXEL_STATIC_SWITCH_CASE_30 VOXEL_STATIC_SWITCH_CASE_29 VOXEL_STATIC_SWITCH_CASE(29)
#define VOXEL_STATIC_SWITCH_CASE_31 VOXEL_STATIC_SWITCH_CASE_30 VOXEL_STATIC_SWITCH_CASE(30)
#define VOXEL_STATIC_SWITCH_CASE_32 VOXEL_STATIC_SWITCH_CASE_31 VOXEL_STATIC_SWITCH_CASE(31)
	
#define VOXEL_STATIC_SWITCH(Index) \
	template<> \
	struct TStaticSwitch<Index> \
	{ \
		template<typename TLambda> \
		static void Switch(int32 Value, TLambda Lambda) \
		{ \
			switch (Value) \
			{ \
			VOXEL_STATIC_SWITCH_CASE_##Index \
			default: ensureVoxelSlow(false); return; \
			} \
		} \
	};

	VOXEL_STATIC_SWITCH(4);
	VOXEL_STATIC_SWITCH(6);
	VOXEL_STATIC_SWITCH(8);
	VOXEL_STATIC_SWITCH(12);

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

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