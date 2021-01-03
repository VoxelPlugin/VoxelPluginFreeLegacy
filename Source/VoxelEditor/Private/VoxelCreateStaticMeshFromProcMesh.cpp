// Copyright 2021 Phyronnaz

#include "VoxelCreateStaticMeshFromProcMesh.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"

#include "RawMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicsEngine/BodySetup.h"

void FVoxelCreateStaticMeshFromProcMesh::Init()
{
	FEditorDelegates::OnLightingBuildStarted.AddStatic(&FVoxelCreateStaticMeshFromProcMesh::OnLightingBuildStarted);
}

UStaticMesh* FVoxelCreateStaticMeshFromProcMesh::Create(UVoxelProceduralMeshComponent* Component, TFunction<UStaticMesh*()> CreateObject, bool bRecomputeNormals, bool bAllowTransientMaterials)
{
	// Raw mesh data we are filling in
	FRawMesh RawMesh;
	// Materials to apply to new mesh
	TArray<UMaterialInterface*> MeshMaterials;

	int32 VertexBase = 0;
	
	Component->IterateSections([&](const FVoxelProcMeshSectionSettings& SectionSettings, const FVoxelProcMeshBuffers& Buffers)
	{
		// Copy verts
		RawMesh.VertexPositions.Reserve(RawMesh.VertexPositions.Num() + Buffers.GetNumVertices());
		auto& PositionBuffer = Buffers.VertexBuffers.PositionVertexBuffer;
		for (int32 Index = 0; Index < Buffers.GetNumVertices(); Index++)
		{
			RawMesh.VertexPositions.Add(PositionBuffer.VertexPosition(Index));
		}

		auto& IndexBuffer = Buffers.IndexBuffer;
		auto& StaticMeshBuffer = Buffers.VertexBuffers.StaticMeshVertexBuffer;
		auto& ColorBuffer = Buffers.VertexBuffers.ColorVertexBuffer;

		// Copy 'wedge' info
		check(StaticMeshBuffer.GetNumTexCoords() <= MAX_MESH_TEXTURE_COORDS);
		for (int32 IndexIterator = 0; IndexIterator < IndexBuffer.GetNumIndices(); IndexIterator++)
		{
			const uint32 VertexIndex = IndexBuffer.GetIndex(IndexIterator);

			RawMesh.WedgeIndices.Add(VertexIndex + VertexBase);

			RawMesh.WedgeTangentX.Add(StaticMeshBuffer.VertexTangentX(VertexIndex));
			RawMesh.WedgeTangentY.Add(StaticMeshBuffer.VertexTangentY(VertexIndex));
			RawMesh.WedgeTangentZ.Add(StaticMeshBuffer.VertexTangentZ(VertexIndex));

			for (uint32 Tex = 0; Tex < StaticMeshBuffer.GetNumTexCoords(); Tex++)
			{
				RawMesh.WedgeTexCoords[Tex].Add(StaticMeshBuffer.GetVertexUV(VertexIndex, Tex));
			}
			RawMesh.WedgeColors.Add(ColorBuffer.VertexColor(VertexIndex));
		}

		// copy face info
		for (int32 Index = 0; Index < IndexBuffer.GetNumIndices() / 3; Index++)
		{
			RawMesh.FaceMaterialIndices.Add(MeshMaterials.Num());
			RawMesh.FaceSmoothingMasks.Add(0); // Assume this is ignored as bRecomputeNormals is false
		}

		UMaterialInterface* Material = nullptr;
		if (SectionSettings.Material.IsValid())
		{
			Material = SectionSettings.Material->GetMaterial();
		}

		if (!bAllowTransientMaterials)
		{
			while (Material &&
				!(Material->GetOutermost() != GetTransientPackage() && Material->HasAnyFlags(RF_Public)))
			{
				if (auto* Instance = Cast<UMaterialInstanceDynamic>(Material))
				{
					Material = Instance->Parent;
					continue;
				}
				if (auto* Instance = Cast<UMaterialInstanceConstant>(Material))
				{
					Material = Instance->Parent;
					continue;
				}

				// Give up
				Material = nullptr;
			}
		}

		// Remember material
		MeshMaterials.Add(Material);

		// Update offset for creating one big index/vertex buffer
		VertexBase += Buffers.GetNumVertices();
	});

	if (RawMesh.VertexPositions.Num() < 3 || RawMesh.WedgeIndices.Num() < 3)
	{
		return nullptr;
	}

	UStaticMesh* StaticMesh = CreateObject();
	check(StaticMesh);
	StaticMesh->InitResources();

	StaticMesh->LightingGuid = FGuid::NewGuid();

	// Add source to new StaticMesh
	FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
	SrcModel.BuildSettings.bRecomputeNormals = bRecomputeNormals;
	SrcModel.BuildSettings.bRecomputeTangents = bRecomputeNormals;
	SrcModel.BuildSettings.bRemoveDegenerates = false;
	SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
	SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
	SrcModel.BuildSettings.bGenerateLightmapUVs = true;
	SrcModel.BuildSettings.SrcLightmapIndex = 0;
	SrcModel.BuildSettings.DstLightmapIndex = 1;
	SrcModel.SaveRawMesh(RawMesh);

	// Copy materials to new mesh
	for (UMaterialInterface* Material : MeshMaterials)
	{
		StaticMesh->StaticMaterials.Add(FStaticMaterial(Material));
	}

	// Configure collision
	StaticMesh->CreateBodySetup();
	StaticMesh->BodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
	
	// Set the Imported version before calling the build
	StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;

	// Build mesh from source
	StaticMesh->Build(false);
	StaticMesh->PostEditChange();

	return StaticMesh;
}

void FVoxelCreateStaticMeshFromProcMesh::OnLightingBuildStarted()
{
	VOXEL_FUNCTION_COUNTER();
	
	for (TObjectIterator<UVoxelProceduralMeshComponent> It; It; ++It)
	{
		auto* ProcMesh = *It;
		if (ProcMesh->HasAnyFlags(RF_ClassDefaultObject))
		{
			continue;
		}
		
		ProcMesh->UpdateStaticMeshComponent();
	}
}