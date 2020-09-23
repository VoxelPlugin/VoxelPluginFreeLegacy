// Copyright 2020 Phyronnaz

#pragma once

#include "FastNoise/VoxelFastNoise_CellularNoise.h"
#include "FastNoise/VoxelFastNoiseBase.inl"

// THIS CODE IS A MODIFIED VERSION OF FAST NOISE: SEE LICENSE BELOW
//
// MIT License
//
// Copyright(c) 2017 Jordan Peck
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// The developer's email is jorzixdan.me2@gzixmail.com (for great email, take
// off every 'zix'.)
//
// VERSION: 0.4.1

// ReSharper disable CppUE4CodingStandardNamingViolationWarning

template<typename T>
FN_FORCEINLINE v_flt TVoxelFastNoise_CellularNoise<T>::GetCellular_2D(v_flt x, v_flt y, v_flt frequency) const
{
	x *= frequency;
	y *= frequency;
	
	switch (This().CellularReturnType)
	{
	case EVoxelCellularReturnType::CellValue:
	case EVoxelCellularReturnType::Distance:
	{
		switch (This().CellularDistanceFunction)
		{
		default: ensureVoxelSlow(false);
#define Macro(Enum) case Enum: return SingleCellular_2D<Enum>(x, y);
			FOREACH_ENUM_EVOXELCELLULARDISTANCEFUNCTION(Macro)
#undef Macro
		}
	}
	default:
	{
		switch (This().CellularDistanceFunction)
		{
		default: ensureVoxelSlow(false);
#define Macro(Enum) case Enum: return SingleCellular2Edge_2D<Enum>(x, y);
			FOREACH_ENUM_EVOXELCELLULARDISTANCEFUNCTION(Macro)
#undef Macro
		}
	}
	}
}

template<typename T>
FN_FORCEINLINE v_flt TVoxelFastNoise_CellularNoise<T>::GetCellular_3D(v_flt x, v_flt y, v_flt z, v_flt frequency) const
{
	x *= frequency;
	y *= frequency;
	z *= frequency;

	switch (This().CellularReturnType)
	{
	case EVoxelCellularReturnType::CellValue:
	case EVoxelCellularReturnType::Distance:
	{
		switch (This().CellularDistanceFunction)
		{
		default: ensureVoxelSlow(false);
#define Macro(Enum) case Enum: return SingleCellular_3D<Enum>(x, y, z);
			FOREACH_ENUM_EVOXELCELLULARDISTANCEFUNCTION(Macro)
#undef Macro
		}
	}
	default:
	{
		switch (This().CellularDistanceFunction)
		{
		default: ensureVoxelSlow(false);
#define Macro(Enum) case Enum: return SingleCellular2Edge_3D<Enum>(x, y, z);
			FOREACH_ENUM_EVOXELCELLULARDISTANCEFUNCTION(Macro)
#undef Macro
		}
	}
	}
}

