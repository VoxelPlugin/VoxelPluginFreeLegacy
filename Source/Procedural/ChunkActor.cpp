// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkActor.h"
#include "DrawDebugHelpers.h"
#include "EngineGlobals.h"
#include "Engine.h"



AChunkActor::AChunkActor()
{
	PrimaryActorTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("GeneratedMesh"));
	RootComponent = mesh;
	bActorLabelEditable = false;
	bLockLocation = false;
	mesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
}

void AChunkActor::Initialize(Chunk* chunk)
{
	this->chunk = chunk;

	// Lines
	//auto drawPoint = [this](float X, float Y, float Z, FColor color)
	//{
	//	DrawDebugPoint(
	//		GetWorld(),
	//		GetActorLocation() + 100 * FVector(X, Y, Z),
	//		5,  					//size
	//		color,
	//		true
	//	);
	//};
	//FColor color = FColor::MakeRandomColor();
	//for (int x = 0; x < 16; x++)
	//{
	//	for (int y = 0; y < 16; y++)
	//	{
	//		for (int z = 0; z < 16; z++)
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
	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FColor> vertexColors;
	TArray<FProcMeshTangent> tangents;

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Triangles/Vertices: %d, %d"), triangles.Num(), vertices.Num()));

	mesh->CreateMeshSection(0, vertices, triangles, normals, UV0, vertexColors, tangents, true);
}

void AChunkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


