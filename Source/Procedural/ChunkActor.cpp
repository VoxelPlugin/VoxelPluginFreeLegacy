// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkActor.h"
#include "DrawDebugHelpers.h"



AChunkActor::AChunkActor()
{
	PrimaryActorTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("GeneratedMesh"));
	RootComponent = mesh;
}

void AChunkActor::Initialize(Chunk* chunk, float scale)
{
	this->chunk = chunk;
	this->scale = scale;

	// Lines
	//auto drawPoint = [this](float X, float Y, float Z, FColor color)
	//{
	//	DrawDebugPoint(
	//		GetWorld(),
	//		GetActorLocation() + this->scale * FVector(X, Y, Z),
	//		5,  					//size
	//		color,
	//		true
	//	);
	//};
	//FColor color = FColor::MakeRandomColor();
	//for (int x = 0; x < Chunk::Size; x++)
	//{
	//	for (int y = 0; y < Chunk::Size; y++)
	//	{
	//		for (int z = 0; z < Chunk::Size; z++)
	//		{
	//			drawPoint(x, y, z, color);
	//		}
	//	}
	//}
}

void AChunkActor::BeginPlay()
{
	Super::BeginPlay();

}

void AChunkActor::Update()
{
	chunk->Process();
	TArray<FVector> vertices = chunk->Vertices;
	TArray<int> triangles = chunk->Triangles;
	TArray<FVector> normals = chunk->Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> vertexColors;
	TArray<FProcMeshTangent> tangents;

	mesh->CreateMeshSection(0, vertices, triangles, normals, UV0, vertexColors, tangents, false);
	mesh->SetWorldScale3D(FVector(scale));
}

void AChunkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