template<typename T>
FN_FORCEINLINE void TVoxelFastNoise_CellularNoise<T>::GetVoronoi_2D(v_flt x, v_flt y, v_flt m_jitter, v_flt& out_x, v_flt& out_y) const
{
	switch (This().CellularDistanceFunction)
	{
	default: ensureVoxelSlow(false);
#define Macro(Enum) case Enum: return SingleVoronoi_2D<Enum>(x, y, m_jitter, out_x, out_y);
		FOREACH_ENUM_EVOXELCELLULARDISTANCEFUNCTION(Macro)
#undef Macro
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
template<EVoxelCellularDistanceFunction CellularDistance>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_CellularNoise<T>::SingleCellular_2D(v_flt x, v_flt y) const
{
	const int32 xr = FNoiseMath::FastRound(x);
	const int32 yr = FNoiseMath::FastRound(y);

	v_flt distance = 999999;
	int32 xc = 0;
	int32 yc = 0;

	for (int32 xi = xr - 1; xi <= xr + 1; xi++)
	{
		for (int32 yi = yr - 1; yi <= yr + 1; yi++)
		{
			const uint8 lutPos = This().Index2D_256(0, xi, yi);

			const v_flt vecX = xi - x + This().CELL_2D_X[lutPos] * This().CellularJitter;
			const v_flt vecY = yi - y + This().CELL_2D_Y[lutPos] * This().CellularJitter;

			const v_flt newDistance = CellularDistance_2D<CellularDistance>(vecX, vecY);
			if (newDistance < distance)
			{
				distance = newDistance;
				xc = xi;
				yc = yi;
			}
		}
	}

	switch (This().CellularReturnType)
	{
	default: ensureVoxelSlow(false);
	case EVoxelCellularReturnType::CellValue:
		return This().ValCoord2D(This().Seed, xc, yc);
	case EVoxelCellularReturnType::Distance:
		return distance;
	}
}

template<typename T>
template<EVoxelCellularDistanceFunction CellularDistance>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_CellularNoise<T>::SingleCellular_3D(v_flt x, v_flt y, v_flt z) const
{
	const int32 xr = FNoiseMath::FastRound(x);
	const int32 yr = FNoiseMath::FastRound(y);
	const int32 zr = FNoiseMath::FastRound(z);

	v_flt distance = 999999;
	int32 xc = 0;
	int32 yc = 0;
	int32 zc = 0;

	for (int32 xi = xr - 1; xi <= xr + 1; xi++)
	{
		for (int32 yi = yr - 1; yi <= yr + 1; yi++)
		{
			for (int32 zi = zr - 1; zi <= zr + 1; zi++)
			{
				const uint8 lutPos = This().Index3D_256(0, xi, yi, zi);

				const v_flt vecX = xi - x + This().CELL_3D_X[lutPos] * This().CellularJitter;
				const v_flt vecY = yi - y + This().CELL_3D_Y[lutPos] * This().CellularJitter;
				const v_flt vecZ = zi - z + This().CELL_3D_Z[lutPos] * This().CellularJitter;
				
				const v_flt newDistance = CellularDistance_3D<CellularDistance>(vecX, vecY, vecZ);
				if (newDistance < distance)
				{
					distance = newDistance;
					xc = xi;
					yc = yi;
					zc = zi;
				}
			}
		}
	}

	switch (This().CellularReturnType)
	{
	default: ensureVoxelSlow(false);
	case EVoxelCellularReturnType::CellValue:
		return This().ValCoord3D(This().Seed, xc, yc, zc);
	case EVoxelCellularReturnType::Distance:
		return distance;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
template<EVoxelCellularDistanceFunction CellularDistance>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_CellularNoise<T>::SingleCellular2Edge_2D(v_flt x, v_flt y) const
{
	// TODO expose?
	constexpr int32 m_cellularDistanceIndex0 = 0;
	constexpr int32 m_cellularDistanceIndex1 = 1;
	static_assert(m_cellularDistanceIndex1 > m_cellularDistanceIndex0, "");
	
	const int32 xr = FNoiseMath::FastRound(x);
	const int32 yr = FNoiseMath::FastRound(y);

	v_flt distance[m_cellularDistanceIndex1 + 1];
	for (auto& it : distance) it = 999999;
	
	for (int32 xi = xr - 1; xi <= xr + 1; xi++)
	{
		for (int32 yi = yr - 1; yi <= yr + 1; yi++)
		{
			const uint8 lutPos = This().Index2D_256(0, xi, yi);

			const v_flt vecX = xi - x + This().CELL_2D_X[lutPos] * This().CellularJitter;
			const v_flt vecY = yi - y + This().CELL_2D_Y[lutPos] * This().CellularJitter;

			const v_flt newDistance = CellularDistance_2D<CellularDistance>(vecX, vecY);
			for (int32 i = m_cellularDistanceIndex1; i > 0; i--)
				distance[i] = FMath::Max(FMath::Min(distance[i], newDistance), distance[i - 1]);
			distance[0] = FMath::Min(distance[0], newDistance);
		}
	}

	switch (This().CellularReturnType)
	{
	default: ensureVoxelSlow(false);
	case EVoxelCellularReturnType::Distance2:
		return distance[m_cellularDistanceIndex1];
	case EVoxelCellularReturnType::Distance2Add:
		return distance[m_cellularDistanceIndex1] + distance[m_cellularDistanceIndex0];
	case EVoxelCellularReturnType::Distance2Sub:
		return distance[m_cellularDistanceIndex1] - distance[m_cellularDistanceIndex0];
	case EVoxelCellularReturnType::Distance2Mul:
		return distance[m_cellularDistanceIndex1] * distance[m_cellularDistanceIndex0];
	case EVoxelCellularReturnType::Distance2Div:
		return distance[m_cellularDistanceIndex0] / distance[m_cellularDistanceIndex1];
	}
}

template<typename T>
template<EVoxelCellularDistanceFunction CellularDistance>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_CellularNoise<T>::SingleCellular2Edge_3D(v_flt x, v_flt y, v_flt z) const
{
	// TODO expose?
	constexpr int32 m_cellularDistanceIndex0 = 0;
	constexpr int32 m_cellularDistanceIndex1 = 1;
	static_assert(m_cellularDistanceIndex1 > m_cellularDistanceIndex0, "");
	
	const int32 xr = FNoiseMath::FastRound(x);
	const int32 yr = FNoiseMath::FastRound(y);
	const int32 zr = FNoiseMath::FastRound(z);
	
	v_flt distance[m_cellularDistanceIndex1 + 1];
	for (auto& it : distance) it = 999999;
	
	for (int32 xi = xr - 1; xi <= xr + 1; xi++)
	{
		for (int32 yi = yr - 1; yi <= yr + 1; yi++)
		{
			for (int32 zi = zr - 1; zi <= zr + 1; zi++)
			{
				const uint8 lutPos = This().Index3D_256(0, xi, yi, zi);

				const v_flt vecX = xi - x + This().CELL_3D_X[lutPos] * This().CellularJitter;
				const v_flt vecY = yi - y + This().CELL_3D_Y[lutPos] * This().CellularJitter;
				const v_flt vecZ = zi - z + This().CELL_3D_Z[lutPos] * This().CellularJitter;

				const v_flt newDistance = CellularDistance_3D<CellularDistance>(vecX, vecY, vecZ);

				for (int32 i = m_cellularDistanceIndex1; i > 0; i--)
					distance[i] = FMath::Max(FMath::Min(distance[i], newDistance), distance[i - 1]);
				distance[0] = FMath::Min(distance[0], newDistance);
			}
		}
	}
	
	switch (This().CellularReturnType)
	{
	default: ensureVoxelSlow(false);
	case EVoxelCellularReturnType::Distance2:
		return distance[m_cellularDistanceIndex1];
	case EVoxelCellularReturnType::Distance2Add:
		return distance[m_cellularDistanceIndex1] + distance[m_cellularDistanceIndex0];
	case EVoxelCellularReturnType::Distance2Sub:
		return distance[m_cellularDistanceIndex1] - distance[m_cellularDistanceIndex0];
	case EVoxelCellularReturnType::Distance2Mul:
		return distance[m_cellularDistanceIndex1] * distance[m_cellularDistanceIndex0];
	case EVoxelCellularReturnType::Distance2Div:
		return distance[m_cellularDistanceIndex0] / distance[m_cellularDistanceIndex1];
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
template<EVoxelCellularDistanceFunction CellularDistance>
FN_FORCEINLINE_SINGLE void TVoxelFastNoise_CellularNoise<T>::SingleVoronoi_2D(v_flt x, v_flt y, v_flt m_jitter, v_flt& out_x, v_flt& out_y) const
{
	const int32 xr = FNoiseMath::FastRound(x);
	const int32 yr = FNoiseMath::FastRound(y);

	v_flt distance = MAX_flt;
	
	for (int32 xi = xr - 1; xi <= xr + 1; xi++)
	{
		for (int32 yi = yr - 1; yi <= yr + 1; yi++)
		{
			const uint8 lutPos = This().Index2D_256(0, xi, yi);

			const v_flt newX = xi + This().CELL_2D_X[lutPos] * m_jitter;
			const v_flt newY = yi + This().CELL_2D_Y[lutPos] * m_jitter;
			const v_flt vecX = x - newX;
			const v_flt vecY = y - newY;

			const v_flt newDistance = CellularDistance_2D<CellularDistance>(vecX, vecY);
			if (newDistance < distance)
			{
				distance = newDistance;
				out_x = newX;
				out_y = newY;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_CellularNoise<T>::SingleCrater_2D(uint8 offset, v_flt x, v_flt y) const
{
	const int32 xi = FNoiseMath::FastFloor(x);
	const int32 yi = FNoiseMath::FastFloor(y);
	const v_flt xf = x - xi;
	const v_flt yf = y - yi;
	
	v_flt va = 0.f;
	v_flt wt = 0.f;

	for (int32 i = -1; i <= 1; i++)
	{
		for (int32 j = -1; j <= 1; j++)
		{
			const uint8 lutPos = This().Index2D_256(offset, xi + i, yi + j);

			const v_flt vecX = xf - (i + 0.5f + This().CELL_3D_X[lutPos] * This().CellularJitter);
			const v_flt vecY = yf - (j + 0.5f + This().CELL_3D_Y[lutPos] * This().CellularJitter);

			const v_flt sqDistance = vecX * vecX + vecY * vecY;
			AccumulateCrater(sqDistance, va, wt);
		}
	}
	
	return wt == 0 ? 0 : FNoiseMath::FastAbs(va / wt);
}

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_CellularNoise<T>::SingleCrater_3D(uint8 offset, v_flt x, v_flt y, v_flt z) const
{
	const int32 xi = FNoiseMath::FastFloor(x);
	const int32 yi = FNoiseMath::FastFloor(y);
	const int32 zi = FNoiseMath::FastFloor(z);
	const v_flt xf = x - xi;
	const v_flt yf = y - yi;
	const v_flt zf = z - zi;
	
	v_flt va = 0.f;
	v_flt wt = 0.f;

	for (int32 i = -1; i <= 1; i++)
	{
		for (int32 j = -1; j <= 1; j++)
		{
			for (int32 k = -1; k <= 1; k++)
			{
				const uint8 lutPos = This().Index3D_256(offset, xi + i, yi + j, zi + k);

				const v_flt vecX = xf - (i + 0.5f + This().CELL_3D_X[lutPos] * This().CellularJitter);
				const v_flt vecY = yf - (j + 0.5f + This().CELL_3D_Y[lutPos] * This().CellularJitter);
				const v_flt vecZ = zf - (k + 0.5f + This().CELL_3D_Z[lutPos] * This().CellularJitter);

				const v_flt sqDistance = vecX * vecX + vecY * vecY + vecZ * vecZ;
				AccumulateCrater(sqDistance, va, wt);
			}
		}
	}
	
	return wt == 0 ? 0 : FNoiseMath::FastAbs(va / wt);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_CellularNoise<T>::SingleGavoronoi_2D(uint8 offset, v_flt x, v_flt y, v_flt dirX, v_flt dirY, v_flt dirVariation) const
{
	// From https://www.shadertoy.com/view/llsGWl
	
	const int32 xi = FNoiseMath::FastFloor(x);
	const int32 yi = FNoiseMath::FastFloor(y);
	const v_flt xf = x - xi;
	const v_flt yf = y - yi;

	v_flt va = 0.f;
	v_flt wt = 0.f;

	for (int32 i = -1; i <= 1; i++)
	{
		for (int32 j = -1; j <= 1; j++)
		{
			const uint8 lutPos = This().Index2D_256(offset, xi + i, yi + j);
			const v_flt jitterX = This().CELL_3D_X[lutPos];
			const v_flt jitterY = This().CELL_3D_Y[lutPos];

			const v_flt vecX = xf - (i + 0.5f + jitterX * This().CellularJitter);
			const v_flt vecY = yf - (j + 0.5f + jitterY * This().CellularJitter);

			const v_flt sqDistance = vecX * vecX + vecY * vecY;
			const v_flt distance = std::sqrt(sqDistance);

			const v_flt w = std::exp(-4.f * distance);
			
			const v_flt noisyDirectionX = dirX + jitterX * dirVariation;
			const v_flt noisyDirectionY = dirY + jitterY * dirVariation;
			
			va += w * std::cos((vecX * noisyDirectionX + vecY * noisyDirectionY) * 2.f * PI);
			wt += w;
		}
	}

	return wt == 0 ? 0 : FNoiseMath::FastAbs(va / wt);
}

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_CellularNoise<T>::SingleGavoronoi_Erosion_2D(uint8 offset, v_flt x, v_flt y, v_flt dirX, v_flt dirY, v_flt& outDx, v_flt& outDy) const
{
	// From https://www.shadertoy.com/view/MtGcWh
	
	const int32 xi = FNoiseMath::FastFloor(x);
	const int32 yi = FNoiseMath::FastFloor(y);
	const v_flt xf = x - xi;
	const v_flt yf = y - yi;

	v_flt va = 0.f;
	v_flt va_dx = 0.f;
	v_flt va_dy = 0.f;
	v_flt wt = 0.f;

	for (int32 i = -2; i <= 2; i++)
	{
		for (int32 j = -2; j <= 2; j++)
		{
			const uint8 lutPos = This().Index2D_256(offset, xi + i, yi + j);
			const v_flt jitterX = This().CELL_3D_X[lutPos];
			const v_flt jitterY = This().CELL_3D_Y[lutPos];

			const v_flt vecX = xf - (i + 0.5f + jitterX * This().CellularJitter);
			const v_flt vecY = yf - (j + 0.5f + jitterY * This().CellularJitter);

			const v_flt sqDistance = vecX * vecX + vecY * vecY;

			const v_flt w = std::exp(-2.f * sqDistance);

			const v_flt sample_pos = (vecX * dirX + vecY * dirY) * 2.f * PI;
			const v_flt cos = std::cos(sample_pos);
			const v_flt sin = std::sin(sample_pos);
			
			va += w * cos;
			va_dx += -w * sin * (vecX + dirX);
			va_dy += -w * sin * (vecY + dirY);
			wt += w;
		}
	}

	if (wt == 0)
	{
		outDx = 0;
		outDy = 0;
		return 0;
	}
	else
	{
		outDx = va_dx / wt;
		outDy = va_dy / wt;
		return va / wt;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE void TVoxelFastNoise_CellularNoise<T>::GetVoronoiNeighbors_2D(
	v_flt x, v_flt y,
	v_flt m_jitter,
	v_flt& out_x0, v_flt& out_y0,
	v_flt& out_x1, v_flt& out_y1, v_flt& out_distance1,
	v_flt& out_x2, v_flt& out_y2, v_flt& out_distance2,
	v_flt& out_x3, v_flt& out_y3, v_flt& out_distance3) const
{
	const int32 xr = FNoiseMath::FastRound(x);
	const int32 yr = FNoiseMath::FastRound(y);
	
	const FVector2D Position(x, y);
	
	FVector2D BestCenter;
	{
		v_flt BestDistance = MAX_flt;

		for (int32 xi = xr - 1; xi <= xr + 1; xi++)
		{
			for (int32 yi = yr - 1; yi <= yr + 1; yi++)
			{
				const uint8 lutPos = This().Index2D_256(0, xi, yi);

				const v_flt NewX = xi + This().CELL_2D_X[lutPos] * m_jitter;
				const v_flt NewY = yi + This().CELL_2D_Y[lutPos] * m_jitter;
				const FVector2D Center(NewX, NewY);

				const v_flt NewDistance = (Position - Center).SizeSquared();

				if (BestDistance > NewDistance)
				{
					BestDistance = NewDistance;
					BestCenter = Center;
				}
			}
		}
	}
	
	constexpr int32 num_dists = 4;
	v_flt distance[num_dists] = { MAX_flt, MAX_flt, MAX_flt, MAX_flt };
	v_flt xs[num_dists];
	v_flt ys[num_dists];

	for (int32 xi = xr - 1; xi <= xr + 1; xi++)
	{
		for (int32 yi = yr - 1; yi <= yr + 1; yi++)
		{
			const uint8 lutPos = This().Index2D_256(0, xi, yi);

			v_flt NewX = xi + This().CELL_2D_X[lutPos] * m_jitter;
			v_flt NewY = yi + This().CELL_2D_Y[lutPos] * m_jitter;
			const FVector2D Center(NewX, NewY);

			v_flt NewDistance = FMath::Abs(FVector2D::DotProduct(Position - (Center + BestCenter) / 2, (BestCenter - Center).GetSafeNormal()));

			for (int32 i = 0; i < num_dists; ++i)
			{
				if (distance[i] > NewDistance)
				{
					Swap(distance[i], NewDistance);
					Swap(xs[i], NewX);
					Swap(ys[i], NewY);
				}
			}
		}
	}

	out_x0 = xs[0];
	out_x1 = xs[1];
	out_x2 = xs[2];
	out_x3 = xs[3];

	out_y0 = ys[0];
	out_y1 = ys[1];
	out_y2 = ys[2];
	out_y3 = ys[3];

	ensureVoxelSlow(FMath::IsNearlyZero(distance[0]));
	// distance0 is always 0 as it's the distance to the border but we're inside out_distance0 = distance[0];
	out_distance1 = distance[1];
	out_distance2 = distance[2];
	out_distance3 = distance[3];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
template<EVoxelCellularDistanceFunction CellularDistance>
FN_FORCEINLINE_MATH v_flt TVoxelFastNoise_CellularNoise<T>::CellularDistance_2D(v_flt vecX, v_flt vecY)
{
	switch (CellularDistance)
	{
	default: ensureVoxelSlow(false);
	case EVoxelCellularDistanceFunction::Euclidean:
		return vecX * vecX + vecY * vecY;
	case EVoxelCellularDistanceFunction::Manhattan:
		return FNoiseMath::FastAbs(vecX) + FNoiseMath::FastAbs(vecY);
	case EVoxelCellularDistanceFunction::Natural:
		return (FNoiseMath::FastAbs(vecX) + FNoiseMath::FastAbs(vecY)) + (vecX * vecX + vecY * vecY);
	}
}

template<typename T>
template<EVoxelCellularDistanceFunction CellularDistance>
FN_FORCEINLINE_MATH v_flt TVoxelFastNoise_CellularNoise<T>::CellularDistance_3D(v_flt vecX, v_flt vecY, v_flt vecZ)
{
	switch (CellularDistance)
	{
	default: ensureVoxelSlow(false);
	case EVoxelCellularDistanceFunction::Euclidean:
		return vecX * vecX + vecY * vecY + vecZ * vecZ;
	case EVoxelCellularDistanceFunction::Manhattan:
		return FNoiseMath::FastAbs(vecX) + FNoiseMath::FastAbs(vecY) + FNoiseMath::FastAbs(vecZ);
	case EVoxelCellularDistanceFunction::Natural:
		return (FNoiseMath::FastAbs(vecX) + FNoiseMath::FastAbs(vecY) + FNoiseMath::FastAbs(vecZ)) + (vecX * vecX + vecY * vecY + vecZ * vecZ);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt TVoxelFastNoise_CellularNoise<T>::GetErosion_2D(v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt noise_dx, v_flt noise_dy, v_flt& outDx, v_flt& outDy) const
{
	// From https://www.shadertoy.com/view/MtGcWh

	x *= frequency;
	y *= frequency;
    
    // Take the curl of the normal to get the gradient facing down the slope
	const v_flt dir_x = noise_dy;
	const v_flt dir_y = -noise_dx;

	v_flt r = 0;
	v_flt r_dx = 0;
	v_flt r_dy = 0;

	// Now we compute another fbm type noise
	// erosion is a type of noise with a strong directionality
	// we pass in the direction based on the slope of the terrain
	// erosion also returns the slope. we add that to a running total
	// so that the direction of successive layers are based on the
	// past layers
	v_flt amp = 1.f;
	for (int32 i = 0; i < octaves; i++)
	{
		v_flt erosion_dx, erosion_dy;
		const v_flt erosion = SingleGavoronoi_Erosion_2D(i, x, y, dir_x + r_dy, dir_y - r_dx, erosion_dx, erosion_dy);

		r += erosion * amp;
		r_dx += erosion_dx * amp;
		r_dy += erosion_dy * amp;
		
		x *= This().Lacunarity;
		y *= This().Lacunarity;
		amp *= This().Gain;
	}
	
	outDx = r_dx;
	outDy = r_dy;
	return r;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE_MATH void TVoxelFastNoise_CellularNoise<T>::AccumulateCrater(const v_flt sqDistance, v_flt& va, v_flt& wt) const
{
	// This is heavily inspired from https://www.shadertoy.com/view/MtjGRD

	// Early cull far craters
	if (sqDistance >= 1) return;
	
	const v_flt distance = std::sqrt(sqDistance);

	// Make sure that the function is nulled beyond 1, else we get issue when only iterating nearby craters
	v_flt w = FMath::Clamp<v_flt>((1.f - distance) * 2.f, 0, 1);

	// Exponential decrease
	w *= std::exp(-4.f * distance);

	v_flt multiplier = 1;
	if (This().CraterFalloffExponent != 0)
	{
		multiplier = std::exp(-This().CraterFalloffExponent * distance);
	}

	// Sin for the bump
	// + 0.055f so we don't get a bump in the middle of the crater
	va += w * std::sin(2.f * PI * std::sqrt(distance + 0.055f)) * multiplier;
	wt += w;
}