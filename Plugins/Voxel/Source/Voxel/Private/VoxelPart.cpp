// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelPart.h"
#include "ProceduralMeshComponent.h"
#include "VoxelData.h"
#include "VoxelPolygonizer.h"
#include "VoxelAutoDisableComponent.h"


AVoxelPart::AVoxelPart()
{
	// Create primary mesh
	PrimaryMesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("PrimaryMesh"));
	RootComponent = PrimaryMesh;

	PrimaryMesh->SetSimulatePhysics(true);
	PrimaryMesh->bCastShadowAsTwoSided = true;
	PrimaryMesh->bUseAsyncCooking = true;
	PrimaryMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	PrimaryMesh->bUseComplexAsSimpleCollision = false;

	SetActorEnableCollision(true);



	// Create auto disable
	AutoDisableComponent = CreateDefaultSubobject<UVoxelAutoDisableComponent>(FName("AutoDisable"));
	AutoDisableComponent->bAutoFindComponent = false;
}

void AVoxelPart::Init(VoxelData* Data, AVoxelWorld* World)
{
	check(Data);

	const int S = Data->Size() / 2;

	TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes;
	ChunkHasHigherRes.SetNumZeroed(6);

	int SectionIndex = 0;

	for (int X = -S; X < S; X += 16)
	{
		for (int Y = -S; Y < S; Y += 16)
		{
			for (int Z = -S; Z < S; Z += 16)
			{
				FIntVector Position = FIntVector(X, Y, Z);

				TSharedPtr<VoxelPolygonizer> Render = MakeShareable(new VoxelPolygonizer(0, Data, Position, ChunkHasHigherRes, false));

				TSharedPtr<FProcMeshSection> Section = MakeShareable(new FProcMeshSection());
				Render->CreateSection(*Section);

				TArray<FVector> Vertice;
				Vertice.SetNumUninitialized(Section->ProcVertexBuffer.Num());

				int i = 0;
				Section->SectionLocalBox.Init();
				for (FProcMeshVertex& ProcVertex : Section->ProcVertexBuffer)
				{
					ProcVertex.Position += (FVector)Position;
					Section->SectionLocalBox += ProcVertex.Position;
					Vertice[i] = ProcVertex.Position;
					i++;
				}

				PrimaryMesh->SetProcMeshSection(SectionIndex, *Section);
				PrimaryMesh->SetMaterial(SectionIndex, World->VoxelMaterial);
				PrimaryMesh->AddCollisionConvexMesh(Vertice);
				SectionIndex++;
			}
		}
	}
	AutoDisableComponent->bSimulatePhysics = true;
	AutoDisableComponent->Component = PrimaryMesh;
	AutoDisableComponent->World = World;
	AutoDisableComponent->bAutoFindWorld = false;
	AutoDisableComponent->CullDepth = 20;
}
