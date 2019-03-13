// Copyright 2019 Phyronnaz

#include "VoxelRender/Polygonizers/VoxelCubicPolygonizer.h"
#include "VoxelData/VoxelData.h"
#include "VoxelMaterial.h"
#include "VoxelDebug/VoxelStats.h"
#include "VoxelRender/VoxelRenderUtilities.h"


inline FVector2D ExtractUVs(const FVector& V, EVoxelDirection Direction)
{
	switch (Direction)
	{
	case XMin:
		return FVector2D(V.Y, -V.Z);
	case XMax:
		return FVector2D(-V.Y, -V.Z);
	case YMin:
		return FVector2D(-V.X, -V.Z);
	case YMax:
		return FVector2D(V.X, -V.Z);
	case ZMin:
		return FVector2D(V.X, -V.Y);
	default:
		check(Direction == ZMax);
		return FVector2D(-V.X, -V.Y);
	}
}

template<typename TPolygonizer>
inline void AddFace(TPolygonizer& This, int Step, const FVoxelMaterial& Material, int X, int Y, int Z, EVoxelDirection Direction, TArray<uint32>& Indices, TArray<FCubicLocalVoxelVertex>& Vertices)
{
	FVector AP, BP, CP, DP;
	FVector Normal;
	FVector Tangent;
	bool bReversed;

	switch (Direction)
	{
	case XMin:
		AP = FVector(0, 0, 1);
		BP = FVector(0, 1, 1);
		CP = FVector(0, 1, 0);
		DP = FVector(0, 0, 0);
		Normal = FVector(-1, 0, 0);
		Tangent = FVector(0, -1, 0);
		bReversed = false;
		break;
	case XMax:
		AP = FVector(1, 0, 1);
		BP = FVector(1, 1, 1);
		CP = FVector(1, 1, 0);
		DP = FVector(1, 0, 0);
		Normal = FVector(1, 0, 0);
		Tangent = FVector(0, 1, 0);
		bReversed = true;
		break;
	case YMin:
		AP = FVector(0, 0, 1);
		BP = FVector(1, 0, 1);
		CP = FVector(1, 0, 0);
		DP = FVector(0, 0, 0);
		Normal = FVector(0, -1, 0);
		Tangent = FVector(-1, 0, 0);
		bReversed = true;
		break;
	case YMax:
		AP = FVector(0, 1, 1);
		BP = FVector(1, 1, 1);
		CP = FVector(1, 1, 0);
		DP = FVector(0, 1, 0);
		Normal = FVector(0, 1, 0);
		Tangent = FVector(1, 0, 0);
		bReversed = false;
		break;
	case ZMin:
		AP = FVector(0, 1, 0);
		BP = FVector(1, 1, 0);
		CP = FVector(1, 0, 0);
		DP = FVector(0, 0, 0);
		Normal = FVector(0, 0, -1);
		Tangent = FVector(-1, 0, 0);
		bReversed = false;
		break;
	default:
		check(Direction == ZMax);
		AP = FVector(0, 1, 1);
		BP = FVector(1, 1, 1);
		CP = FVector(1, 0, 1);
		DP = FVector(0, 0, 1);
		Normal = FVector(0, 0, 1);
		Tangent = FVector(1, 0, 0);
		bReversed = true;
		break;
	}
	
	FVector P(X, Y, Z);

	FCubicLocalVoxelVertex Vertex;
	Vertex.Normal = Normal;
	Vertex.Tangent = FVoxelProcMeshTangent(Tangent, false);
	Vertex.Material = Material;
	if (This.UVConfig == EVoxelUVConfig::PackWorldUpInUVs)
	{
		FVector WorldUp = This.Data->WorldGenerator->GetUpVector(This.ChunkPosition + FIntVector(X, Y, Z)).GetSafeNormal();
		Vertex.UVs = FVector2D(WorldUp.X, WorldUp.Y);
	}
	else if (This.UVConfig == EVoxelUVConfig::UseRGAsUVs)
	{
		Vertex.UVs = FVector2D(Material.GetR(), Material.GetG());
	}
	else if (This.UVConfig == EVoxelUVConfig::UseGrassAndActorIdsAsUVs)
	{
		Vertex.UVs = FVector2D(Material.GetVoxelGrassId(), Material.GetVoxelActorId());
	}

	int A = Vertices.Num();
	{
		FCubicLocalVoxelVertex AV = Vertex;
		AV.Position = (AP + P) * Step;
		if (This.UVConfig == EVoxelUVConfig::GlobalUVs)
		{
			AV.UVs = ExtractUVs(AV.Position + FVector(This.ChunkPosition), Direction);
		}
		else if(This.UVConfig == EVoxelUVConfig::PerVoxelUVs)
		{			
			AV.UVs = FVector2D(bReversed ? 1 : 0, 0);
		}
		Vertices.Add(AV);
	}

	int B = Vertices.Num();
	{
		FCubicLocalVoxelVertex BV = Vertex;
		BV.Position = (BP + P) * Step;
		if (This.UVConfig == EVoxelUVConfig::GlobalUVs)
		{
			BV.UVs = ExtractUVs(BV.Position + FVector(This.ChunkPosition), Direction);
		}
		else if(This.UVConfig == EVoxelUVConfig::PerVoxelUVs)
		{
			BV.UVs = FVector2D(bReversed ? 0 : 1, 0);
		}
		Vertices.Add(BV);
	}

	int C = Vertices.Num();
	{
		FCubicLocalVoxelVertex CV = Vertex;
		CV.Position = (CP + P) * Step;
		if (This.UVConfig == EVoxelUVConfig::GlobalUVs)
		{
			CV.UVs = ExtractUVs(CV.Position + FVector(This.ChunkPosition), Direction);
		}
		else if(This.UVConfig == EVoxelUVConfig::PerVoxelUVs)
		{			
			CV.UVs = FVector2D(bReversed ? 0 : 1, 1);
		}
		Vertices.Add(CV);
	}

	int D = Vertices.Num();
	{
		FCubicLocalVoxelVertex DV = Vertex;
		DV.Position = (DP + P) * Step;
		if (This.UVConfig == EVoxelUVConfig::GlobalUVs)
		{
			DV.UVs = ExtractUVs(DV.Position + FVector(This.ChunkPosition), Direction);
		}
		else if(This.UVConfig == EVoxelUVConfig::PerVoxelUVs)
		{		
			DV.UVs = FVector2D(bReversed ? 1 : 0, 1);
		}
		Vertices.Add(DV);
	}	
	
	if (!bReversed)
	{
		Indices.Add(D);
		Indices.Add(B);
		Indices.Add(A);
		
		Indices.Add(D);
		Indices.Add(C);
		Indices.Add(B);
	}
	else
	{
		Indices.Add(A);
		Indices.Add(B);
		Indices.Add(D);

		Indices.Add(B);
		Indices.Add(C);
		Indices.Add(D);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const FIntVector CubicChunkDataSize = FIntVector(CUBIC_EXTENDED_CHUNK_SIZE);

FIntBox FVoxelCubicPolygonizer::GetValuesBounds() const
{
	return FIntBox(ChunkPosition - FIntVector(Step), ChunkPosition - FIntVector(Step) + CubicChunkDataSize * Step);
}

FIntBox FVoxelCubicPolygonizer::GetLockedBounds() const
{
	return GetValuesBounds();
}

bool FVoxelCubicPolygonizer::CreateChunk()
{		   
	Stats.StartStat("GetValuesAndMaterials");
	Data->GetValuesAndMaterials(CachedValues, CachedMaterials, FVoxelWorldGeneratorQueryZone(GetValuesBounds(), CubicChunkDataSize, LOD), LOD);
	
	Stats.StartStat("UnlockRead");
	Data->Unlock<EVoxelLockType::Read>(Octrees);
	
	Stats.StartStat("Iteration");

	TArray<FCubicLocalVoxelVertex> Vertices;
	TArray<uint32> Indices;

	for (int X = 0; X < CHUNK_SIZE; X++)
	{
		for (int Y = 0; Y < CHUNK_SIZE; Y++)
		{
			for (int Z = 0; Z < CHUNK_SIZE; Z++)
			{
				const FVoxelValue Value = GetValue(X, Y, Z);

				if (!Value.IsEmpty())
				{
					const auto& Material = GetMaterial(X, Y, Z);
					if (GetValue(X - 1, Y, Z).IsEmpty())
					{
						AddFace(*this, Step, Material, X, Y, Z, XMin, Indices, Vertices);
					}
					if (GetValue(X + 1, Y, Z).IsEmpty())
					{
						AddFace(*this, Step, Material, X, Y, Z, XMax, Indices, Vertices);
					}
					if (GetValue(X, Y - 1, Z).IsEmpty())
					{
						AddFace(*this, Step, Material, X, Y, Z, YMin, Indices, Vertices);
					}
					if (GetValue(X, Y + 1, Z).IsEmpty())
					{
						AddFace(*this, Step, Material, X, Y, Z, YMax, Indices, Vertices);
					}
					if (GetValue(X, Y, Z - 1).IsEmpty())
					{
						AddFace(*this, Step, Material, X, Y, Z, ZMin, Indices, Vertices);
					}
					if (GetValue(X, Y, Z + 1).IsEmpty())
					{
						AddFace(*this, Step, Material, X, Y, Z, ZMax, Indices, Vertices);
					}
				}
			}
		}
	}
	
	Stats.StartStat("ConvertArrays");
	FVoxelRenderUtilities::ConvertArrays(LOD, MaterialConfig, MeshParameters, MoveTemp(Indices), MoveTemp(Vertices), Chunk, Stats);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntBox FVoxelCubicTransitionsPolygonizer::GetBounds() const
{
	return FIntBox(ChunkPosition - FIntVector(Step), ChunkPosition - FIntVector(Step) + CubicChunkDataSize * Step);
}

bool FVoxelCubicTransitionsPolygonizer::CreateTransitions()
{
	Stats.StartStat("GetMap");
	MapAccelerator = MakeUnique<FVoxelDataUtilities::MapAccelerator>(GetBounds(), Data);
	
	Stats.StartStat("Iteration");

	TArray<FCubicLocalVoxelVertex> Vertices;
	TArray<uint32> Indices;

	int DirectionIndex = -1;
	for (auto& Direction : { XMin, XMax, YMin, YMax, ZMin, ZMax })
	{
		DirectionIndex++;

		if (TransitionsMask & Direction)
		{
			for (int LX = 0; LX < CHUNK_SIZE; LX++)
			{
				for (int LY = 0; LY < CHUNK_SIZE; LY++)
				{
					bool bBigIsFull = !GetValue(Step, Direction, LX * Step, LY * Step, 0).IsEmpty();

					if (bBigIsFull)
					{
						bool bBigIsFullOtherSide = !GetValue(Step, Direction, LX * Step, LY * Step, -Step).IsEmpty();
						if (bBigIsFullOtherSide) // Else face is already created
						{
							FVoxelValue SmallValues[4];

							SmallValues[0] = GetValue(HalfStep, Direction, (2 * LX + 0) * HalfStep, (2 * LY + 0) * HalfStep, -HalfStep);
							SmallValues[1] = GetValue(HalfStep, Direction, (2 * LX + 1) * HalfStep, (2 * LY + 0) * HalfStep, -HalfStep);
							SmallValues[2] = GetValue(HalfStep, Direction, (2 * LX + 0) * HalfStep, (2 * LY + 1) * HalfStep, -HalfStep);
							SmallValues[3] = GetValue(HalfStep, Direction, (2 * LX + 1) * HalfStep, (2 * LY + 1) * HalfStep, -HalfStep);

							if (SmallValues[0].IsEmpty() ||
								SmallValues[1].IsEmpty() || 
								SmallValues[2].IsEmpty() ||
								SmallValues[3].IsEmpty())
							{
								auto Material = GetMaterial(Step, Direction, LX * Step, LY * Step, 0);
								Add2DFace(Step, Direction, true, Material, LX, LY, Vertices, Indices);
							}
						}
					}
					else
					{
						FVoxelValue SmallValues[2][4];
						SmallValues[0][0] = GetValue(HalfStep, Direction, (2 * LX + 0) * HalfStep, (2 * LY + 0) * HalfStep, 0);
						SmallValues[0][1] = GetValue(HalfStep, Direction, (2 * LX + 1) * HalfStep, (2 * LY + 0) * HalfStep, 0);
						SmallValues[0][2] = GetValue(HalfStep, Direction, (2 * LX + 0) * HalfStep, (2 * LY + 1) * HalfStep, 0);
						SmallValues[0][3] = GetValue(HalfStep, Direction, (2 * LX + 1) * HalfStep, (2 * LY + 1) * HalfStep, 0);

						SmallValues[1][0] = GetValue(HalfStep, Direction, (2 * LX + 0) * HalfStep, (2 * LY + 0) * HalfStep, -HalfStep);
						SmallValues[1][1] = GetValue(HalfStep, Direction, (2 * LX + 1) * HalfStep, (2 * LY + 0) * HalfStep, -HalfStep);
						SmallValues[1][2] = GetValue(HalfStep, Direction, (2 * LX + 0) * HalfStep, (2 * LY + 1) * HalfStep, -HalfStep);
						SmallValues[1][3] = GetValue(HalfStep, Direction, (2 * LX + 1) * HalfStep, (2 * LY + 1) * HalfStep, -HalfStep);

						uint8 IsFull =
							((!SmallValues[0][0].IsEmpty()) << 0) |
							((!SmallValues[0][1].IsEmpty()) << 1) |
							((!SmallValues[0][2].IsEmpty()) << 2) |
							((!SmallValues[0][3].IsEmpty()) << 3);

						uint8 IsFullOtherSide =
							((!SmallValues[1][0].IsEmpty()) << 0) |
							((!SmallValues[1][1].IsEmpty()) << 1) |
							((!SmallValues[1][2].IsEmpty()) << 2) |
							((!SmallValues[1][3].IsEmpty()) << 3);

						if (IsFullOtherSide & IsFull) // Else all checks below will fail
						{
							auto Material = GetMaterial(Step, Direction, LX * Step, LY * Step, -HalfStep);
							if (IsFullOtherSide & IsFull & 0x1) // Both are full
							{
								Add2DFace(HalfStep, Direction, false, Material, 2 * LX + 0, 2 * LY + 0, Vertices, Indices);
							}
							if (IsFullOtherSide & IsFull & 0x2)
							{
								Add2DFace(HalfStep, Direction, false, Material, 2 * LX + 1, 2 * LY + 0, Vertices, Indices);
							}
							if (IsFullOtherSide & IsFull & 0x4)
							{
								Add2DFace(HalfStep, Direction, false, Material, 2 * LX + 0, 2 * LY + 1, Vertices, Indices);
							}
							if (IsFullOtherSide & IsFull & 0x8)
							{
								Add2DFace(HalfStep, Direction, false, Material, 2 * LX + 1, 2 * LY + 1, Vertices, Indices);
							}
						}
					}
				}
			}
		}
	}
	
	Stats.StartStat("UnlockRead");
	Data->Unlock<EVoxelLockType::Read>(Octrees);
	
	Stats.StartStat("ConvertArrays");
	FVoxelRenderUtilities::ConvertArrays(LOD, MaterialConfig, MeshParameters, MoveTemp(Indices), MoveTemp(Vertices), Chunk, Stats);

	return true;
}

inline bool IsDirectionMax(EVoxelDirection Direction)
{
	return Direction == XMax || Direction == YMax || Direction == ZMax;
}

void FVoxelCubicTransitionsPolygonizer::Add2DFace(int InStep, EVoxelDirection Direction, bool bInvert, const FVoxelMaterial& Material, int LX, int LY, TArray<FCubicLocalVoxelVertex>& Vertices, TArray<uint32>& Indices)
{
	auto FaceDirection = bInvert ? Direction : InverseVoxelDirection(Direction); // AddFace takes reversed directions
	int LZ = 0;
	if (IsDirectionMax(FaceDirection))
	{
		if (IsDirectionMax(Direction))
		{
			LZ = 1;
		}
		else
		{
			LZ = -1;
		}
	}
	int GX, GY, GZ;
	Local2DToGlobal(Step / InStep * CHUNK_SIZE, Direction, LX, LY, LZ, GX, GY, GZ);
	AddFace(*this, InStep, Material, GX, GY, GZ, FaceDirection, Indices, Vertices);
}