// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/GCObject.h"
#include "Materials/MaterialInterface.h"
#include "VoxelGlobals.h"
#include "VoxelRender/VoxelProcMeshTangent.h"

DECLARE_MEMORY_STAT(TEXT("Voxel Intermediate Chunks Memory"), STAT_VoxelIntermediateChunksMemory, STATGROUP_VoxelMemory);

class UMaterialInterface;
class UTexture;
class AVoxelWorld;

struct FVoxelBlendedMaterial
{
	enum EKind
	{
		Single,
		Double,
		Triple,
		Invalid
	};

	FVoxelBlendedMaterial() = default;

	FVoxelBlendedMaterial(uint8 Index)
		: Index0(Index)
		, Kind(Single)
	{
	}

	FVoxelBlendedMaterial(uint8 Index0, uint8 Index1)
		: Index0(Index0)
		, Index1(Index1)
		, Kind(Double)
	{
		check(Index0 < Index1);
	}

	FVoxelBlendedMaterial(uint8 Index0, uint8 Index1, uint8 Index2)
		: Index0(Index0)
		, Index1(Index1)
		, Index2(Index2)
		, Kind(Triple)
	{
		check(Index0 < Index1 && Index1 < Index2);
	}

	inline bool operator==(const FVoxelBlendedMaterial& Other) const
	{
		return Kind == Other.Kind && Index0 == Other.Index0 && Index1 == Other.Index1 && Index2 == Other.Index2;
	}

	inline FString ToString() const
	{
		switch (Kind)
		{
		case Single:
			return FString::Printf(TEXT("Single %d"), Index0);
		case Double:
			return FString::Printf(TEXT("Double %d %d"), Index0, Index1);
		case Triple:
			return FString::Printf(TEXT("Triple %d %d %d"), Index0, Index1, Index2);
		case Invalid:
		default:
			return "Invalid";
		}
	}

	inline FString KindToString() const
	{
		switch (Kind)
		{
		case Single:
			return "Single";
		case Double:
			return "Double";
		case Triple:
			return "Triple";
		case Invalid:
		default:
			return "Invalid";
		}
	}

	inline TArray<uint8, TFixedAllocator<3>> GetElements() const
	{
		switch (Kind)
		{
		case Single:
			return { Index0 };
		case Double:
			return { Index0, Index1 };
		case Triple:
			return { Index0, Index1, Index2 };
		case Invalid:
		default:
			return {};
		}
	}

	uint8 Index0 = 255;
	uint8 Index1 = 255;
	uint8 Index2 = 255;
	EKind Kind = Invalid;
};

inline uint32 GetTypeHash(const FVoxelBlendedMaterial& O)
{
	return GetTypeHash(O.Index0) ^ GetTypeHash(O.Index1) ^ GetTypeHash(O.Index2) ^ GetTypeHash(O.Kind);
}

struct FVoxelMeshProcessingParameters
{
	bool bBuildAdjacency;
	bool bOptimizeIndices;

	FVoxelMeshProcessingParameters() = default;
	FVoxelMeshProcessingParameters(bool bBuildAdjacency, bool bOptimizeIndices)
		: bBuildAdjacency(bBuildAdjacency)
		, bOptimizeIndices(bOptimizeIndices)
	{
	}
};

struct FVoxelProcMeshVertex
{
	FVector Position = FVector::ZeroVector;
	FVector Normal = FVector::UpVector;
	FVoxelProcMeshTangent Tangent = FVoxelProcMeshTangent(FVector::RightVector, false);
	FColor Color = FColor::White;
	FVector2D TextureCoordinate = FVector2D::ZeroVector;

	FVoxelProcMeshVertex() = default;

	FVoxelProcMeshVertex(const FVector& Position, const FVector& Normal, const FVoxelProcMeshTangent& Tangent, const FColor& Color, const FVector2D& TextureCoordinate)
		: Position(Position)
		, Normal(Normal)
		, Tangent(Tangent)
		, Color(Color)
		, TextureCoordinate(TextureCoordinate)
	{
	}
};

struct VOXEL_API FVoxelChunkBuffers
{
	TArray<uint32> Indices;
	TArray<FVector> Positions;
	TArray<FVector> Normals;
	TArray<FVoxelProcMeshTangent> Tangents;
	TArray<FColor> Colors;
	TArray<FVector2D> TextureCoordinates;
	TArray<uint32> AdjacencyIndices;

