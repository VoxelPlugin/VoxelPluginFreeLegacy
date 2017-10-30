#include "VoxelPrivatePCH.h"
#include "Octree.h"

FOctree::FOctree(FIntVector Position, uint8 Depth, uint64 Id /*= -1*/) : Position(Position), Depth(Depth), Id(Id), bHasChilds(false)
{
	// Max for Id
	check(Depth <= 20);
}

bool FOctree::operator==(const FOctree& Other) const
{
	check((Id == Other.Id) == (Position == Other.Position && Depth == Other.Depth));
	return Id == Other.Id;
}

bool FOctree::operator<(const FOctree& Other) const
{
	return Id < Other.Id;
}

bool FOctree::operator>(const FOctree& Other) const
{
	return Id > Other.Id;
}

int FOctree::Size() const
{
	return 16 << Depth;
}

FIntVector FOctree::GetMinimalCornerPosition() const
{
	return Position - FIntVector(Size() / 2, Size() / 2, Size() / 2);
}

FIntVector FOctree::GetMaximalCornerPosition() const
{
	return Position + FIntVector(Size() / 2, Size() / 2, Size() / 2);
}

bool FOctree::IsLeaf() const
{
	return !bHasChilds;
}

bool FOctree::IsInOctree(int X, int Y, int Z) const
{
	return Position.X - Size() / 2 <= X && X < Position.X + Size() / 2
		&& Position.Y - Size() / 2 <= Y && Y < Position.Y + Size() / 2
		&& Position.Z - Size() / 2 <= Z && Z < Position.Z + Size() / 2;
}

void FOctree::LocalToGlobal(int X, int Y, int Z, int& OutX, int& OutY, int& OutZ) const
{
	OutX = X + (Position.X - Size() / 2);
	OutY = Y + (Position.Y - Size() / 2);
	OutZ = Z + (Position.Z - Size() / 2);
}

void FOctree::GlobalToLocal(int X, int Y, int Z, int& OutX, int& OutY, int& OutZ) const
{
	OutX = X - (Position.X - Size() / 2);
	OutY = Y - (Position.Y - Size() / 2);
	OutZ = Z - (Position.Z - Size() / 2);
}

uint64 FOctree::GetTopIdFromDepth(int8 Depth)
{
	return IntPow9(Depth);
}
