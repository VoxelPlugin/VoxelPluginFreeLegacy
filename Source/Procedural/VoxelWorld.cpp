#include "VoxelWorld.h"
#include "Components/CapsuleComponent.h"
#include "Engine.h"


AVoxelWorld::AVoxelWorld()
{
	PrimaryActorTick.bCanEverTick = true;
	Size = 1;
	bNotCreated = true;

	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionCylinder"));
	TouchCapsule->InitCapsuleSize(30.0f, 80.0f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	SetActorScale3D(100 * FVector::OneVector);
}

AVoxelWorld::~AVoxelWorld()
{
	DeleteWorld();
}


void AVoxelWorld::CreateWorld()
{
	bNotCreated = false;
	this->world = new World(Size);
	this->world->Level = Level;
	chunks.SetNumUninitialized(Size * Size * Size);
	for (int x = 0; x < Size; x++)
	{
		for (int y = 0; y < Size; y++)
		{
			for (int z = 0; z < Size; z++)
			{

				AChunkActor* chunkActor = this->GetWorld()->SpawnActor<AChunkActor>(FVector::ZeroVector, FRotator::ZeroRotator);
				Chunk* chunk = world->Chunks[x + Size*y + Size*Size*z];
				FString name = FString::FromInt(x) + ", " + FString::FromInt(y) + ", " + FString::FromInt(z);
				FVector relativeLocation = (Chunk::Size - 1) * FVector(x, y, z);

				chunkActor->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
				chunkActor->SetActorLabel(name);
				chunkActor->SetActorRelativeLocation(relativeLocation);
				chunkActor->SetActorRelativeRotation(FRotator::ZeroRotator);
				chunkActor->SetActorRelativeScale3D(FVector::OneVector);
				chunkActor->Initialize(chunk);

				chunks[x + Size*y + Size*Size*z] = chunkActor;
			}
		}
	}
}

void AVoxelWorld::DeleteWorld()
{
	for (auto& chunk : chunks)
	{
		chunk->Destroy();
	}

	delete world;
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

bool AVoxelWorld::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, Size))
		return ParentVal && bNotCreated;
	else
		return ParentVal;
}

void AVoxelWorld::ModifyVoxel(FVector hitPoint)
{
	FVector localVector = GetTransform().InverseTransformPosition(hitPoint);
	int x = FMath::RoundToInt(localVector.X);
	int y = FMath::RoundToInt(localVector.Y);
	int z = FMath::RoundToInt(localVector.Z);

	if (world->IsInWorld(x, y, z))
	{
		world->Modify(x, y, z);
		GetChunk(x, y, z)->Update();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Error: (%d, %d, %d) not in world"), x, y, z));
	}
}

AChunkActor* AVoxelWorld::GetChunk(int x, int y, int z)
{
	int chunkX = x / (Chunk::Size - 1);
	int chunkY = y / (Chunk::Size - 1);
	int chunkZ = z / (Chunk::Size - 1);
	return chunks[chunkX + Size*chunkY + Size*Size*chunkZ];
}
