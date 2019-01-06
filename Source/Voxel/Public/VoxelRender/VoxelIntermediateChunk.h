// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelMaterial.h"
#include "VoxelGlobals.h"
#include "VoxelDirection.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "VoxelLogStatDefinitions.h"
#include "VoxelIntermediateChunk.generated.h"

DECLARE_MEMORY_STAT(TEXT("Voxel Intermediate Chunks Memory"), STAT_VoxelIntermediateChunksMemory, STATGROUP_VoxelMemory);

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
		FString Result;
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
	uint8 GrassId = 0;
	uint8 ActorId = 0;

	FVoxelProcMeshVertex() = default;

	FVoxelProcMeshVertex(const FVector& Position, const FVector& Normal, const FVoxelProcMeshTangent& Tangent, const FColor& Color, const FVector2D& TextureCoordinate, uint8 GrassId, uint8 ActorId)
		: Position(Position)
		, Normal(Normal)
		, Tangent(Tangent)
		, Color(Color)
		, TextureCoordinate(TextureCoordinate)
		, GrassId(GrassId)
		, ActorId(ActorId)
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
	TArray<uint8> GrassIds;
	TArray<uint8> ActorIds;
	TArray<uint32> AdjacencyIndices;

	~FVoxelChunkBuffers()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelIntermediateChunksMemory, LastAllocatedSize);
	}

	inline void SetIndices(TArray<uint32>&& InIndices)
	{
		Indices = MoveTemp(InIndices);
	}

	template<typename T>
	void LoadVerticesFromArray(const TArray<T>& Vertices)
	{
		Positions.Reserve(Vertices.Num());
		Normals.Reserve(Vertices.Num());
		Tangents.Reserve(Vertices.Num());
		Colors.Reserve(Vertices.Num());
		TextureCoordinates.Reserve(Vertices.Num());
		GrassIds.Reserve(Vertices.Num());
		ActorIds.Reserve(Vertices.Num());

		for (auto& Vertex : Vertices)
		{
			AddVertex(Vertex.GetProcMeshVertex());
		}

		UpdateStat();
	}

	inline uint32 AddVertex(const FVoxelProcMeshVertex& Vertex)
	{
		int Index = Positions.Emplace(Vertex.Position);
		Normals.Emplace(Vertex.Normal);
		Tangents.Emplace(Vertex.Tangent);
		Colors.Emplace(Vertex.Color);
		TextureCoordinates.Emplace(Vertex.TextureCoordinate);
		GrassIds.Emplace(Vertex.GrassId);
		ActorIds.Emplace(Vertex.ActorId);
		return Index;
	}
	
	inline void AddIndex(uint32 Index)
	{
		Indices.Add(Index);
	}

	inline FVoxelProcMeshVertex GetVertex(int Index) const
	{
		return FVoxelProcMeshVertex(Positions[Index], Normals[Index], Tangents[Index], Colors[Index], TextureCoordinates[Index], GrassIds[Index], ActorIds[Index]);
	}

	inline void Shrink()
	{
		Positions.Shrink();
		Normals.Shrink();
		Tangents.Shrink();
		Colors.Shrink();
		TextureCoordinates.Shrink();
		GrassIds.Shrink();
		ActorIds.Shrink();
		AdjacencyIndices.Shrink();

		UpdateStat();
	}

	inline int GetNumVertices() const
	{
		return Positions.Num();
	}

	inline int GetAllocatedSize() const
	{
		return Indices.GetAllocatedSize()
			+ Positions.GetAllocatedSize()
			+ Normals.GetAllocatedSize()
			+ Tangents.GetAllocatedSize()
			+ Colors.GetAllocatedSize()
			+ TextureCoordinates.GetAllocatedSize()
			+ GrassIds.GetAllocatedSize()
			+ ActorIds.GetAllocatedSize()
			+ AdjacencyIndices.GetAllocatedSize();
	}

	void BuildAdjacency();
	void OptimizeIndices();

private:

	int LastAllocatedSize = 0;
	void UpdateStat()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelIntermediateChunksMemory, LastAllocatedSize);
		LastAllocatedSize = GetAllocatedSize();
		INC_DWORD_STAT_BY(STAT_VoxelIntermediateChunksMemory, LastAllocatedSize);

		check(Positions.Num() == Normals.Num() &&
			Positions.Num() == Tangents.Num() &&
			Positions.Num() == Colors.Num() &&
			Positions.Num() == TextureCoordinates.Num() &&
			Positions.Num() == GrassIds.Num() &&
			Positions.Num() == ActorIds.Num()
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
			for (int I = 0; I < SingleBuffers.Indices.Num(); I += 3)
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
				for (int I = 0; I < Buffer.Indices.Num(); I += 3)
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
};

USTRUCT()
struct FVoxelMaterialsRefHolder
{
	GENERATED_BODY()

	UPROPERTY()
	TSet<UMaterialInterface*> UsedMaterials;
};

struct FVoxelChunkMaterials
{
public:
	FVoxelChunkMaterials(FVoxelMaterialsRefHolder* MaterialsRef) : MaterialsRef(MaterialsRef) {}

	inline UMaterialInstanceDynamic* GetSingleMaterial() const { return SingleMaterial; }
	inline UMaterialInstanceDynamic* GetMultipleMaterial(const FVoxelBlendedMaterial& Key) const { UMaterialInstanceDynamic* const * Result = Materials.Find(Key); return Result ? *Result : nullptr; }
	
	void SetSingleMaterial(UMaterialInstanceDynamic* NewSingleMaterial)
	{
		if (SingleMaterial)
		{
			MaterialsRef->UsedMaterials.Remove(SingleMaterial);
		}
		SingleMaterial = NewSingleMaterial;
		if (SingleMaterial)
		{
			MaterialsRef->UsedMaterials.Add(SingleMaterial);
		}
	}

	void SetMultipleMaterial(const FVoxelBlendedMaterial& Key, UMaterialInstanceDynamic* NewMultipleMaterial)
	{
		UMaterialInstanceDynamic*& Ref = Materials.FindOrAdd(Key);
		if (Ref)
		{
			MaterialsRef->UsedMaterials.Remove(Ref);
		}
		Ref = NewMultipleMaterial;
		if (Ref)
		{
			MaterialsRef->UsedMaterials.Add(Ref);
		}
	}
	void Reset()
	{
		MaterialsRef->UsedMaterials.Remove(SingleMaterial);
		for (auto& Material : Materials)
		{
			MaterialsRef->UsedMaterials.Remove(Material.Value);
		}
		SingleMaterial = nullptr;
		Materials.Reset();
	}
	
	void SetScalarParameterValue(FName ParameterName, float Value)
	{
		Apply([&](auto& M) { M->SetScalarParameterValue(ParameterName, Value); });
	}
	void SetTextureParameterValue(FName ParameterName, UTexture* Value)
	{		
		Apply([&](auto& M) { M->SetTextureParameterValue(ParameterName, Value); });
	}
	void SetVectorParameterValue(FName ParameterName, FLinearColor Value)
	{
		Apply([&](auto& M) { M->SetVectorParameterValue(ParameterName, Value); });
	}

private:
	FVoxelMaterialsRefHolder* MaterialsRef;
	UMaterialInstanceDynamic* SingleMaterial = nullptr;
	TMap<FVoxelBlendedMaterial, UMaterialInstanceDynamic*> Materials;

	template<typename T>
	inline void Apply(T F)
	{
		if (SingleMaterial)
		{
			F(SingleMaterial);
		}
		else
		{
			for (auto& MaterialIt : Materials)
			{
				F(MaterialIt.Value);
			}
		}
	}
};
