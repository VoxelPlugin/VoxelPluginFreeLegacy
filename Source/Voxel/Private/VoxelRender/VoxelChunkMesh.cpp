// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelChunkMesh.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "StackArray.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "DistanceFieldAtlas.h"

#if ENABLE_TESSELLATION
#include "ThirdParty/nvtesslib/inc/nvtess.h"
#endif

#if ENABLE_OPTIMIZE_INDICES
#include "ThirdParty/ForsythTriOO/Src/forsythtriangleorderoptimizer.h"
#endif

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelChunkMeshMemory);

#if ENABLE_TESSELLATION
/**
* Provides static mesh render data to the NVIDIA tessellation library.
*/
class FVoxelStaticMeshNvRenderBuffer : public nv::RenderBuffer
{
public:

	/** Construct from static mesh render buffers. */
	FVoxelStaticMeshNvRenderBuffer(
		const TArray<FVector>& InPositionVertexBuffer,
		const TArray<uint32>& Indices)
		: PositionVertexBuffer(InPositionVertexBuffer)
	{
		mIb = new nv::IndexBuffer((void*)Indices.GetData(), nv::IBT_U32, Indices.Num(), false);
	}

	/** Retrieve the position and first texture coordinate of the specified index. */
	virtual nv::Vertex getVertex(const unsigned int Index) const override
	{
		nv::Vertex Vertex;

		const FVector& Position = PositionVertexBuffer[Index];
		Vertex.pos.x = Position.X;
		Vertex.pos.y = Position.Y;
		Vertex.pos.z = Position.Z;

		Vertex.uv.x = 0.0f;
		Vertex.uv.y = 0.0f;

		return Vertex;
	}

private:
	/** The position vertex buffer for the static mesh. */
	const TArray<FVector>& PositionVertexBuffer;

	/** Copying is forbidden. */
	FVoxelStaticMeshNvRenderBuffer(const FVoxelStaticMeshNvRenderBuffer&) = delete;
	FVoxelStaticMeshNvRenderBuffer& operator=(const FVoxelStaticMeshNvRenderBuffer&) = delete;
};
#endif

void FVoxelChunkMeshBuffers::BuildAdjacency(TArray<uint32>& OutAdjacencyIndices) const
{
	VOXEL_FUNCTION_COUNTER();
	
#if ENABLE_TESSELLATION
	if (Indices.Num())
	{
		FVoxelStaticMeshNvRenderBuffer StaticMeshRenderBuffer(Positions, Indices);
		nv::IndexBuffer* PnAENIndexBuffer = nv::tess::buildTessellationBuffer(&StaticMeshRenderBuffer, nv::DBM_PnAenDominantCorner, true);
		check(PnAENIndexBuffer);
		const int32 IndexCount = int32(PnAENIndexBuffer->getLength());
		OutAdjacencyIndices.Empty(IndexCount);
		OutAdjacencyIndices.AddUninitialized(IndexCount);
		for (int32 Index = 0; Index < IndexCount; ++Index)
		{
			OutAdjacencyIndices[Index] = (*PnAENIndexBuffer)[Index];
		}
		delete PnAENIndexBuffer;
	}
	else
	{
		OutAdjacencyIndices.Empty();
	}
#endif
}

void FVoxelChunkMeshBuffers::OptimizeIndices()
{
#if ENABLE_OPTIMIZE_INDICES
	VOXEL_FUNCTION_COUNTER();
	
	TArray<uint32> OptimizedIndices;
	OptimizedIndices.AddUninitialized(Indices.Num());
	const uint16 CacheSize = 32;
	Forsyth::OptimizeFaces(Indices.GetData(), Indices.Num(), GetNumVertices(), OptimizedIndices.GetData(), CacheSize);
	Indices = MoveTemp(OptimizedIndices);
#endif
}

void FVoxelChunkMeshBuffers::Shrink()
{
	Positions.Shrink();
	Normals.Shrink();
	Tangents.Shrink();
	Colors.Shrink();
	for (auto& T : TextureCoordinates) T.Shrink();

	UpdateStats();
}

void FVoxelChunkMeshBuffers::ComputeBounds()
{
	Bounds = FBox(ForceInit);
	for (auto& Vertex : Positions)
	{
		Bounds += Vertex;
	}
}

void FVoxelChunkMeshBuffers::UpdateStats()
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelChunkMeshMemory, LastAllocatedSize);
	LastAllocatedSize = Indices.GetAllocatedSize();
	LastAllocatedSize += Positions.GetAllocatedSize();
	LastAllocatedSize += Normals.GetAllocatedSize();
	LastAllocatedSize += Tangents.GetAllocatedSize();
	LastAllocatedSize += Colors.GetAllocatedSize();
	for (auto& T : TextureCoordinates) LastAllocatedSize += T.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelChunkMeshMemory, LastAllocatedSize);
}

