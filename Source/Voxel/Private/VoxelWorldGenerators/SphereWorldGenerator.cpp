// Copyright 2018 Phyronnaz

#include "VoxelWorldGenerators/SphereWorldGenerator.h"
#include "VoxelWorld.h"

FSphereWorldGeneratorInstance::FSphereWorldGeneratorInstance(float Radius, const FVoxelMaterial& Material, bool bInverseOutsideInside)
    : Radius(Radius)
    , Material(Material)
    , bInverseOutsideInside(bInverseOutsideInside)
{
    
}

void FSphereWorldGeneratorInstance::GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const
{
	for (int K = 0; K < Size.Z; K++)
	{
		const int Z = Start.Z + K * Step;

		for (int J = 0; J < Size.Y; J++)
		{
			const int Y = Start.Y + J * Step;

			for (int I = 0; I < Size.X; I++)
			{
				const int X = Start.X + I * Step;


				const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

				// Distance to the center
				float Distance = FVector(X, Y, Z).Size();

				// Alpha = -1 inside the sphere, 1 outside and an interpolated value on intersection
				float Alpha = FMath::Clamp(Distance - LocalRadius, -2.f, 2.f) / 2;

				if (Values)
				{
					Values[Index] = Alpha * (bInverseOutsideInside ? -1 : 1);
				}
				if (Materials)
				{
					Materials[Index] = Material;
				}
				if (VoxelTypes)
				{
					VoxelTypes[Index] = FVoxelType(Alpha > 1 - KINDA_SMALL_NUMBER ? EVoxelValueType::IgnoreValue : EVoxelValueType::UseValue, EVoxelMaterialType::UseMaterial);
				}
			}
		}
	}
}

void FSphereWorldGeneratorInstance::SetVoxelWorld(const AVoxelWorld* VoxelWorld)
{
	LocalRadius = Radius / VoxelWorld->GetVoxelSize();
}

FVector FSphereWorldGeneratorInstance::GetUpVector(int X, int Y, int Z) const
{
	return FVector(X, Y, Z).GetSafeNormal();
}

////////////////////////////////////////////////////////////////////

USphereWorldGenerator::USphereWorldGenerator() 
    : Radius(1000)
    , InverseOutsideInside(false)
{

}

TSharedRef<FVoxelWorldGeneratorInstance> USphereWorldGenerator::GetWorldGenerator() 
{
     return MakeShareable(new FSphereWorldGeneratorInstance(Radius, Material, InverseOutsideInside));
}