	~FVoxelChunkBuffers()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelIntermediateChunksMemory, LastAllocatedSize);
	}

	template<typename TAllocator>
	inline void SetIndices(TArray<uint32, TAllocator>&& InIndices)
	{
		Indices = MoveTemp(InIndices);
	}

	template<typename T, typename TAllocator>
	void LoadVerticesFromArray(const TArray<T, TAllocator>& Vertices)
	{
		Positions.Reserve(Vertices.Num());
		Normals.Reserve(Vertices.Num());
		Tangents.Reserve(Vertices.Num());
		Colors.Reserve(Vertices.Num());
		TextureCoordinates.Reserve(Vertices.Num());

		for (auto& Vertex : Vertices)
		{
			AddVertex(Vertex.GetProcMeshVertex());
		}

		UpdateStat();
	}

	inline uint32 AddVertex(const FVoxelProcMeshVertex& Vertex)
	{
		int32 Index = Positions.Emplace(Vertex.Position);
		Normals.Emplace(Vertex.Normal);
		Tangents.Emplace(Vertex.Tangent);
		Colors.Emplace(Vertex.Color);
		TextureCoordinates.Emplace(Vertex.TextureCoordinate);
		return Index;
	}
	
	inline void AddIndex(uint32 Index)
	{
		Indices.Add(Index);
	}

	inline FVoxelProcMeshVertex GetVertex(int32 Index) const
	{
		return FVoxelProcMeshVertex(Positions[Index], Normals[Index], Tangents[Index], Colors[Index], TextureCoordinates[Index]);
	}

	inline void Shrink()
	{
		Positions.Shrink();
		Normals.Shrink();
		Tangents.Shrink();
		Colors.Shrink();
		TextureCoordinates.Shrink();
		AdjacencyIndices.Shrink();

		UpdateStat();
	}

	inline int32 GetNumVertices() const
	{
		return Positions.Num();
	}

	inline int32 GetAllocatedSize() const
	{
		return Indices.GetAllocatedSize()
			+ Positions.GetAllocatedSize()
			+ Normals.GetAllocatedSize()
			+ Tangents.GetAllocatedSize()
			+ Colors.GetAllocatedSize()
			+ TextureCoordinates.GetAllocatedSize()
			+ AdjacencyIndices.GetAllocatedSize();
	}

	inline bool HasAdjacency() const
	{
		return bHasAdjacency;
	}

	void BuildAdjacency();
	void OptimizeIndices();

private:
	bool bHasAdjacency = false;
	int32 LastAllocatedSize = 0;

	void UpdateStat()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelIntermediateChunksMemory, LastAllocatedSize);
		LastAllocatedSize = GetAllocatedSize();
		INC_DWORD_STAT_BY(STAT_VoxelIntermediateChunksMemory, LastAllocatedSize);

		check(
			Positions.Num() == Normals.Num() &&
			Positions.Num() == Tangents.Num() &&
			Positions.Num() == Colors.Num() &&
			Positions.Num() == TextureCoordinates.Num()
		);
	}
};

struct FVoxelChunk
{
	bool bSingleBuffers;
	TMap<FVoxelBlendedMaterial, FVoxelChunkBuffers> Map;
	FVoxelChunkBuffers SingleBuffers;

	inline bool IsEmpty() const { return bSingleBuffers ? SingleBuffers.Indices.Num() == 0 : Map.Num() == 0; }
	template<typename T>
	inline void IterateOnTriangles(T F) const
	{
		if (bSingleBuffers)
		{
			for (int32 I = 0; I < SingleBuffers.Indices.Num(); I += 3)
			{
				F(SingleBuffers.GetVertex(SingleBuffers.Indices[I]), 
				  SingleBuffers.GetVertex(SingleBuffers.Indices[I + 1]), 
				  SingleBuffers.GetVertex(SingleBuffers.Indices[I + 2]));
			}
		}
		else
		{
			for (auto& BufferIt : Map)
			{
				auto& Buffer = BufferIt.Value;
				for (int32 I = 0; I < Buffer.Indices.Num(); I += 3)
				{
					F(Buffer.GetVertex(Buffer.Indices[I]),
					  Buffer.GetVertex(Buffer.Indices[I + 1]),
					  Buffer.GetVertex(Buffer.Indices[I + 2]));
				}
			}
		}
	}
	template<typename T>
	inline void IterateOnBuffers(T F)
	{
		if (bSingleBuffers)
		{
			F(SingleBuffers);
		}
		else
		{
			for (auto& BufferIt : Map)
			{
				F(BufferIt.Value);
			}
		}
	}
	template<typename T>
	inline void IterateOnBuffers(T F) const
	{
		if (bSingleBuffers)
		{
			F(SingleBuffers);
		}
		else
		{
			for (auto& BufferIt : Map)
			{
				F(BufferIt.Value);
			}
		}
	}
};

struct FVoxelChunkMaterials : public FGCObject
{
public:
	FVoxelChunkMaterials() = default;

	inline UMaterialInterface* GetSingleMaterial() const
	{
		return SingleMaterial;
	}
	inline UMaterialInterface* GetMultipleMaterial(const FVoxelBlendedMaterial& Key) const
	{
		auto* Result = Materials.Find(Key);
		return Result ? *Result : nullptr;
	}

	inline void SetSingleMaterial(UMaterialInterface* NewSingleMaterial)
	{
		SingleMaterial = NewSingleMaterial;
	}
	inline void SetMultipleMaterial(const FVoxelBlendedMaterial& Key, UMaterialInterface* NewMultipleMaterial)
	{
		Materials.Add(Key, NewMultipleMaterial);
	}

	inline void Reset()
	{
		SingleMaterial = nullptr;
		Materials.Empty();
	}

protected:
	virtual void AddReferencedObjects(FReferenceCollector& Collector) final
	{
		Collector.AddReferencedObject(SingleMaterial);
		Collector.AddReferencedObjects(Materials);
	}

private:
	UMaterialInterface* SingleMaterial = nullptr;
	TMap<FVoxelBlendedMaterial, UMaterialInterface*> Materials;
};