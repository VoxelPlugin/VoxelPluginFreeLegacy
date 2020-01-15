// Copyright 2020 Phyronnaz

#include "VoxelImporters/VoxelMeshImporter.h"

#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelMathUtilities.h"
#include "VoxelMessages.h"


#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "UObject/ConstructorHelpers.h"

static void GetMergedSectionFromStaticMesh(
	UStaticMesh* InMesh, 
	int32 LODIndex, 
	TArray<FVector>& Vertices, 
	TArray<FIntVector>& Triangles, 
	TArray<FVector2D>& UVs)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(InMesh->RenderData) || !ensure(InMesh->RenderData->LODResources.IsValidIndex(LODIndex))) return;
	
	const bool bAllowCPUAccess = InMesh->bAllowCPUAccess;
	InMesh->bAllowCPUAccess = true;

	const FStaticMeshLODResources& LODResources = InMesh->RenderData->LODResources[LODIndex];
	const auto& IndexBuffer = LODResources.IndexBuffer;
	const auto& PositionVertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;
	const auto& StaticMeshVertexBuffer = LODResources.VertexBuffers.StaticMeshVertexBuffer;
	const int32 NumTextureCoordinates = StaticMeshVertexBuffer.GetNumTexCoords();

	Vertices.Reserve(PositionVertexBuffer.GetNumVertices());
	if (NumTextureCoordinates > 0)
	{
		UVs.Reserve(StaticMeshVertexBuffer.GetNumVertices());
	}
	Triangles.Reserve(IndexBuffer.GetNumIndices());
	
	TMap<int32, int32> MeshToNewVertices;
	MeshToNewVertices.Reserve(IndexBuffer.GetNumIndices());
	
	for (auto& Section : LODResources.Sections)
	{
		for (uint32 TriangleIndex = 0; TriangleIndex < Section.NumTriangles; TriangleIndex++)
		{
			FIntVector NewTriangle;
			for (uint32 TriangleVertexIndex = 0; TriangleVertexIndex < 3; TriangleVertexIndex++)
			{
				int32 IndexInNewVertices;
				{
					const int32 Index = IndexBuffer.GetIndex(Section.FirstIndex + 3 * TriangleIndex + TriangleVertexIndex);
					int32* NewIndexPtr = MeshToNewVertices.Find(Index);
					if (NewIndexPtr)
					{
						IndexInNewVertices = *NewIndexPtr;
					}
					else
					{
						const FVector Vertex = PositionVertexBuffer.VertexPosition(Index);
						IndexInNewVertices = Vertices.Add(Vertex);
						if (NumTextureCoordinates > 0)
						{
							const FVector2D UV = StaticMeshVertexBuffer.GetVertexUV(Index, 0);
							ensure(IndexInNewVertices == UVs.Add(UV));
						}
						MeshToNewVertices.Add(Index, IndexInNewVertices);
					}
				}
				NewTriangle[TriangleVertexIndex] = IndexInNewVertices;
			}
			Triangles.Add(NewTriangle);
		}
	}

	InMesh->bAllowCPUAccess = bAllowCPUAccess;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMeshImporterSettings::FVoxelMeshImporterSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ColorMaterialFinder(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_M_Chair_Emissive_Color"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UVsMaterialFinder(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_M_Chair_Emissive_UVs"));
	ColorsMaterial = ColorMaterialFinder.Object;
	UVsMaterial = UVsMaterialFinder.Object;
}

void UVoxelMeshImporterLibrary::CreateMeshDataFromStaticMesh(UStaticMesh* StaticMesh, FVoxelMeshImporterInputData& Data)
{
	VOXEL_PRO_ONLY_VOID();
}

bool UVoxelMeshImporterLibrary::ConvertMeshToVoxels(
	UObject* WorldContextObject,
	const FVoxelMeshImporterInputData& Mesh,
	const FTransform& Transform,
	const FVoxelMeshImporterSettings& Settings,
	FVoxelMeshImporterRenderTargetCache& RenderTargetCache,
	FVoxelDataAssetData& OutAsset,
	FIntVector& OutOffset,
	int32& OutNumLeaks)
{
	VOXEL_PRO_ONLY();
}

