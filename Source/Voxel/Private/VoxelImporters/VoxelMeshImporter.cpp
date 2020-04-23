// Copyright 2020 Phyronnaz

#include "VoxelImporters/VoxelMeshImporter.h"

#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelShaders/VoxelDistanceFieldShader.h"
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
	TArray<uint32>& Indices, 
	TArray<FVector2D>& UVs)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(InMesh->RenderData) || !ensure(InMesh->RenderData->LODResources.IsValidIndex(LODIndex))) return;

	const FStaticMeshLODResources& LODResources = InMesh->RenderData->LODResources[LODIndex];
	const FRawStaticIndexBuffer& IndexBuffer = LODResources.IndexBuffer;
	const FPositionVertexBuffer& PositionVertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;
	const FStaticMeshVertexBuffer& StaticMeshVertexBuffer = LODResources.VertexBuffers.StaticMeshVertexBuffer;
	const int32 NumTextureCoordinates = StaticMeshVertexBuffer.GetNumTexCoords();

	ensure(IndexBuffer.GetNumIndices() % 3 == 0);

	const auto Get = [](auto& Array, auto Index) -> auto&
	{
#if VOXEL_DEBUG
		return Array[Index];
#else
		return Array.GetData()[Index];
#endif
	};
	
	if (!FPlatformProperties::RequiresCookedData() || InMesh->bAllowCPUAccess)
	{
		{
			VOXEL_SCOPE_COUNTER("Copy Vertices from CPU");
			Vertices.SetNumUninitialized(PositionVertexBuffer.GetNumVertices());
			for (uint32 Index = 0; Index < PositionVertexBuffer.GetNumVertices(); Index++)
			{
				Get(Vertices, Index) = PositionVertexBuffer.VertexPosition(Index);
			}
		}
		{
			VOXEL_SCOPE_COUNTER("Copy Triangles from CPU");
			Indices.SetNumUninitialized(IndexBuffer.GetNumIndices());
			for (int32 Index = 0; Index < IndexBuffer.GetNumIndices(); Index++)
			{
				Get(Indices, Index) = IndexBuffer.GetIndex(Index);
			}
		}
		if (NumTextureCoordinates > 0)
		{
			VOXEL_SCOPE_COUNTER("Copy UVs from CPU");
			UVs.SetNumUninitialized(StaticMeshVertexBuffer.GetNumVertices());
			for (uint32 Index = 0; Index < StaticMeshVertexBuffer.GetNumVertices(); Index++)
			{
				Get(UVs, Index) = StaticMeshVertexBuffer.GetVertexUV(Index, 0);
			}
		}
	}
	else
	{
		LOG_VOXEL(Log, TEXT("Extracting mesh data from GPU for %s"), *InMesh->GetName());
		
		ENQUEUE_RENDER_COMMAND(VoxelDistanceFieldCompute)([&](FRHICommandListImmediate& RHICmdList)
		{
			{
				VOXEL_SCOPE_COUNTER("Copy Vertices from GPU");
				Vertices.SetNumUninitialized(PositionVertexBuffer.GetNumVertices());
				const int32 NumBytes = PositionVertexBuffer.GetNumVertices() * PositionVertexBuffer.GetStride();
				
				void* BufferData = RHICmdList.LockVertexBuffer(PositionVertexBuffer.VertexBufferRHI, 0, NumBytes, EResourceLockMode::RLM_ReadOnly);
				FMemory::Memcpy(Vertices.GetData(), BufferData, NumBytes);
				RHICmdList.UnlockVertexBuffer(PositionVertexBuffer.VertexBufferRHI);
			}
			{
				VOXEL_SCOPE_COUNTER("Copy Triangles from GPU");
				Indices.SetNumUninitialized(IndexBuffer.GetNumIndices());
				
				const bool bIs32Bit = IndexBuffer.Is32Bit();
				const int32 NumBytes = IndexBuffer.GetNumIndices() * (bIs32Bit ? sizeof(uint32) : sizeof(uint16));
				
				void* BufferData = RHICmdList.LockIndexBuffer(IndexBuffer.IndexBufferRHI, 0, NumBytes, EResourceLockMode::RLM_ReadOnly);
				if (bIs32Bit)
				{
					FMemory::Memcpy(Indices.GetData(), BufferData, NumBytes);
				}
				else
				{
					TArray<uint16> Indices16;
					Indices16.SetNumUninitialized(IndexBuffer.GetNumIndices());
					FMemory::Memcpy(Indices16.GetData(), BufferData, NumBytes);
					for (int32 Index = 0; Index < Indices16.Num(); Index++)
					{
						Get(Indices, Index) = Get(Indices16, Index);
					}
				}
				RHICmdList.UnlockIndexBuffer(IndexBuffer.IndexBufferRHI);
			}
			if (NumTextureCoordinates > 0)
			{
				VOXEL_SCOPE_COUNTER("Copy UVs from GPU");
				UVs.SetNumUninitialized(StaticMeshVertexBuffer.GetNumVertices());

				const bool bFullPrecision = StaticMeshVertexBuffer.GetUseFullPrecisionUVs();
				const int32 NumBytes = StaticMeshVertexBuffer.GetNumVertices() * (bFullPrecision ? sizeof(FVector2D) : sizeof(FVector2DHalf));
				
				void* BufferData = RHICmdList.LockVertexBuffer(StaticMeshVertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, NumBytes, EResourceLockMode::RLM_ReadOnly);
				if (bFullPrecision)
				{
					FMemory::Memcpy(UVs.GetData(), BufferData, NumBytes);
				}
				else
				{
					TArray<FVector2DHalf> UVsHalf;
					UVsHalf.SetNumUninitialized(StaticMeshVertexBuffer.GetNumVertices());
					FMemory::Memcpy(UVsHalf.GetData(), BufferData, NumBytes);
					for (int32 Index = 0; Index < UVsHalf.Num(); Index++)
					{
						Get(UVs, Index) = Get(UVsHalf, Index);
					}
				}
				RHICmdList.UnlockVertexBuffer(StaticMeshVertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
			}
		});
		
		FRenderCommandFence Fence;
		Fence.BeginFence();
		Fence.Wait();
	}
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

void UVoxelMeshImporterLibrary::ConvertMeshToDistanceField(
	const FVoxelMeshImporterInputData& Mesh,
	const FTransform& Transform,
	const float VoxelSize,
	float MaxDistance,
	float BoxExtension,
	TArray<float>& OutDistanceField,
	FIntVector& OutSize,
	FIntVector& OutOffset,
	int32& OutNumLeaks)
{
	VOXEL_PRO_ONLY_VOID();
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

			TArray<uint32> Indices;
			TArray<FVector2D> UVs;
			CachedVertices.Reset();
			GetMergedSectionFromStaticMesh(StaticMesh, 0, CachedVertices, Indices, UVs);
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
	auto* Material = LoadObject<UMaterial>(nullptr, TEXT("Material'/Voxel/MaterialHelpers/MeshImporterMaterial.MeshImporterMaterial'"));
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