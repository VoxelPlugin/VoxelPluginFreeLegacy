#include "VoxelPrivatePCH.h"
#include "VoxelRender.h"

VoxelRender::VoxelRender(int Depth, ValueOctree* Octree) : Depth(Depth), Octree(Octree)
{

}

void VoxelRender::CreateSection(FProcMeshSection & OutSection)
{
	// Cache signs
	for (int X = 0; X < 16; X++)
	{
		for (int Y = 0; Y < 16; Y++)
		{
			for (int Z = 0; Z < 16; Z++)
			{
				int CurrentBit = (X % 4) + 4 * (Y % 4) + 4 * 4 * (Z % 4);
				bool Sign = Octree->GetValue(FIntVector(X, Y, Z)) > 0;
				// X / 4 + 4 * (Y / 4) + 4 * 4 * (Z / 4)
				Signs[X / 4 + Y + 4 * Z] = Signs[X / 4 + Y + 4 * Z] & (0xFFFFFFFFFFFFFFFF - CurrentBit) | (CurrentBit * Sign);
			}
		}
	}

	TSharedPtr<std::forward_list<FVector>> Vertices;
	TSharedPtr<std::forward_list<FColor>> Colors;
	TSharedPtr<std::forward_list<int>> Triangles;


}

int VoxelRender::Width()
{
	return 16 << Depth;
}

int VoxelRender::Step()
{
	return 1 << Depth;
}
