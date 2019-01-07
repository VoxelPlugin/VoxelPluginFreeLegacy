// Copyright 2019 Phyronnaz

#include "VoxelRender/Polygonizers/VoxelCubicPolygonizer.h"
#include "VoxelLogStatDefinitions.h"
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
		Vertex.UVs = FVector2D(Vertex.Material.GetR(), Vertex.Material.GetG());
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

FVoxelCubicPolygonizer::FVoxelCubicPolygonizer(
	int LOD, 
	FVoxelData* Data, 
	const FIntVector& ChunkPosition,
	EVoxelMaterialConfig MaterialConfig, 
	EVoxelUVConfig UVConfig,
	bool bCacheLOD0Chunks,
	FVoxelMeshProcessingParameters MeshParameters)
	: LOD(LOD)
	, Step(1 << LOD)
	, Data(Data)
	, ChunkPosition(ChunkPosition)
	, MaterialConfig(MaterialConfig)
	, UVConfig(UVConfig)
	, bCacheLOD0Chunks(bCacheLOD0Chunks)
	, MeshParameters(MeshParameters)
{
}

bool FVoxelCubicPolygonizer::CreateSection(FVoxelChunk& OutChunk, FVoxelStatsElement& Stats)
{	
	FIntVector ChunkDataSize(CUBIC_EXTENDED_CHUNK_SIZE);
	const FIntBox Bounds = FIntBox(ChunkPosition - FIntVector(Step), ChunkPosition - FIntVector(Step) + ChunkDataSize * Step);
	
	if (bCacheLOD0Chunks && LOD == 0)
	{
		TArray<FVoxelId> Octrees;

		Stats.StartStat("BeginSet");
		Data->BeginSet(Bounds, Octrees, FString::Printf(TEXT("MarchingCubesPolygonizer LOD=%d Cache"), LOD));
		
		Stats.StartStat("Cache");
		Data->Cache(Bounds, false);
		
		Stats.StartStat("EndSet");
		Data->EndSet(Octrees);
	}

	TArray<FVoxelId> Octrees;
	Stats.StartStat("BeginGet");
	Data->BeginGet(Bounds, Octrees, FString::Printf(TEXT("CubicPolygonizer LOD=%d"), LOD));
	
	Stats.StartStat("IsEmpty");
	bool bIsEmpty = Data->IsEmpty(Bounds, LOD);
	Stats.SetValue("bIsEmpty", bIsEmpty ? "1" : "0");

	if (bIsEmpty)
	{
		Stats.StartStat("EndGet");
		Data->EndGet(Octrees);

		return true;
	}
	   
	Stats.StartStat("GetValuesAndMaterials");
	Data->GetValuesAndMaterials(CachedValues, CachedMaterials, FVoxelWorldGeneratorQueryZone(Bounds, ChunkDataSize, LOD), LOD);
	
	Stats.StartStat("EndGet");
	Data->EndGet(Octrees);
	
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
	
	Stats.SetValue("Num Vertices", FString::FromInt(Vertices.Num()));
	
	Stats.StartStat("ConvertArrays");
	FVoxelRenderUtilities::ConvertArraysToMap(LOD, MaterialConfig, MeshParameters, MoveTemp(Indices), MoveTemp(Vertices), OutChunk, Stats);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelCubicTransitionsPolygonizer::FVoxelCubicTransitionsPolygonizer(
	int LOD,
	FVoxelData* Data,
	const FIntVector& ChunkPosition,
	uint8 TransitionsMask,
	EVoxelMaterialConfig MaterialConfig,
	EVoxelUVConfig UVConfig,
	FVoxelMeshProcessingParameters MeshParameters)
	: LOD(LOD)
	, Step(1 << LOD)
	, Data(Data)
	, ChunkPosition(ChunkPosition)
	, TransitionsMask(TransitionsMask)
	, MaterialConfig(MaterialConfig)
	, UVConfig(UVConfig)
	, MeshParameters(MeshParameters)
{
}

bool FVoxelCubicTransitionsPolygonizer::CreateTransitions(FVoxelChunk& OutChunk, FVoxelStatsElement& Stats)
{
	if (!TransitionsMask)
	{
		return true;
	}

	const int HalfStep = Step / 2;

	FIntVector ChunkDataSize(CUBIC_EXTENDED_CHUNK_SIZE);
	const FIntBox Bounds = FIntBox(ChunkPosition - FIntVector(Step), ChunkPosition - FIntVector(Step) + ChunkDataSize * Step);

	TArray<FVoxelId> Octrees;
	Stats.StartStat("BeginGet");
	Data->BeginGet(Bounds, Octrees, FString::Printf(TEXT("CubicTransitionsPolygonizer LOD=%d"), LOD));
	
	Stats.StartStat("GetMap");
	MapAccelerator = MakeUnique<FVoxelData::MapAccelerator>(Bounds, Data);
	
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
	
	Stats.StartStat("EndGet");
	Data->EndGet(Octrees);
	
	Stats.SetValue("Num Vertices", FString::FromInt(Vertices.Num()));
	
	Stats.StartStat("ConvertArrays");
	FVoxelRenderUtilities::ConvertArraysToMap(LOD, MaterialConfig, MeshParameters, MoveTemp(Indices), MoveTemp(Vertices), OutChunk, Stats);

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