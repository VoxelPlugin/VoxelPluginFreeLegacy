#include "VoxelPrivatePCH.h"
#include "Octree.h"

Octree::Octree(FIntVector Position, uint8 Depth, uint64 Id /*= -1*/) : Position(Position), Depth(Depth), Id(Id == -1 ? IntPow9(Depth) : Id), bHasChilds(false)
{
	// Max for Id
	check(Depth <= 20);
};

bool Octree::operator<(const Octree& Other) const
{
	return Id < Other.Id;
}

bool Octree::operator>(const Octree& Other) const
{
	return Id > Other.Id;
}

int Octree::Width() const
{
	return 16 << Depth;
}

bool Octree::IsLeaf() const
{
	return !bHasChilds;
}

bool Octree::IsInOctree(FIntVector GlobalPosition) const
{
	return IsInOctree(GlobalPosition.X, GlobalPosition.Y, GlobalPosition.Z);
}

bool Octree::IsInOctree(int X, int Y, int Z) const
{
	return Position.X - Width() / 2 <= X && X < Position.X + Width() / 2 &&
		Position.Y - Width() / 2 <= Y && Y < Position.Y + Width() / 2 &&
		Position.Z - Width() / 2 <= Z && Z < Position.Z + Width() / 2;
}

FIntVector Octree::LocalToGlobal(FIntVector LocalPosition) const
{
	FIntVector GlobalPosition;
	GlobalToLocal(LocalPosition.X, LocalPosition.Y, LocalPosition.Z, LocalPosition.X, LocalPosition.Y, LocalPosition.Z);
	return GlobalPosition;
}

FORCEINLINE void Octree::LocalToGlobal(int X, int Y, int Z, int& OutX, int& OutY, int& OutZ) const
{
	OutX = X + (Position.X - Width() / 2);
	OutY = Y + (Position.Y - Width() / 2);
	OutZ = Z + (Position.Z - Width() / 2);
}

FIntVector Octree::GlobalToLocal(FIntVector GlobalPosition) const
{
	FIntVector LocalPosition;
	GlobalToLocal(GlobalPosition.X, GlobalPosition.Y, GlobalPosition.Z, LocalPosition.X, LocalPosition.Y, LocalPosition.Z);
	return LocalPosition;
}

void Octree::GlobalToLocal(int X, int Y, int Z, int& OutX, int& OutY, int& OutZ) const
{
	OutX = X - (Position.X - Width() / 2);
	OutY = Y - (Position.Y - Width() / 2);
	OutZ = Z - (Position.Z - Width() / 2);
}

bool Octree::operator==(const Octree& Other) const
{
	check((Id == Other.Id) == (Position == Other.Position && Depth == Other.Depth));
	return Id == Other.Id;
}
