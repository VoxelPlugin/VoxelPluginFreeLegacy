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
	
}

