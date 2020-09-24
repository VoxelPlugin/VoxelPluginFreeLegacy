// Copyright 2020 Phyronnaz

#include "VoxelRender/Meshers/VoxelCubicMesher.h"
#include "VoxelRender/Meshers/VoxelMesherUtilities.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelData/VoxelDataIncludes.h"

struct FVoxelCubicFullVertex : FVoxelMesherVertex
{
	static constexpr bool bComputeNormal = true;
	static constexpr bool bComputeTangent = true;
	static constexpr bool bComputeTextureCoordinate = true;
	static constexpr bool bComputeMaterial = true;
	
	FORCEINLINE void SetPosition(const FVector& InPosition)
	{
		Position = InPosition;
	}
	FORCEINLINE void SetNormal(const FVector& InNormal)
	{
		Normal = InNormal;
	}
	FORCEINLINE void SetTangent(const FVoxelProcMeshTangent& InTangent)
	{
		Tangent = InTangent;
	}
	FORCEINLINE void SetTextureCoordinate(const FVector2D& InTextureCoordinate)
	{
		TextureCoordinate = InTextureCoordinate;
	}
	FORCEINLINE void SetMaterial(const FVoxelMaterial& InMaterial)
	{
		Material = InMaterial;
	}
};
static_assert(sizeof(FVoxelCubicFullVertex) == sizeof(FVoxelMesherVertex), "");

struct FVoxelCubicGeometryVertex : FVector
{
	static constexpr bool bComputeNormal = false;
	static constexpr bool bComputeTangent = false;
	static constexpr bool bComputeTextureCoordinate = false;
	static constexpr bool bComputeMaterial = false;

	FORCEINLINE void SetPosition(const FVector& InPosition)
	{
		static_cast<FVector&>(*this) = InPosition;
	}
	FORCEINLINE void SetNormal(const FVector&)
	{
		checkVoxelSlow(false);
	}
	FORCEINLINE void SetTangent(const FVoxelProcMeshTangent&)
	{
		checkVoxelSlow(false);
	}
	FORCEINLINE void SetTextureCoordinate(const FVector2D&)
	{
		checkVoxelSlow(false);
	}
	FORCEINLINE void SetMaterial(const FVoxelMaterial&)
	{
		checkVoxelSlow(false);
	}
};
static_assert(sizeof(FVoxelCubicGeometryVertex) == sizeof(FVector), "");

