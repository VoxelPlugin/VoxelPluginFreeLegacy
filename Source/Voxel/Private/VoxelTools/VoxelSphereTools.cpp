// Copyright 2019 Phyronnaz

#include "VoxelTools/VoxelSphereTools.h"
#include "VoxelTools/VoxelToolsHelpers.h"
#include "VoxelTools/VoxelLatentActionHelpers.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "Curves/CurveFloat.h"

DECLARE_CYCLE_STAT(TEXT("UVoxelSphereTools::SetValueSphere"),    STAT_UVoxelSphereTools_SetValueSphere, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelSphereTools::AddSphere"),         STAT_UVoxelSphereTools_AddSphere, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelSphereTools::RemoveSphere"),      STAT_UVoxelSphereTools_RemoveSphere, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelSphereTools::SetMaterialSphere"), STAT_UVoxelSphereTools_SetMaterialSphere, STATGROUP_Voxel);

inline FIntBox GetSphereBounds(float Radius, const FIntVector& Position)
{
	FIntVector R(FMath::CeilToInt(Radius) + 3);
	return FIntBox(Position - R, Position + R);
}

template<typename T>
inline bool SphereHelper(const FString& Name, AVoxelWorld* World, const FIntVector& Position, float Radius, bool bUpdateRender, bool bAllowFailure, T Lambda)
{
	auto Bounds = GetSphereBounds(Radius, Position);
	return FVoxelToolsHelpers::EditToolsHelper<EVoxelLockType::ReadWrite>(Name, World, Bounds, bUpdateRender, bAllowFailure, [&](FVoxelData& Data) { Lambda(Data, Bounds); });
}

void SetValueSphereImpl(FVoxelData& Data, const FIntBox& Bounds, const FIntVector& Position, float Radius, float Value)
{
	const float SquaredRadius = FMath::Square(Radius);
	Data.SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& OldValue)
	{
		uint64 SquaredDistance = FVoxelIntVector::SquaredSize(FIntVector(X - Position.X, Y - Position.Y, Z - Position.Z));
		if (SquaredDistance <= SquaredRadius)
		{
			OldValue = Value;
		}
	});
}

template<bool bAdd>
void SphereEditImpl(FVoxelData& Data, const FIntBox& Bounds, const FIntVector& Position, float Radius)
{
	const float SquaredRadiusPlus2 = FMath::Square(Radius + 2);
	const float SquaredRadiusMinus2 = FMath::Square(Radius - 2);
	Data.SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& OldValue)
	{
		float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();

		if (SquaredDistance <= SquaredRadiusMinus2)
		{
			OldValue = bAdd ? FVoxelValue::Full : FVoxelValue::Empty;
		}
		else if (SquaredDistance <= SquaredRadiusPlus2)
		{
			float Distance = FMath::Sqrt(SquaredDistance);
			FVoxelValue NewValue = FMath::Clamp(Radius - Distance, -2.f, 2.f) / 2 * (bAdd ? -1 : 1);

			if ((bAdd && !NewValue.IsEmpty()) || (!bAdd && NewValue.IsEmpty()) || FVoxelValue::HaveSameSign(OldValue, NewValue))
			{
				OldValue = NewValue;
			}
		}
	});
}

void SetMaterialSphereImpl(FVoxelData& Data, const FIntBox& Bounds, const FIntVector& Position, float Radius, const FVoxelPaintMaterial& PaintMaterial, const FRichCurve* StrengthCurve)
{
	const float SquaredRadius = FMath::Square(Radius);
	Data.SetValueOrMaterialLambda<FVoxelMaterial>(Bounds, [&](int X, int Y, int Z, FVoxelMaterial& Material)
	{
		float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance <= SquaredRadius)
		{
			PaintMaterial.ApplyToMaterial(Material, StrengthCurve ? StrengthCurve->Eval(FMath::Sqrt(SquaredDistance / SquaredRadius)) : 1);
		}
	});
}

///////////////////////////////////////////////////////////////////////////////

bool UVoxelSphereTools::SetValueSphere(AVoxelWorld* World, const FIntVector& Position, float Radius, float Value, bool bUpdateRender, bool bAllowFailure)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelSphereTools_SetValueSphere);
	CHECK_VOXELWORLD_IS_CREATED();
	return SphereHelper(__FUNCTION__, World, Position, Radius, bUpdateRender, bAllowFailure, [&](FVoxelData& Data, const FIntBox& Bounds)
	{
		SetValueSphereImpl(Data, Bounds, Position, Radius, Value);
	});
}

