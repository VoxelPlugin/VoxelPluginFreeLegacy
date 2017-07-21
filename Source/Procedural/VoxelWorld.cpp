#include "VoxelWorld.h"
#include "Components/CapsuleComponent.h"


AVoxelWorld::AVoxelWorld()
{
	PrimaryActorTick.bCanEverTick = true;
	Size = 10;
	Scale = 100;

	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionCylinder"));
	TouchCapsule->InitCapsuleSize(30.0f, 80.0f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;
}

AVoxelWorld::~AVoxelWorld()
{
	delete world;
}


void AVoxelWorld::CreateWorld()
{
	this->world = new World(Size);
	chunks.SetNumUninitialized(Size * Size * Size);
	for (int x = 0; x < Size; x++)
	{
		for (int y = 0; y < Size; y++)
		{
			for (int z = 0; z < Size; z++)
			{

				AChunkActor* chunkActor = this->GetWorld()->SpawnActor<AChunkActor>(Chunk::Size * Scale * FVector(x, y, z), FRotator::ZeroRotator);
				Chunk* chunk = world->Chunks[x + Size*y + Size*Size*z];
				FString name = FString::FromInt(x) + ", " + FString::FromInt(y) + ", " + FString::FromInt(z);

				chunkActor->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
				chunkActor->SetActorLabel(name);
				chunkActor->Initialize(chunk, Scale);

				chunks[x + Size*y + Size*Size*z] = chunkActor;
			}
		}
	}
}

void AVoxelWorld::SetLevel(float level)
{
	world->Level = level;
	Update();
}

void AVoxelWorld::Update()
{
	for (auto& chunk : chunks)
	{
		chunk->Update();
	}
}


void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();
	CreateWorld();
	world->Sphere();
	Update();
}


void AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
