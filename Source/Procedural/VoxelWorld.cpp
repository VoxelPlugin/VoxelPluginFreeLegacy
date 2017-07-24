#include "VoxelWorld.h"
#include "Components/CapsuleComponent.h"
#include "Engine.h"


AVoxelWorld::AVoxelWorld()
{
	PrimaryActorTick.bCanEverTick = true;
	Size = 1;
	bNotCreated = true;

	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionCylinder"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
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
				FVector relativeLocation = 16 * FVector(x, y, z);

				chunkActor->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
				chunkActor->SetActorLabel(name);
				chunkActor->SetActorRelativeLocation(relativeLocation);
				chunkActor->SetActorRelativeRotation(FRotator::ZeroRotator);
				chunkActor->SetActorRelativeScale3D(FVector::OneVector);
				chunkActor->Initialize(chunk, material);

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
	world->Plane();
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

void AVoxelWorld::Add(FVector hitPoint)
{
	FVector localVector = GetTransform().InverseTransformPosition(hitPoint);
	int x = FMath::RoundToInt(localVector.X);
	int y = FMath::RoundToInt(localVector.Y);
	int z = FMath::RoundToInt(localVector.Z);

	if (world->IsInWorld(x, y, z))
	{
		world->Add(x, y, z);
		GetChunk(x, y, z)->Update();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Error: (%d, %d, %d) not in world"), x, y, z));
	}
}

void AVoxelWorld::Remove(FVector hitPoint)
{
	FVector localVector = GetTransform().InverseTransformPosition(hitPoint);
	int x = FMath::RoundToInt(localVector.X);
	int y = FMath::RoundToInt(localVector.Y);
	int z = FMath::RoundToInt(localVector.Z);

	if (world->IsInWorld(x, y, z))
	{
		world->Remove(x, y, z);
		GetChunk(x, y, z)->Update();
		GetChunk(x + 1, y, z)->Update();
		GetChunk(x, y + 1, z)->Update();
		GetChunk(x, y, z + 1)->Update();

		GetChunk(x - 1, y, z)->Update();
		GetChunk(x, y - 1, z)->Update();
		GetChunk(x, y, z - 1)->Update();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Error: (%d, %d, %d) not in world"), x, y, z));
	}
}

AChunkActor* AVoxelWorld::GetChunk(int x, int y, int z)
{
	int chunkX = x / 16;
	int chunkY = y / 16;
	int chunkZ = z / 16;

	if (chunkX + Size*chunkY + Size*Size*chunkZ < chunks.Num())
	{
		return chunks[chunkX + Size*chunkY + Size*Size*chunkZ];
	}
	else
	{
		return chunks[0];
	}
}