bool UVoxelSphereTools::AddSphere(AVoxelWorld* World, const FIntVector& Position, float Radius, bool bUpdateRender, bool bAllowFailure)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelSphereTools_AddSphere);
	CHECK_VOXELWORLD_IS_CREATED();
	return SphereHelper(__FUNCTION__, World, Position, Radius, bUpdateRender, bAllowFailure, [&](FVoxelData& Data, const FIntBox& Bounds)
	{
		SphereEditImpl<true>(Data, Bounds, Position, Radius);
	});
}

bool UVoxelSphereTools::RemoveSphere(AVoxelWorld* World, const FIntVector& Position, float Radius, bool bUpdateRender, bool bAllowFailure)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelSphereTools_RemoveSphere);
	CHECK_VOXELWORLD_IS_CREATED();
	return SphereHelper(__FUNCTION__, World, Position, Radius, bUpdateRender, bAllowFailure, [&](FVoxelData& Data, const FIntBox& Bounds)
	{
		SphereEditImpl<false>(Data, Bounds, Position, Radius);
	});
}

bool UVoxelSphereTools::SetMaterialSphere(AVoxelWorld* World, const FIntVector& Position, float Radius, const FVoxelPaintMaterial& PaintMaterial, UCurveFloat* StrengthCurve, bool bUpdateRender, bool bAllowFailure)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelSphereTools_SetMaterialSphere);
	CHECK_VOXELWORLD_IS_CREATED();
	return SphereHelper(__FUNCTION__, World, Position, Radius, bUpdateRender, bAllowFailure, [&](FVoxelData& Data, const FIntBox& Bounds)
	{
		SetMaterialSphereImpl(Data, Bounds, Position, Radius, PaintMaterial, StrengthCurve ? &StrengthCurve->FloatCurve : nullptr);
	});
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
inline bool SphereAsyncHelper(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo,
	const FString& Name, 
	AVoxelWorld* World, 
	const FIntVector& Position, 
	float Radius, 
	bool bUpdateRender, 
	T Lambda)
{
	auto Bounds = GetSphereBounds(Radius, Position);
	return FVoxelLatentActionHelpers::AsyncHelper(WorldContextObject, LatentInfo, Name, World, Bounds, bUpdateRender, [Lambda, Bounds, Name](FVoxelData& Data)
	{
		FVoxelReadWriteScopeLock Lock(Data, Bounds, Name);
		Lambda(Data, Bounds);
	});
}

void UVoxelSphereTools::SetValueSphereAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World, 
	const FIntVector& Position, 
	float Radius, 
	float Value, 
	bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	SphereAsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, Position, Radius, bUpdateRender, [Position, Radius, Value](FVoxelData& Data, const FIntBox& Bounds)
	{
		SetValueSphereImpl(Data, Bounds, Position, Radius, Value);
	});
}

void UVoxelSphereTools::AddSphereAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World,
	const FIntVector& Position,
	float Radius, 
	bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	SphereAsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, Position, Radius, bUpdateRender, [Position, Radius](FVoxelData& Data, const FIntBox& Bounds)
	{
		SphereEditImpl<true>(Data, Bounds, Position, Radius);
	});
}

void UVoxelSphereTools::RemoveSphereAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World, 
	const FIntVector& Position, 
	float Radius, 
	bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	SphereAsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, Position, Radius, bUpdateRender, [Position, Radius](FVoxelData& Data, const FIntBox& Bounds)
	{
		SphereEditImpl<false>(Data, Bounds, Position, Radius);
	});
}

void UVoxelSphereTools::SetMaterialSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	const FIntVector& Position,
	float Radius,
	const FVoxelPaintMaterial& PaintMaterial,
	UCurveFloat* StrengthCurve,
	bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	SphereAsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, Position, Radius, bUpdateRender,
		[
			Position,
			Radius,
			PaintMaterial,
			StrengthCurveCopy = StrengthCurve ? StrengthCurve->FloatCurve : FRichCurve(),
			bUseStrengthCurve = StrengthCurve != nullptr](FVoxelData& Data, const FIntBox& Bounds)
	{
		SetMaterialSphereImpl(Data, Bounds, Position, Radius, PaintMaterial, bUseStrengthCurve ? &StrengthCurveCopy : nullptr);
	});
}
