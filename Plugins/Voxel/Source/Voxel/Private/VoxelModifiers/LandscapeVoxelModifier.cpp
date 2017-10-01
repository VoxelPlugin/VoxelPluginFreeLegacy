#include "VoxelPrivatePCH.h"
#include "VoxelModifiers/LandscapeVoxelModifier.h"
#include "VoxelWorld.h"
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

	ALandscapeVoxelAsset* InstancedLandscape = Landscape.GetDefaultObject();

	check(InstancedLandscape);

	InstancedLandscape->Init(World->GetVoxelSize());

	FIntVector Position = World->GlobalToLocal(GetActorLocation());

	for (int X = 0; X < InstancedLandscape->Size; X++)
	{
		for (int Y = 0; Y < InstancedLandscape->Size; Y++)
		{
			for (int Z = 0; Z < InstancedLandscape->Size; Z++)
			{
				FIntVector CurrentPosition = Position + FIntVector(X, Y, Z);
				World->SetValue(CurrentPosition, InstancedLandscape->GetDefaultValue(X, Y, Z));
				World->SetMaterial(CurrentPosition, InstancedLandscape->GetDefaultMaterial(X, Y, Z));
			}
		}
	}
}

void ALandscapeVoxelModifier::Render(FVector WorldPosition, float VoxelSize)
{

}
