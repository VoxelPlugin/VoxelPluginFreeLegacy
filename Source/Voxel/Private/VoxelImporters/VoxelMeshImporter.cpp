// Copyright 2021 Phyronnaz

#include "VoxelImporters/VoxelMeshImporter.h"

#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.inl"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelUtilities/VoxelExampleUtilities.h"
#include "VoxelUtilities/VoxelDistanceFieldUtilities.h"
#include "VoxelMessages.h"


#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"

static void GetMergedSectionFromStaticMesh(
	UStaticMesh* InMesh, 
	int32 LODIndex, 
	TArray<FVector>& Vertices, 
	TArray<uint32>& Indices, 
	TArray<FVector2D>& UVs)
{
	VOXEL_FUNCTION_COUNTER();
	
	FStaticMeshRenderData* RenderData = UE_5_SWITCH(InMesh->RenderData.Get(), InMesh->GetRenderData());
	if (!ensure(RenderData) || !ensure(RenderData->LODResources.IsValidIndex(LODIndex))) return;

	const FStaticMeshLODResources& LODResources = RenderData->LODResources[LODIndex];
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
	ColorsMaterial = FVoxelExampleUtilities::LoadExampleObject<UMaterialInterface>(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_M_Chair_Emissive_Color"));
	UVsMaterial = FVoxelExampleUtilities::LoadExampleObject<UMaterialInterface>(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_M_Chair_Emissive_UVs"));
}

FVoxelMeshImporterSettings::FVoxelMeshImporterSettings(const FVoxelMeshImporterSettingsBase& Base)
	: FVoxelMeshImporterSettingsBase(Base)
{
	bImportColors = false;
	bImportUVs = false;
}

void UVoxelMeshImporterLibrary::CreateMeshDataFromStaticMesh(UStaticMesh* StaticMesh, FVoxelMeshImporterInputData& Data)
{
	VOXEL_FUNCTION_COUNTER();

	check(StaticMesh);
	Data.Vertices.Reset();
	Data.Triangles.Reset();
	Data.UVs.Reset();
	
	const int32 LOD = 0;
	TArray<uint32> Indices;

	GetMergedSectionFromStaticMesh(StaticMesh, LOD, Data.Vertices, Indices, Data.UVs);
	
	const auto Get = [](auto& Array, auto Index) -> auto&
	{
#if VOXEL_DEBUG
		return Array[Index];
#else
		return Array.GetData()[Index];
#endif
	};

	for (uint32 Index : Indices)
	{
		if (Index >= uint32(Data.Vertices.Num()))
		{
			FVoxelMessages::Error(FUNCTION_ERROR("Invalid index buffer"));
			Data = {};
			return;
		}
	}
	
	ensure(Indices.Num() % 3 == 0);
	Data.Triangles.SetNumUninitialized(Indices.Num() / 3);
	for (int32 Index = 0; Index < Data.Triangles.Num(); Index++)
	{
		Get(Data.Triangles, Index) = FIntVector(
			Get(Indices, 3 * Index + 0),
			Get(Indices, 3 * Index + 1),
			Get(Indices, 3 * Index + 2));
	}
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
	FVoxelMessages::Info(FUNCTION_ERROR("Converting meshes to voxels require Voxel Plugin Pro"));
	return false;
}

void UVoxelMeshImporterLibrary::ConvertMeshToDistanceField(
	const FVoxelMeshImporterInputData& Mesh,
	const FTransform& Transform,
	const FVoxelMeshImporterSettingsBase& Settings,
	float BoxExtension,
	TArray<float>& OutDistanceField,
	TArray<FVector>& OutSurfacePositions,
	FIntVector& OutSize,
	FIntVector& OutOffset,
	int32& OutNumLeaks,
	EVoxelComputeDevice Device,
	bool bMultiThreaded,
	int32 MaxPasses_Debug)
{
	FVoxelMessages::Info(FUNCTION_ERROR("Converting meshes to voxels require Voxel Plugin Pro"));
}

UVoxelMeshImporterInputData* UVoxelMeshImporterLibrary::CreateMeshDataFromStaticMesh(UStaticMesh* StaticMesh)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!StaticMesh)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid StaticMesh"));
		return nullptr;
	}
	auto* Object = NewObject<UVoxelMeshImporterInputData>(GetTransientPackage());
	CreateMeshDataFromStaticMesh(StaticMesh, Object->Data);
	return Object;
}

UTextureRenderTarget2D* UVoxelMeshImporterLibrary::CreateTextureFromMaterial(
	UObject* WorldContextObject,
	UMaterialInterface* Material,
	int32 Width,
	int32 Height)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!WorldContextObject)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid WorldContextObject"));
		return nullptr;
	}
	if (!Material)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid Material"));
		return nullptr;
	}
	if (Width <= 0)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Width <= 0"));
		return nullptr;
	}
	if (Height <= 0)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Height <= 0"));
		return nullptr;
	}

	UTextureRenderTarget2D* RenderTarget2D = UKismetRenderingLibrary::CreateRenderTarget2D(WorldContextObject, Width, Height, ETextureRenderTargetFormat::RTF_RGBA8);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(WorldContextObject, RenderTarget2D, Material);
	return RenderTarget2D;
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
	FVoxelMessages::Info(FUNCTION_ERROR("Converting meshes to voxels require Voxel Plugin Pro"));
}

void UVoxelMeshImporterLibrary::ConvertMeshToVoxels_NoMaterials(
	UObject* WorldContextObject, 
	UVoxelMeshImporterInputData* Mesh, 
	FTransform Transform, 
	bool bSubtractive, 
	FVoxelMeshImporterSettingsBase Settings, 
	UVoxelDataAsset*& Asset, 
	int32& NumLeaks)
{
	FVoxelMeshImporterRenderTargetCache RenderTargetCache;
	ConvertMeshToVoxels(WorldContextObject, Mesh, Transform, bSubtractive, FVoxelMeshImporterSettings(Settings), RenderTargetCache, Asset, NumLeaks);

	ensure(!RenderTargetCache.ColorsRenderTarget);
	ensure(!RenderTargetCache.UVsRenderTarget);
	ensure(!RenderTargetCache.LastRenderedColorsMaterial);
	ensure(!RenderTargetCache.LastRenderedUVsMaterial);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelMeshImporter::AVoxelMeshImporter()
{
#if WITH_EDITOR
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");

	StaticMesh = FVoxelExampleUtilities::LoadExampleObject<UStaticMesh>(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_SM_Chair"));
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
	const bool bHasMaterials = Settings.bImportColors || Settings.bImportUVs;
	SizeInMB = double(NumberOfVoxels) * ((ONE_BIT_VOXEL_VALUE ? 1 / 8. : sizeof(FVoxelValue)) + (bHasMaterials ? sizeof(FVoxelMaterial) : 0)) / double(1 << 20);
}