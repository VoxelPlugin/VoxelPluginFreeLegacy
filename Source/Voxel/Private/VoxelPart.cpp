// Copyright 2018 Phyronnaz

#include "VoxelPart.h"
#include "VoxelProceduralMeshComponent.h"
#include "VoxelAutoDisableComponent.h"
#include "VoxelData.h"
#include "VoxelPolygonizer.h"
#include "VoxelGlobals.h"
#include "VoxelThread.h"
#include "VoxelWorld.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"

AVoxelPartSimpleMesh::AVoxelPartSimpleMesh()
	: AutoDisableComponent(nullptr)
{
	SetActorEnableCollision(true);
	Mesh = CreateDefaultSubobject<UVoxelProceduralMeshComponent>("Mesh");
	SetRootComponent(Mesh);
}

void AVoxelPartSimpleMesh::Init(FVoxelData* Data, AVoxelWorld* World)
{
	check(Data);
		
	Mesh->bUseAsyncCooking = true;
	Mesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	Mesh->bUseComplexAsSimpleCollision = false;
	Mesh->bCastShadowAsTwoSided = true;
	Mesh->SetSimulatePhysics(false);

	auto Material = World->GetVoxelMaterialDynamicInstance();

	const int S = Data->Size() / 2;
	int SectionIndex = 0;

	for (int X = -S; X < S; X += CHUNK_SIZE)
	{
		for (int Y = -S; Y < S; Y += CHUNK_SIZE)
		{
			for (int Z = -S; Z < S; Z += CHUNK_SIZE)
			{
				const FIntVector Position = FIntVector(X, Y, Z);

				TSharedPtr<FAsyncPolygonizerWork> Thread = MakeShareable(new FAsyncPolygonizerWork(0, Data, Position, FIntVector(0, 0, 0), World));
				Thread->DoWork();

				// Mesh
				{
					FVoxelProcMeshSection Section;
					Section.bEnableCollision = true;
					Thread->Chunk.InitSectionBuffers(Section.ProcVertexBuffer, Section.ProcIndexBuffer, 0);

					TArray<FVector> Vertices;
					Vertices.SetNumUninitialized(Section.ProcVertexBuffer.Num());

					int i = 0;
					Section.SectionLocalBox.Init();
					for (FVoxelProcMeshVertex& ProcVertex : Section.ProcVertexBuffer)
					{
						ProcVertex.Position += (FVector)Position;
						ProcVertex.Position = World->LocalToGlobalFloat(ProcVertex.Position) - World->GetActorLocation();
						Section.SectionLocalBox += ProcVertex.Position;
						Vertices[i] = ProcVertex.Position;
						i++;
					}

					Mesh->SetProcMeshSection(SectionIndex, Section);
					Mesh->SetMaterial(SectionIndex, Material);
					Mesh->AddCollisionConvexMesh(Vertices);
					SectionIndex++;
				}

			}
		}
	}

	check(!AutoDisableComponent);
	AutoDisableComponent = NewObject<UVoxelAutoDisableComponent>(this, NAME_None, RF_NoFlags);
	AutoDisableComponent->RegisterComponent();
	
	FTimerHandle DummyHandle;
	GetWorldTimerManager().SetTimer(DummyHandle, [=]() { Mesh->SetSimulatePhysics(true); }, 0.1f, false);

	Super::Init(Data, World);
}
