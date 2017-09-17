// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelPart.h"
#include "ProceduralMeshComponent.h"
#include "VoxelData.h"
#include "VoxelRender.h"


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
}

void AVoxelPart::Init(VoxelData* Data, AVoxelWorld* World)
{
	check(Data);

	const int W = Data->Width() / 2;

	TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes;
	ChunkHasHigherRes.SetNumZeroed(6);

	int SectionIndex = 0;

	for (int X = -W; X < W; X += 16)
	{
		for (int Y = -W; Y < W; Y += 16)
		{
			for (int Z = -W; Z < W; Z += 16)
			{
				FIntVector Position = FIntVector(X, Y, Z);

				TSharedPtr<VoxelRender> Render = MakeShareable(new VoxelRender(0, Data, Position, ChunkHasHigherRes));

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
}