void FVoxelChunkMesh::BuildDistanceField(int32 LOD, const FIntVector & Position, const FVoxelData & Data)
{
#if ENABLE_VOXEL_DISTANCE_FIELDS
	VOXEL_FUNCTION_COUNTER();
	
	if (IsEmpty())
	{
		return;
	}

	// One voxel thick layer around the chunk where the distance is > 0 to have a valid distance field
	// Will be fine in the global one has nearby chunks will have a < 0 distance at these positions
	
	constexpr int32 BorderSize = 1;
	constexpr int32 Size = RENDER_CHUNK_SIZE + 1 + 2 * BorderSize;
	constexpr int32 NumVoxels = Size * Size * Size;

	const int32 Step = 1 << LOD;

	TStackArray<FVoxelValue, NumVoxels> Values;
	{
		const FIntBox LockedBounds(Position - BorderSize * Step, Position + (Size - BorderSize) * Step);
		FVoxelReadScopeLock Lock(Data, LockedBounds, "Distance Field Build");
		TVoxelQueryZone<FVoxelValue> QueryZone(LockedBounds, FIntVector(Size), LOD, Values);
		Data.Get<FVoxelValue>(QueryZone, LOD);
	}

	TStackArray<float, NumVoxels> DistanceFieldVolume;
	for (int32 Z = 0; Z < Size; Z++)
	{
		for (int32 Y = 0; Y < Size; Y++)
		{
			for (int32 X = 0; X < Size; X++)
			{
				const uint32 Index = X + Size * Y + Size * Size * Z;
				float Value = Values[Index].ToFloat();
				if (X == 0 || X == Size - 1 ||
					Y == 0 || Y == Size - 1 ||
					Z == 0 || Z == Size - 1)
				{
					Value = FMath::Max(1.f, Value);
				}
				DistanceFieldVolume[Index] = Value * Step;
			}
		}
	}
	
	const float MinVolumeDistance = -1.f;
	const float MaxVolumeDistance = 1.f;
	const float InvDistanceRange = 1.0f / (MaxVolumeDistance - MinVolumeDistance);

	static const auto CVarEightBit = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DistanceFieldBuild.EightBit"));

	const bool bEightBitFixedPoint = CVarEightBit->GetValueOnAnyThread() != 0;
	const int32 FormatSize = bEightBitFixedPoint ? sizeof(uint8) : sizeof(FFloat16);
	constexpr int32 MaxFormatSize = FMath::Max(sizeof(uint8), sizeof(FFloat16));

	TStackArray<uint8, MaxFormatSize * NumVoxels> QuantizedDistanceFieldVolume;
	for (int32 Z = 0; Z < Size; Z++)
	{
		for (int32 Y = 0; Y < Size; Y++)
		{
			for (int32 X = 0; X < Size; X++)
			{
				const uint32 Index = X + Size * Y + Size * Size * Z;
				const float Distance = DistanceFieldVolume[Index];
				if (bEightBitFixedPoint)
				{
					check(FormatSize == sizeof(uint8));
					// [MinVolumeDistance, MaxVolumeDistance] -> [0, 1]
					const float RescaledDistance = (Distance - MinVolumeDistance) * InvDistanceRange;
					// Encoding based on D3D format conversion rules for float -> UNORM
					const int32 QuantizedDistance = FMath::FloorToInt(RescaledDistance * 255.0f + .5f);
					QuantizedDistanceFieldVolume[Index * FormatSize] = FMath::Clamp<int32>(QuantizedDistance, 0, 255);
				}
				else
				{
					check(FormatSize == sizeof(FFloat16));
					reinterpret_cast<FFloat16&>(QuantizedDistanceFieldVolume[Index * FormatSize]) = Distance;
				}
			}
		}
	}

	check(!DistanceFieldVolumeData.IsValid());
	DistanceFieldVolumeData = MakeVoxelShared<FDistanceFieldVolumeData>();
	DistanceFieldVolumeData->bMeshWasClosed = true; // Not used
	DistanceFieldVolumeData->bBuiltAsIfTwoSided = true;
	DistanceFieldVolumeData->bMeshWasPlane = false; // Maybe check this?
	DistanceFieldVolumeData->Size = FIntVector(Size);
	DistanceFieldVolumeData->LocalBoundingBox = FBox(FVector(-BorderSize - 0.5f) * Step, FVector(-BorderSize - 0.5f + Size) * Step);
	DistanceFieldVolumeData->DistanceMinMax = FVector2D(MinVolumeDistance, MaxVolumeDistance);

	auto& CompressedDistanceFieldVolume = DistanceFieldVolumeData->CompressedDistanceFieldVolume;
	static const auto CVarCompress = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DistanceFieldBuild.Compress"));
	const bool bCompress = CVarCompress->GetValueOnAnyThread() != 0;

	if (bCompress)
	{
		const int32 UncompressedSize = QuantizedDistanceFieldVolume.Num() * QuantizedDistanceFieldVolume.GetTypeSize();

		// Compressed can be slightly larger than uncompressed
		CompressedDistanceFieldVolume.Empty(UncompressedSize * 4 / 3);
		CompressedDistanceFieldVolume.AddUninitialized(UncompressedSize * 4 / 3);
		int32 CompressedSize = CompressedDistanceFieldVolume.Num() * CompressedDistanceFieldVolume.GetTypeSize();

		verify(FCompression::CompressMemory(
			NAME_Zlib,
			CompressedDistanceFieldVolume.GetData(),
			CompressedSize,
			QuantizedDistanceFieldVolume.GetData(),
			UncompressedSize));

		CompressedDistanceFieldVolume.SetNum(CompressedSize);
		CompressedDistanceFieldVolume.Shrink();
	}
	else
	{
		CompressedDistanceFieldVolume = QuantizedDistanceFieldVolume;
	}
#endif
}