UVoxelMeshImporterInputData* UVoxelMeshImporterLibrary::CreateMeshDataFromStaticMesh(UStaticMesh* StaticMesh)
{
	VOXEL_PRO_ONLY();
}

UTextureRenderTarget2D* UVoxelMeshImporterLibrary::CreateTextureFromMaterial(
	UObject* WorldContextObject,
	UMaterialInterface* Material,
	int32 Width,
	int32 Height)
{
	VOXEL_PRO_ONLY();
}

void UVoxelMeshImporterLibrary::ConvertMeshToVoxels(
	UObject* WorldContextObject,
	UVoxelMeshImporterInputData* Mesh,
	FTransform Transform,
	bool bSubtractive,
	FVoxelMeshImporterSettings Settings,
	FVoxelMeshImporterRenderTargetCache& RenderTargetCache,
	UVoxelDataAsset*& Asset,
	int32& NumLeaks)
{
	VOXEL_PRO_ONLY_VOID();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelMeshImporter::AVoxelMeshImporter()
{
#if WITH_EDITOR
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_SM_Chair"));

	StaticMesh = MeshFinder.Object;
	MeshComponent->SetStaticMesh(StaticMesh);
	MeshComponent->SetRelativeScale3D(FVector(100.f));
	RootComponent = MeshComponent;

	PrimaryActorTick.bCanEverTick = true;
#endif
}

void AVoxelMeshImporter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (GetWorld()->WorldType != EWorldType::Editor)
	{
		Destroy();
	}

	if (StaticMesh)
	{
		if (CachedStaticMesh != StaticMesh)
		{
			CachedStaticMesh = StaticMesh;

			TArray<FIntVector> Triangles;
			TArray<FVector2D> UVs;
			CachedVertices.Reset();
			GetMergedSectionFromStaticMesh(StaticMesh, 0, CachedVertices, Triangles, UVs);
		}

		// TODO: Use PostEditMove
		const FTransform Transform = GetTransform();
		if (CachedTransform.ToMatrixWithScale() != Transform.ToMatrixWithScale())
		{
			CachedTransform = Transform;

			CachedBox = FBox(ForceInit);
			for (auto& Vertex : CachedVertices)
			{
				CachedBox += Transform.TransformPosition(Vertex);
			}
			CachedBox = CachedBox.ExpandBy(Settings.VoxelSize);

			InitMaterialInstance();
			MaterialInstance->SetVectorParameterValue("Offset", CachedBox.Min);
		}

		UpdateSizes();
	}
}

#if WITH_EDITOR
void AVoxelMeshImporter::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	MeshComponent->SetStaticMesh(StaticMesh);
	InitMaterialInstance();
	MaterialInstance->SetScalarParameterValue("VoxelSize", Settings.VoxelSize);
	UpdateSizes();
}
#endif

void AVoxelMeshImporter::InitMaterialInstance()
{
	if (MaterialInstance)
	{
		return;
	}
	auto* Material = LoadObject<UMaterial>(GetTransientPackage(), TEXT("Material'/Voxel/MaterialHelpers/MeshImporterMaterial.MeshImporterMaterial'"));
	MaterialInstance = UMaterialInstanceDynamic::Create(Material, GetTransientPackage());
	MeshComponent->SetMaterial(0, MaterialInstance);
	MaterialInstance->SetScalarParameterValue("VoxelSize", Settings.VoxelSize); // To have it on start
}

void AVoxelMeshImporter::UpdateSizes()
{
	const FVector SizeFloat = CachedBox.GetSize() / Settings.VoxelSize;
	SizeX = FMath::CeilToInt(SizeFloat.X);
	SizeY = FMath::CeilToInt(SizeFloat.Y);
	SizeZ = FMath::CeilToInt(SizeFloat.Z);
	NumberOfVoxels = SizeX * SizeY * SizeZ;
	const bool bHasMaterials =
		Settings.bPaintColors ||
		Settings.bPaintUVs ||
		Settings.bSetSingleIndex ||
		Settings.bSetDoubleIndex;
	SizeInMB = double(NumberOfVoxels) * (sizeof(FVoxelValue) + (bHasMaterials ? sizeof(FVoxelMaterial) : 0)) / double(1 << 20);
}