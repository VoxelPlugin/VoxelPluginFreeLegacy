#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "VoxelWorld.h"

namespace MeshImporter
{
	void LineTrace(bool bAdd, UPrimitiveComponent* Component, TArray<std::forward_list<float>>& Values, TArray<bool>& IsInside, FTransform& WorldTransform, FIntVector IMin, FIntVector Size, int X, int Y, int Z, int DeltaX, int DeltaY, int DeltaZ);

	void ImportMesh(bool bAdd, UPrimitiveComponent* Component, AVoxelWorld* World, FVector MinBound, FVector MaxBound);
}
