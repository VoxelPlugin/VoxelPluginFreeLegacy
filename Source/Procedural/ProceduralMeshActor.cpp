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
	oldLevel = 0;
	level = -5;
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
	chunk = Chunk();
	chunk.Sphere();
}

// Called every frame
void AProceduralMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/**
	*	Create/replace a section for this procedural mesh component.
	*	@param	SectionIndex		Index of the section to create or replace.
	*	@param	Vertices			Vertex buffer of all vertex positions to use for this mesh section.
	*	@param	Triangles			Index buffer indicating which vertices make up each triangle. Length must be a multiple of 3.
	*	@param	Normals				Optional array of normal vectors for each vertex. If supplied, must be same length as Vertices array.
	*	@param	UV0					Optional array of texture co-ordinates for each vertex. If supplied, must be same length as Vertices array.
	*	@param	VertexColors		Optional array of colors for each vertex. If supplied, must be same length as Vertices array.
	*	@param	Tangents			Optional array of tangent vector for each vertex. If supplied, must be same length as Vertices array.
	*	@param	bCreateCollision	Indicates whether collision should be created for this section. This adds significant cost.
	*/
	//UFUNCTION(BlueprintCallable, Category = "Components|ProceduralMesh", meta = (AutoCreateRefTerm = "Normals,UV0,VertexColors,Tangents"))
	//	void CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals,
	// const TArray<FVector2D>& UV0, const TArray<FColor>& VertexColors, const TArray<FProcMeshTangent>& Tangents, bool bCreateCollision);

	if (oldLevel == level)
	{
		return;
	}
	chunk.Level = level;
	oldLevel = level;
	chunk.Process();


	TArray<FVector> vertices = chunk.Vertices;
	TArray<int> triangles = chunk.Triangles;
	TArray<FVector> normals = chunk.Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> vertexColors;
	TArray<FProcMeshTangent> tangents;


	UE_LOG(YourLog, Warning, TEXT("Vertices: %d"), vertices.Num());
	UE_LOG(YourLog, Warning, TEXT("Triangles: %d"), triangles.Num());


	mesh->CreateMeshSection(1, vertices, triangles, normals, UV0, vertexColors, tangents, false);
	mesh->SetWorldScale3D(FVector(100));
}

void AProceduralMeshActor::SetLevel(float level)
{
	this->level = level;
}