template<EVoxelDirectionFlag::Type Direction, typename TVertex, typename TMesher>
FORCEINLINE void AddFace(
	TMesher& Mesher, int32 Step, FVoxelMaterial Material, 
	int32 X, int32 Y, int32 Z, 
	TArray<uint32>& Indices, TArray<TVertex>& Vertices)
{
	if (TVertex::bComputeMaterial && Mesher.Settings.bOneMaterialPerCubeSide)
	{
		uint8 Index = Material.GetSingleIndex();
		
		switch (Direction)
		{
		case EVoxelDirectionFlag::XMin:
		case EVoxelDirectionFlag::XMax:
		case EVoxelDirectionFlag::YMin:
		case EVoxelDirectionFlag::YMax:
			Index = 3 * Index + 1;
			break;
		case EVoxelDirectionFlag::ZMin:
			Index = 3 * Index + 2;
			break;
		case EVoxelDirectionFlag::ZMax:
			Index = 3 * Index + 0;
			break;
		}

		Material.SetSingleIndex(Index);
	}
	
	FVector Positions[4];
	FVector Normal;
	FVector Tangent;

	/**
	 * 0 --- 1
	 * |  \  |
	 * 3 --- 2
	 * 
	 * Triangles: 0 1 2, 0 2 3
	 */

	switch (Direction)
	{
	case EVoxelDirectionFlag::XMin:
		Positions[0] = FVector(0, 0, 1);
		Positions[1] = FVector(0, 1, 1);
		Positions[2] = FVector(0, 1, 0);
		Positions[3] = FVector(0, 0, 0);
		Normal = FVector(-1, 0, 0);
		Tangent = FVector(0, 1, 0);
		break;
	case EVoxelDirectionFlag::XMax:
		Positions[0] = FVector(1, 1, 1);
		Positions[1] = FVector(1, 0, 1);
		Positions[2] = FVector(1, 0, 0);
		Positions[3] = FVector(1, 1, 0);
		Normal = FVector(1, 0, 0);
		Tangent = FVector(0, -1, 0);
		break;
	case EVoxelDirectionFlag::YMin:
		Positions[0] = FVector(1, 0, 1);
		Positions[1] = FVector(0, 0, 1);
		Positions[2] = FVector(0, 0, 0);
		Positions[3] = FVector(1, 0, 0);
		Normal = FVector(0, -1, 0);
		Tangent = FVector(-1, 0, 0);
		break;
	case EVoxelDirectionFlag::YMax:
		Positions[0] = FVector(0, 1, 1);
		Positions[1] = FVector(1, 1, 1);
		Positions[2] = FVector(1, 1, 0);
		Positions[3] = FVector(0, 1, 0);
		Normal = FVector(0, 1, 0);
		Tangent = FVector(1, 0, 0);
		break;
	case EVoxelDirectionFlag::ZMin:
		Positions[0] = FVector(0, 1, 0);
		Positions[1] = FVector(1, 1, 0);
		Positions[2] = FVector(1, 0, 0);
		Positions[3] = FVector(0, 0, 0);
		Normal = FVector(0, 0, -1);
		Tangent = FVector(1, 0, 0);
		break;
	default:
		check(Direction == EVoxelDirectionFlag::ZMax);
		Positions[0] = FVector(1, 0, 1);
		Positions[1] = FVector(1, 1, 1);
		Positions[2] = FVector(0, 1, 1);
		Positions[3] = FVector(0, 0, 1);
		Normal = FVector(0, 0, 1);
		Tangent = FVector(1, 0, 0);
		break;
	}

	int32 PositionsIndices[4];
	for (int32 Index = 0; Index < 4; Index++)
	{
		const FVector VertexPositionInCube = Positions[Index];
		const FVector VertexPosition = (VertexPositionInCube + FVector(X, Y, Z)) * Step - FVector(0.5f);
		
		TVertex Vertex;
		Vertex.SetPosition(VertexPosition);
		if (TVertex::bComputeNormal) Vertex.SetNormal(Normal);
		if (TVertex::bComputeTangent) Vertex.SetTangent(FVoxelProcMeshTangent(Tangent, false));
		if (TVertex::bComputeMaterial) Vertex.SetMaterial(Material);

		if (TVertex::bComputeTextureCoordinate)
		{
			FVector2D TextureCoordinate;
			if (Mesher.Settings.UVConfig == EVoxelUVConfig::GlobalUVs)
			{
				const FVector V = VertexPosition + FVector(Mesher.ChunkPosition);
				switch (Direction)
				{
				case EVoxelDirectionFlag::XMin:
					TextureCoordinate = { V.Y, V.Z };
					break;
				case EVoxelDirectionFlag::XMax:
					TextureCoordinate = { -V.Y, V.Z };
					break;
				case EVoxelDirectionFlag::YMin:
					TextureCoordinate = { -V.X, V.Z };
					break;
				case EVoxelDirectionFlag::YMax:
					TextureCoordinate = { V.X, V.Z };
					break;
				case EVoxelDirectionFlag::ZMin:
					TextureCoordinate = { V.X, V.Y };
					break;
				default:
					check(Direction == EVoxelDirectionFlag::ZMax);
					TextureCoordinate = { V.X, -V.Y };
					break;
				}
				TextureCoordinate /= Mesher.Settings.UVScale;
				TextureCoordinate.Y *= -1; // Y is down
			}
			else if (Mesher.Settings.UVConfig == EVoxelUVConfig::PackWorldUpInUVs)
			{
				TextureCoordinate = FVoxelMesherUtilities::GetUVs(Mesher, FVector(X, Y, Z));
			}
			else
			{
				check(Mesher.Settings.UVConfig == EVoxelUVConfig::PerVoxelUVs);
				const auto& V = VertexPositionInCube;
				switch (Direction)
				{
				case EVoxelDirectionFlag::XMin:
					TextureCoordinate = { V.Y, V.Z };
					break;
				case EVoxelDirectionFlag::XMax:
					TextureCoordinate = { 1 - V.Y, V.Z };
					break;
				case EVoxelDirectionFlag::YMin:
					TextureCoordinate = { 1 - V.X, V.Z };
					break;
				case EVoxelDirectionFlag::YMax:
					TextureCoordinate = { V.X, V.Z };
					break;
				case EVoxelDirectionFlag::ZMin:
					TextureCoordinate = { V.X, V.Y };
					break;
				default:
					check(Direction == EVoxelDirectionFlag::ZMax);
					TextureCoordinate = { V.X, 1 - V.Y };
					break;
				}
				TextureCoordinate.Y = 1 - TextureCoordinate.Y; // Y is down
			}
			Vertex.SetTextureCoordinate(TextureCoordinate);
		}

		PositionsIndices[Index] = Vertices.Num();
		Vertices.Emplace(Vertex);
	}
	
	Indices.Add(PositionsIndices[2]);
	Indices.Add(PositionsIndices[1]);
	Indices.Add(PositionsIndices[0]);

	Indices.Add(PositionsIndices[3]);
	Indices.Add(PositionsIndices[2]);
	Indices.Add(PositionsIndices[0]);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelIntBox FVoxelCubicMesher::GetBoundsToCheckIsEmptyOn() const
{
	return FVoxelIntBox(ChunkPosition - FIntVector(Step), ChunkPosition - FIntVector(Step) + CUBIC_CHUNK_SIZE_WITH_NEIGHBORS * Step);
}

FVoxelIntBox FVoxelCubicMesher::GetBoundsToLock() const
{
	return GetBoundsToCheckIsEmptyOn();
}

TVoxelSharedPtr<FVoxelChunkMesh> FVoxelCubicMesher::CreateFullChunkImpl(FVoxelMesherTimes& Times)
{
	TArray<FVoxelCubicFullVertex> Vertices;
	TArray<uint32> Indices;

	CreateGeometryTemplate(Times, Indices, Vertices);

	UnlockData();
	
	return MESHER_TIME_RETURN(CreateChunk, FVoxelMesherUtilities::CreateChunkFromVertices(
		Settings,
		LOD,
		MoveTemp(Indices),
		MoveTemp(reinterpret_cast<TArray<FVoxelMesherVertex>&>(Vertices))));
}


void FVoxelCubicMesher::CreateGeometryImpl(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<FVector>& Vertices)
{
	CreateGeometryTemplate(Times, Indices, reinterpret_cast<TArray<FVoxelCubicGeometryVertex>&>(Vertices));
	UnlockData();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void FVoxelCubicMesher::CreateGeometryTemplate(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<T>& Vertices)
{
	if (T::bComputeMaterial)
	{
		Accelerator = MakeUnique<FVoxelConstDataAccelerator>(Data, GetBoundsToLock());
	}

	TVoxelQueryZone<FVoxelValue> QueryZone(GetBoundsToCheckIsEmptyOn(), FIntVector(CUBIC_CHUNK_SIZE_WITH_NEIGHBORS), LOD, CachedValues);
	MESHER_TIME_VALUES(CUBIC_CHUNK_SIZE_WITH_NEIGHBORS * CUBIC_CHUNK_SIZE_WITH_NEIGHBORS * CUBIC_CHUNK_SIZE_WITH_NEIGHBORS, Data.Get<FVoxelValue>(QueryZone, LOD));
	
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Iteration");
		for (int32 X = 0; X < RENDER_CHUNK_SIZE; X++)
		{
			for (int32 Y = 0; Y < RENDER_CHUNK_SIZE; Y++)
			{
				for (int32 Z = 0; Z < RENDER_CHUNK_SIZE; Z++)
				{
					const FVoxelValue Value = GetValue(X, Y, Z);
					if (Value.IsEmpty()) continue;

					const uint8 Flag =
						(GetValue(X - 1, Y, Z).IsEmpty() << 0) |
						(GetValue(X + 1, Y, Z).IsEmpty() << 1) |
						(GetValue(X, Y - 1, Z).IsEmpty() << 2) |
						(GetValue(X, Y + 1, Z).IsEmpty() << 3) |
						(GetValue(X, Y, Z - 1).IsEmpty() << 4) |
						(GetValue(X, Y, Z + 1).IsEmpty() << 5);

					if (!Flag) continue;

					FVoxelMaterial Material;
					if (T::bComputeMaterial)
					{
						Material = MESHER_TIME_RETURN_MATERIALS(1, Accelerator->GetMaterial(
							X + ChunkPosition.X,
							Y + ChunkPosition.Y,
							Z + ChunkPosition.Z,
							LOD));
					}

#define CHECK_SIDE(Direction) if (Flag & Direction) AddFace<Direction>(*this, Step, Material, X, Y, Z, Indices, Vertices)
					CHECK_SIDE(EVoxelDirectionFlag::XMin);
					CHECK_SIDE(EVoxelDirectionFlag::XMax);
					CHECK_SIDE(EVoxelDirectionFlag::YMin);
					CHECK_SIDE(EVoxelDirectionFlag::YMax);
					CHECK_SIDE(EVoxelDirectionFlag::ZMin);
					CHECK_SIDE(EVoxelDirectionFlag::ZMax);
#undef CHECK_SIDE
				}
			}
		}
	}
}

FORCEINLINE FVoxelValue FVoxelCubicMesher::GetValue(int32 X, int32 Y, int32 Z) const
{
	checkVoxelSlow(
		-1 <= X &&
		-1 <= Y &&
		-1 <= Z &&
		X <= RENDER_CHUNK_SIZE &&
		Y <= RENDER_CHUNK_SIZE &&
		Z <= RENDER_CHUNK_SIZE);
	const int32 Index =
			(X + 1) +
			(Y + 1) * CUBIC_CHUNK_SIZE_WITH_NEIGHBORS +
			(Z + 1) * CUBIC_CHUNK_SIZE_WITH_NEIGHBORS * CUBIC_CHUNK_SIZE_WITH_NEIGHBORS;
	return CachedValues[Index];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelIntBox FVoxelCubicTransitionsMesher::GetBoundsToCheckIsEmptyOn() const
{
	return FVoxelIntBox(ChunkPosition - FIntVector(Step), ChunkPosition - FIntVector(Step) + CUBIC_CHUNK_SIZE_WITH_NEIGHBORS * Step);
}

FVoxelIntBox FVoxelCubicTransitionsMesher::GetBoundsToLock() const
{
	return GetBoundsToCheckIsEmptyOn();
}

TVoxelSharedPtr<FVoxelChunkMesh> FVoxelCubicTransitionsMesher::CreateFullChunkImpl(FVoxelMesherTimes& Times)
{
	Accelerator = MakeUnique<FVoxelConstDataAccelerator>(Data, GetBoundsToLock());

	TArray<FVoxelCubicFullVertex> Vertices;
	TArray<uint32> Indices;

	CreateTransitionsForDirection<EVoxelDirectionFlag::XMin>(Times, Indices, Vertices);
	CreateTransitionsForDirection<EVoxelDirectionFlag::XMax>(Times, Indices, Vertices);
	CreateTransitionsForDirection<EVoxelDirectionFlag::YMin>(Times, Indices, Vertices);
	CreateTransitionsForDirection<EVoxelDirectionFlag::YMax>(Times, Indices, Vertices);
	CreateTransitionsForDirection<EVoxelDirectionFlag::ZMin>(Times, Indices, Vertices);
	CreateTransitionsForDirection<EVoxelDirectionFlag::ZMax>(Times, Indices, Vertices);

	UnlockData();

	return MESHER_TIME_RETURN(CreateChunk, FVoxelMesherUtilities::CreateChunkFromVertices(
		Settings,
		LOD,
		MoveTemp(Indices),
		MoveTemp(reinterpret_cast<TArray<FVoxelMesherVertex>&>(Vertices))));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<EVoxelDirectionFlag::Type Direction>
inline constexpr EVoxelDirectionFlag::Type InverseVoxelDirection()
{
	switch (Direction)
	{
	case EVoxelDirectionFlag::XMin:
		return EVoxelDirectionFlag::XMax;
	case EVoxelDirectionFlag::XMax:
		return EVoxelDirectionFlag::XMin;
	case EVoxelDirectionFlag::YMin:
		return EVoxelDirectionFlag::YMax;
	case EVoxelDirectionFlag::YMax:
		return EVoxelDirectionFlag::YMin;
	case EVoxelDirectionFlag::ZMin:
		return EVoxelDirectionFlag::ZMax;
	case EVoxelDirectionFlag::ZMax:
	default:
		return EVoxelDirectionFlag::ZMin;
	}
}

template<EVoxelDirectionFlag::Type Direction, typename TVertex>
void FVoxelCubicTransitionsMesher::CreateTransitionsForDirection(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<TVertex>& Vertices)
{
	if (!(TransitionsMask & Direction)) return;

	for (int32 LX = 0; LX < RENDER_CHUNK_SIZE; LX++)
	{
		for (int32 LY = 0; LY < RENDER_CHUNK_SIZE; LY++)
		{
			// "Big" denotes the low resolution LOD voxels
			// "Small" denotes the high resolution LOD voxels
			// "Other Side" is the name of neighbor we are creating transitions for
			// "Other Side" neighbor has a higher resolution than us (their LOD = our LOD - 1)
			
			const FVoxelValue BigValue = MESHER_TIME_RETURN_VALUES(1, GetValue<Direction>(Step, LX * Step, LY * Step, 0));
			if (!BigValue.IsEmpty())
			{
				// First case: the big voxel is full
				
				const FVoxelValue BigOtherSideValue = MESHER_TIME_RETURN_VALUES(1, GetValue<Direction>(Step, LX * Step, LY * Step, -Step));
				// Face is already created by low res normal mesher
				if (BigOtherSideValue.IsEmpty())
				{
					continue;
				}

				// Query the high res other side values
				
				FVoxelValue SmallOtherSideValues[4];
				{
					MESHER_TIME_SCOPE_VALUES(4);
					SmallOtherSideValues[0] = GetValue<Direction>(HalfStep, (2 * LX + 0) * HalfStep, (2 * LY + 0) * HalfStep, -HalfStep);
					SmallOtherSideValues[1] = GetValue<Direction>(HalfStep, (2 * LX + 1) * HalfStep, (2 * LY + 0) * HalfStep, -HalfStep);
					SmallOtherSideValues[2] = GetValue<Direction>(HalfStep, (2 * LX + 0) * HalfStep, (2 * LY + 1) * HalfStep, -HalfStep);
					SmallOtherSideValues[3] = GetValue<Direction>(HalfStep, (2 * LX + 1) * HalfStep, (2 * LY + 1) * HalfStep, -HalfStep);
				}

				if (!SmallOtherSideValues[0].IsEmpty() &&
					!SmallOtherSideValues[1].IsEmpty() &&
					!SmallOtherSideValues[2].IsEmpty() &&
					!SmallOtherSideValues[3].IsEmpty())
				{
					// All the higher res LOD values are full too, nothing to do
					continue;

				}

				// Need to do some stitching

				// The new faces are facing outwards, same direction as the transitions
				constexpr EVoxelDirectionFlag::Type FaceDirection = Direction;
				
				const auto Material = MESHER_TIME_RETURN_MATERIALS(1, GetMaterial<Direction>(Step, LX * Step, LY * Step, 0));
				Add2DFace<Direction, FaceDirection>(Step, Material, LX, LY, Vertices, Indices);
			}
			else
			{
				FVoxelValue SmallOtherSideValues[4];
				{
					MESHER_TIME_SCOPE_VALUES(4);
					SmallOtherSideValues[0] = GetValue<Direction>(HalfStep, (2 * LX + 0) * HalfStep, (2 * LY + 0) * HalfStep, -HalfStep);
					SmallOtherSideValues[1] = GetValue<Direction>(HalfStep, (2 * LX + 1) * HalfStep, (2 * LY + 0) * HalfStep, -HalfStep);
					SmallOtherSideValues[2] = GetValue<Direction>(HalfStep, (2 * LX + 0) * HalfStep, (2 * LY + 1) * HalfStep, -HalfStep);
					SmallOtherSideValues[3] = GetValue<Direction>(HalfStep, (2 * LX + 1) * HalfStep, (2 * LY + 1) * HalfStep, -HalfStep);
				}

				const uint8 IsFullOtherSide =
					((!SmallOtherSideValues[0].IsEmpty()) << 0) |
					((!SmallOtherSideValues[1].IsEmpty()) << 1) |
					((!SmallOtherSideValues[2].IsEmpty()) << 2) |
					((!SmallOtherSideValues[3].IsEmpty()) << 3);

				// All the voxels in the high res chunk are empty too, nothing to do
				if (!IsFullOtherSide) continue;
				
				FVoxelValue SmallValues[4];
				{
					MESHER_TIME_SCOPE_VALUES(4);
					SmallValues[0] = GetValue<Direction>(HalfStep, (2 * LX + 0) * HalfStep, (2 * LY + 0) * HalfStep, 0);
					SmallValues[1] = GetValue<Direction>(HalfStep, (2 * LX + 1) * HalfStep, (2 * LY + 0) * HalfStep, 0);
					SmallValues[2] = GetValue<Direction>(HalfStep, (2 * LX + 0) * HalfStep, (2 * LY + 1) * HalfStep, 0);
					SmallValues[3] = GetValue<Direction>(HalfStep, (2 * LX + 1) * HalfStep, (2 * LY + 1) * HalfStep, 0);
				}

				const uint8 IsFull =
					((!SmallValues[0].IsEmpty()) << 0) |
					((!SmallValues[1].IsEmpty()) << 1) |
					((!SmallValues[2].IsEmpty()) << 2) |
					((!SmallValues[3].IsEmpty()) << 3);

				const uint8 AreBothFull = IsFullOtherSide & IsFull;

				if (!AreBothFull) continue;

				// When AreBothFull is true, the high res mesher did not create a face, and we need to add one ourselves

				// The face direction is from the high res to the low res: the opposite of the transition direction,
				// which is low res to high res (ie us to other)
				constexpr EVoxelDirectionFlag::Type FaceDirection = InverseVoxelDirection<Direction>();
				
				const auto Material = MESHER_TIME_RETURN_MATERIALS(1, GetMaterial<Direction>(Step, LX * Step, LY * Step, -HalfStep));
				if (AreBothFull & 0x1)
				{
					Add2DFace<Direction, FaceDirection>(HalfStep, Material, 2 * LX + 0, 2 * LY + 0, Vertices, Indices);
				}
				if (AreBothFull & 0x2)
				{
					Add2DFace<Direction, FaceDirection>(HalfStep, Material, 2 * LX + 1, 2 * LY + 0, Vertices, Indices);
				}
				if (AreBothFull & 0x4)
				{
					Add2DFace<Direction, FaceDirection>(HalfStep, Material, 2 * LX + 0, 2 * LY + 1, Vertices, Indices);
				}
				if (AreBothFull & 0x8)
				{
					Add2DFace<Direction, FaceDirection>(HalfStep, Material, 2 * LX + 1, 2 * LY + 1, Vertices, Indices);
				}
			}
		}
	}
}

template<EVoxelDirectionFlag::Type Direction>
FORCEINLINE FVoxelValue FVoxelCubicTransitionsMesher::GetValue(int32 InStep, int32 X, int32 Y, int32 Z) const
{
	const FIntVector Position = Local2DToGlobal<Direction>(RENDER_CHUNK_SIZE * Step - InStep, X, Y, Z);
	return Accelerator->GetValue(ChunkPosition + Position, LOD);
}

template<EVoxelDirectionFlag::Type Direction>
FORCEINLINE FVoxelMaterial FVoxelCubicTransitionsMesher::GetMaterial(int32 InStep, int32 X, int32 Y, int32 Z) const
{
	const FIntVector Position = Local2DToGlobal<Direction>(RENDER_CHUNK_SIZE * Step - InStep, X, Y, Z);
	return Accelerator->GetMaterial(ChunkPosition + Position, LOD);
}

template<EVoxelDirectionFlag::Type Direction>
inline bool IsDirectionMax()
{
	return Direction == EVoxelDirectionFlag::XMax || Direction == EVoxelDirectionFlag::YMax || Direction == EVoxelDirectionFlag::ZMax;
}

template<EVoxelDirectionFlag::Type Direction, EVoxelDirectionFlag::Type FaceDirection, typename TVertex>
void FVoxelCubicTransitionsMesher::Add2DFace(
	int32 InStep, 
	const FVoxelMaterial& Material, 
	int32 LX, int32 LY, 
	TArray<TVertex>& Vertices, TArray<uint32>& Indices)
{
	const int32 LZ = IsDirectionMax<FaceDirection>()
		? IsDirectionMax<Direction>() ? 1 : -1
		: 0;

	const FIntVector P = Local2DToGlobal<Direction>(Step / InStep * RENDER_CHUNK_SIZE, LX, LY, LZ);
	AddFace<FaceDirection>(*this, InStep, Material, P.X, P.Y, P.Z, Indices, Vertices);
}

template<EVoxelDirectionFlag::Type Direction>
FORCEINLINE FIntVector FVoxelCubicTransitionsMesher::Local2DToGlobal(int32 InSize, int32 LX, int32 LY, int32 LZ)
{
	const int32& S = InSize;
	switch (Direction)
	{
	case EVoxelDirectionFlag::XMin:
		return { LZ, LX, LY };
	case EVoxelDirectionFlag::XMax:
		return { S - LZ, LY, LX };
	case EVoxelDirectionFlag::YMin:
		return { LY, LZ, LX };
	case EVoxelDirectionFlag::YMax:
		return { LX, S - LZ, LY };
	case EVoxelDirectionFlag::ZMin:
		return { LX, LY, LZ };
	case EVoxelDirectionFlag::ZMax:
		return { LY, LX, S - LZ };
	default:
	check(false);
		return {};
	}
}