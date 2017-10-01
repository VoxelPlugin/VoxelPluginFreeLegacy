#include "LandscapeVoxelModifier.h"
#include "VoxelWorld.h"
#include "LandscapeVoxelAsset.h"
#include "Components/CapsuleComponent.h"

ALandscapeVoxelModifier::ALandscapeVoxelModifier()
{
	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;
}

void ALandscapeVoxelModifier::ApplyToWorld(AVoxelWorld* World)
{
	check(Landscape);

	Landscape->Init(World->GetVoxelSize());

	FIntVector Position = World->GlobalToLocal(GetActorLocation());

	for (int X = 0; X < Landscape->Size; X++)
	{
		for (int Y = 0; Y < Landscape->Size; Y++)
		{
			for (int Z = 0; Z < Landscape->Size; Z++)
			{
				FIntVector CurrentPosition = Position + FIntVector(X, Y, Z);
				World->SetValue(CurrentPosition, Landscape->GetDefaultValue(X, Y, Z));
				World->SetMaterial(CurrentPosition, Landscape->GetDefaultMaterial(X, Y, Z));
			}
		}
	}
}

void ALandscapeVoxelModifier::Render(FVector WorldPosition, float VoxelSize)
{

}
