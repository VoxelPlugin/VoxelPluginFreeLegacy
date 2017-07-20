// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralMeshActor.h"
#include "DrawDebugHelpers.h"

//General Log
DEFINE_LOG_CATEGORY(YourLog);

//Logging during game startup
DEFINE_LOG_CATEGORY(YourInit);

//Logging for your AI system
DEFINE_LOG_CATEGORY(YourAI);

//Logging for Critical Errors that must always be addressed
DEFINE_LOG_CATEGORY(YourCriticalErrors);


// Sets default values
AProceduralMeshActor::AProceduralMeshActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;
	chunk = Chunk();
	chunk.Sphere();
}

// Called when the game starts or when spawned
void AProceduralMeshActor::BeginPlay()
{
	Super::BeginPlay();

	// Lines
	//auto drawPoint = [this](float X, float Y, float Z)
	//{
	//	DrawDebugPoint(
	//		GetWorld(),
	//		GetActorLocation() + 100 * FVector(X, Y, Z),
	//		2,  					//size
	//		FColor(255, 0, 0),
	//		true
	//	);
	//};
	//for (int x = 0; x < chunk.Size; x++)
	//{
	//	for (int y = 0; y < chunk.Size; y++)
	//	{
	//		for (int z = 0; z < chunk.Size; z++)
	//		{
	//			drawPoint(x, y, z);
	//		}
	//	}
	//}
}

// Called every frame
void AProceduralMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProceduralMeshActor::SetLevel(float level)
{
	chunk.Level = level;
	chunk.Process();


	TArray<FVector> vertices = chunk.Vertices;
	TArray<int> triangles = chunk.Triangles;
	TArray<FVector> normals = chunk.Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> vertexColors;
	TArray<FProcMeshTangent> tangents;

	/*UE_LOG(YourLog, Warning, TEXT("Vertices: %d"), vertices.Num());
	UE_LOG(YourLog, Warning, TEXT("Triangles: %d"), triangles.Num());*/

	mesh->CreateMeshSection(0, vertices, triangles, normals, UV0, vertexColors, tangents, false);
	mesh->SetWorldScale3D(FVector(100));
}

