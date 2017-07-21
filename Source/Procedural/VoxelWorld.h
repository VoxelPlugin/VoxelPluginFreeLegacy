#pragma once

#include "World.h"
#include "ChunkActor.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshActor.h"
#include "VoxelWorld.generated.h"


UCLASS()
class PROCEDURAL_API AVoxelWorld : public AActor
{
	GENERATED_BODY()

public:
	AVoxelWorld();
	~AVoxelWorld();

	virtual void Tick(float DeltaTime) override;

	void Update();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetLevel(float level);

protected:
	virtual void BeginPlay() override;

private:
	void CreateWorld();

	UPROPERTY(EditAnywhere, Category = Voxel)
		int NewSize;
	UPROPERTY(EditAnywhere, Category = Voxel)
		float NewScale;

	int Size;
	float Scale;
	World* world;
	TArray<AChunkActor*> chunks;
};
