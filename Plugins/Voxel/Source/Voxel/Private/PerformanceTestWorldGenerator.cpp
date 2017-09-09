#include "VoxelPrivatePCH.h"
#include "PerformanceTestWorldGenerator.h"
#include "Engine/World.h"

float UPerformanceTestWorldGenerator::GetDefaultValue_Implementation(FIntVector Position)
{
	float CurrentRadius = (1 + FMath::Sin(Speed * World->GetWorld()->TimeSeconds)) * Radius / 2;
	return FVector(FMath::Abs(Position.X) % (2 * Radius) - Radius, FMath::Abs(Position.Y) % (2 * Radius) - Radius, FMath::Abs(Position.Z) % (2 * Radius) - Radius).Size() - CurrentRadius;
}

FColor UPerformanceTestWorldGenerator::GetDefaultColor_Implementation(FIntVector Position)
{
	return FColor::Green;
}

void UPerformanceTestWorldGenerator::SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld)
{
	World = VoxelWorld;
}
