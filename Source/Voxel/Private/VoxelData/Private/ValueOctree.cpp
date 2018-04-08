// Copyright 2018 Phyronnaz

#include "ValueOctree.h"
#include "VoxelWorldGenerator.h"
#include "VoxelUtilities.h"
#include "ScopeLock.h"

FValueOctree::FValueOctree(TSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator, uint8 LOD, bool bMultiplayer)
	: TVoxelOctree(LOD)
	, WorldGenerator(WorldGenerator)
	, bIsDirty(false)
	, bIsNetworkDirty(false)
	, bMultiplayer(bMultiplayer)
	, Values(nullptr)
	, Materials(nullptr)
{

}

FValueOctree::FValueOctree(FValueOctree* Parent, uint8 ChildIndex)
	: TVoxelOctree(Parent, ChildIndex)
	, WorldGenerator(Parent->WorldGenerator)
	, bIsDirty(false)
	, bIsNetworkDirty(false)
	, bMultiplayer(Parent->bMultiplayer)
	, Values(nullptr)
	, Materials(nullptr)
{

}

FValueOctree::~FValueOctree()
{
	if (Values)
	{
		delete[] Values;
	}
	if (Materials)
	{
		delete[] Materials;
	}
}

bool FValueOctree::IsDirty() const
{
	check(LOD == 0);
	return bIsDirty;
}

bool FValueOctree::IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const
{
	FIntBox InBounds(Start, Start + Size * Step);
	

	if (IsLeaf())
	{
		return (LOD != 0 || !IsDirty()) && WorldGenerator->IsEmpty(Start, Step, Size);
	}
	else
	{
		for (auto Child : GetChilds())
		{
			if (Child->GetBounds().Intersect(InBounds) && !Child->IsEmpty(Start, Step, Size))
			{
				return false;
			}
		}
		return true;
	}
}

void FValueOctree::GetValuesAndMaterials(float InValues[], FVoxelMaterial InMaterials[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const
{
	check(Size.GetMin() >= 0);
	if (Size.X == 0 || Size.Y == 0 || Size.Z == 0)
	{
		return;
	}
	check(IsInOctree(Start.X, Start.Y, Start.Z));
	check(IsInOctree(Start.X + (Size.X - 1) * Step, Start.Y + (Size.Y - 1) * Step, Start.Z + (Size.Z - 1) * Step));

	if (IsLeaf())
	{
		if (LOD == 0 && IsDirty())
		{
			for (int I = 0; I < Size.X; I++)
			{
				for (int J = 0; J < Size.Y; J++)
				{
					for (int K = 0; K < Size.Z; K++)
					{
						const int X = Start.X + I * Step;
						const int Y = Start.Y + J * Step;
						const int Z = Start.Z + K * Step;

						int LocalX, LocalY, LocalZ;
						GlobalToLocal(X, Y, Z, LocalX, LocalY, LocalZ);

						const int LocalIndex = IndexFromCoordinates(LocalX, LocalY, LocalZ);
						const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

						if (InValues)
						{
							InValues[Index] = Values[LocalIndex];
						}
						if (InMaterials)
						{
							InMaterials[Index] = Materials[LocalIndex];
						}
					}
				}
			}
		}
		else
		{
			WorldGenerator->GetValuesAndMaterialsAndVoxelTypes(InValues, InMaterials, nullptr, Start, StartIndex, Step, Size, ArraySize);
		}
	}
	else if (Size.X == 1 && Size.Y == 1 && Size.Z == 1 && false)
	{
		GetChild(Start.X, Start.Y, Start.Z)->GetValuesAndMaterials(InValues, InMaterials, Start, StartIndex, Step, Size, ArraySize);
	}
	else
	{
		const int StartI = StartIndex.X;
		const int StartJ = StartIndex.Y;
		const int StartK = StartIndex.Z;

		const int StartX = Start.X;
		const int StartY = Start.Y;
		const int StartZ = Start.Z;

		const int RealSizeX = (Size.X - 1) * Step;
		const int RealSizeY = (Size.Y - 1) * Step;
		const int RealSizeZ = (Size.Z - 1) * Step;



		const int StartXBot = StartX;
		const int StartIBot = StartI;
		const int SizeXBot = (Position.X - StartX) / Step;
		const int StartXTop = StartXBot + SizeXBot * Step;
		const int StartITop = StartIBot + SizeXBot;
		const int SizeXTop = Size.X - SizeXBot;

		const int StartYBot = StartY;
		const int StartJBot = StartJ;
		const int SizeYBot = (Position.Y - StartY) / Step;
		const int StartYTop = StartYBot + SizeYBot * Step;
		const int StartJTop = StartJBot + SizeYBot;
		const int SizeYTop = Size.Y - SizeYBot;

		const int StartZBot = StartZ;
		const int StartKBot = StartK;
		const int SizeZBot = (Position.Z - StartZ) / Step;
		const int StartZTop = StartZBot + SizeZBot * Step;
		const int StartKTop = StartKBot + SizeZBot;
		const int SizeZTop = Size.Z - SizeZBot;


		if (StartX + RealSizeX < Position.X || Position.X <= StartX)
		{
			if (StartY + RealSizeY < Position.Y || Position.Y <= StartY)
			{
				if (StartZ + RealSizeZ < Position.Z || Position.Z <= StartZ)
				{
					GetChild(StartX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartX, StartY, StartZ },
						{ StartI, StartJ, StartK },
						Step,
						{ Size.X, Size.Y, Size.Z },
						ArraySize);
				}
				else
				{
					// Split Z

					GetChild(StartX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartX, StartY, StartZBot },
						{ StartI, StartJ, StartKBot },
						Step,
						{ Size.X, Size.Y, SizeZBot },
						ArraySize);

					GetChild(StartX, StartY, StartZ + RealSizeZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartX, StartY, StartZTop },
						{ StartI, StartJ, StartKTop },
						Step,
						{ Size.X, Size.Y, SizeZTop },
						ArraySize);
				}
			}
			else
			{
				// Split Y

				if (StartZ + RealSizeZ < Position.Z || Position.Z <= StartZ)
				{
					GetChild(StartX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartX, StartYBot, StartZ },
						{ StartI, StartJBot, StartK },
						Step,
						{ Size.X, SizeYBot, Size.Z },
						ArraySize);

					GetChild(StartX, StartY + RealSizeY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartX, StartYTop, StartZ },
						{ StartI, StartJTop, StartK },
						Step,
						{ Size.X, SizeYTop, Size.Z },
						ArraySize);
				}
				else
				{
					// Split Z

					GetChild(StartX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartX, StartYBot, StartZBot },
						{ StartI, StartJBot, StartKBot },
						Step,
						{ Size.X, SizeYBot, SizeZBot },
						ArraySize);

					GetChild(StartX, StartY, StartZ + RealSizeZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartX, StartYBot, StartZTop },
						{ StartI, StartJBot, StartKTop },
						Step,
						{ Size.X, SizeYBot, SizeZTop },
						ArraySize);


					GetChild(StartX, StartY + RealSizeY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartX, StartYTop, StartZBot },
						{ StartI, StartJTop, StartKBot },
						Step,
						{ Size.X, SizeYTop, SizeZBot },
						ArraySize);

					GetChild(StartX, StartY + RealSizeY, StartZ + RealSizeZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartX, StartYTop, StartZTop },
						{ StartI, StartJTop, StartKTop },
						Step,
						{ Size.X, SizeYTop, SizeZTop },
						ArraySize);
				}
			}
		}
		else
		{
			// Split X

			if (StartY + RealSizeY < Position.Y || Position.Y <= StartY)
			{
				if (StartZ + RealSizeZ < Position.Z || Position.Z <= StartZ)
				{
					GetChild(StartX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXBot, StartY, StartZ },
						{ StartIBot, StartJ, StartK },
						Step,
						{ SizeXBot, Size.Y, Size.Z },
						ArraySize);

					GetChild(StartX + RealSizeX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXTop, StartY, StartZ },
						{ StartITop, StartJ, StartK },
						Step,
						{ SizeXTop, Size.Y, Size.Z },
						ArraySize);
				}
				else
				{
					// Split Z

					GetChild(StartX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXBot, StartY, StartZBot },
						{ StartIBot, StartJ, StartKBot },
						Step,
						{ SizeXBot, Size.Y, SizeZBot },
						ArraySize);

					GetChild(StartX, StartY, StartZ + RealSizeZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXBot, StartY, StartZTop },
						{ StartIBot, StartJ, StartKTop },
						Step,
						{ SizeXBot, Size.Y, SizeZTop },
						ArraySize);


					GetChild(StartX + RealSizeX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXTop, StartY, StartZBot },
						{ StartITop, StartJ, StartKBot },
						Step,
						{ SizeXTop, Size.Y, SizeZBot },
						ArraySize);

					GetChild(StartX + RealSizeX, StartY, StartZ + RealSizeZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXTop, StartY, StartZTop },
						{ StartITop, StartJ, StartKTop },
						Step,
						{ SizeXTop, Size.Y, SizeZTop },
						ArraySize);
				}
			}
			else
			{
				// Split Y

				if (StartZ + RealSizeZ < Position.Z || Position.Z <= StartZ)
				{
					GetChild(StartX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXBot, StartYBot, StartZ },
						{ StartIBot, StartJBot, StartK },
						Step,
						{ SizeXBot, SizeYBot, Size.Z },
						ArraySize);

					GetChild(StartX, StartY + RealSizeY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXBot, StartYTop, StartZ },
						{ StartIBot, StartJTop, StartK },
						Step,
						{ SizeXBot, SizeYTop, Size.Z },
						ArraySize);

					GetChild(StartX + RealSizeX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXTop, StartYBot, StartZ },
						{ StartITop, StartJBot, StartK },
						Step,
						{ SizeXTop, SizeYBot, Size.Z },
						ArraySize);

					GetChild(StartX + RealSizeX, StartY + RealSizeY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXTop, StartYTop, StartZ },
						{ StartITop, StartJTop, StartK },
						Step,
						{ SizeXTop, SizeYTop, Size.Z },
						ArraySize);
				}
				else
				{
					// Split Z

					GetChild(StartX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXBot, StartYBot, StartZBot },
						{ StartIBot, StartJBot, StartKBot },
						Step,
						{ SizeXBot, SizeYBot, SizeZBot },
						ArraySize);

					GetChild(StartX, StartY, StartZ + RealSizeZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXBot, StartYBot, StartZTop },
						{ StartIBot, StartJBot, StartKTop },
						Step,
						{ SizeXBot, SizeYBot, SizeZTop },
						ArraySize);


					GetChild(StartX, StartY + RealSizeY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXBot, StartYTop, StartZBot },
						{ StartIBot, StartJTop, StartKBot },
						Step,
						{ SizeXBot, SizeYTop, SizeZBot },
						ArraySize);

					GetChild(StartX, StartY + RealSizeY, StartZ + RealSizeZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXBot, StartYTop, StartZTop },
						{ StartIBot, StartJTop, StartKTop },
						Step,
						{ SizeXBot, SizeYTop, SizeZTop },
						ArraySize);





					GetChild(StartX + RealSizeX, StartY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXTop, StartYBot, StartZBot },
						{ StartITop, StartJBot, StartKBot },
						Step,
						{ SizeXTop, SizeYBot, SizeZBot },
						ArraySize);

					GetChild(StartX + RealSizeX, StartY, StartZ + RealSizeZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXTop, StartYBot, StartZTop },
						{ StartITop, StartJBot, StartKTop },
						Step,
						{ SizeXTop, SizeYBot, SizeZTop },
						ArraySize);


					GetChild(StartX + RealSizeX, StartY + RealSizeY, StartZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXTop, StartYTop, StartZBot },
						{ StartITop, StartJTop, StartKBot },
						Step,
						{ SizeXTop, SizeYTop, SizeZBot },
						ArraySize);

					GetChild(StartX + RealSizeX, StartY + RealSizeY, StartZ + RealSizeZ)->GetValuesAndMaterials(InValues, InMaterials,
						{ StartXTop, StartYTop, StartZTop },
						{ StartITop, StartJTop, StartKTop },
						Step,
						{ SizeXTop, SizeYTop, SizeZTop },
						ArraySize);
				}
			}
		}
	}
}

void FValueOctree::SetValueAndMaterial(int X, int Y, int Z, float Value, FVoxelMaterial Material, bool bSetValue, bool bSetMaterial)
{
	check(IsLeaf());
	check(IsInOctree(X, Y, Z));

	bIsNetworkDirty = true;

	if (LOD != 0)
	{
		CreateChilds();
		GetChild(X, Y, Z)->SetValueAndMaterial(X, Y, Z, Value, Material, bSetValue, bSetMaterial);
	}
	else
	{
		if (!IsDirty())
		{
			SetAsDirtyAndSetDefaultValues();
		}

		int LocalX, LocalY, LocalZ;
		GlobalToLocal(X, Y, Z, LocalX, LocalY, LocalZ);

		uint32 Index = IndexFromCoordinates(LocalX, LocalY, LocalZ);
		if (bSetValue)
		{
			Values[Index] = Value;

			if (bMultiplayer)
			{
				DirtyValues.Add(Index);
			}
		}
		if (bSetMaterial)
		{
			Materials[Index] = Material;

			if (bMultiplayer)
			{
				DirtyMaterials.Add(Index);
			}
		}
	}
}

void FValueOctree::SetAsNotDirty()
{
	check(LOD == 0);

	bIsDirty = false;
	if (Values)
	{
		delete Values;
		Values = nullptr;
	}
	if (Materials)
	{
		delete Materials;
		Materials = nullptr;
	}
}


void FValueOctree::AddDirtyChunksToSaveQueue(TArray<FVoxelChunkSave>& SaveQueue)
{
	if (IsLeaf())
	{
		if (LOD == 0 && IsDirty())
		{
			SaveQueue.Add(FVoxelChunkSave(Id, Position, Values, Materials));
		}
	}
	else
	{
		for (auto Child : GetChilds())
		{
			Child->AddDirtyChunksToSaveQueue(SaveQueue);
		}
	}
}

void FValueOctree::LoadFromSaveQueueAndGetModifiedPositions(TArray<FVoxelChunkSave>& SaveQueue, TArray<FIntVector>& OutModifiedPositions)
{
	if (SaveQueue.Num() == 0)
	{
		return;
	}

	if (LOD == 0)
	{
		if (SaveQueue.Last().Id == Id)
		{
			bIsDirty = true;
			if (!Values)
			{
				Values = new float[DATA_CHUNK_TOTAL_SIZE];
			}
			if (!Materials)
			{
				Materials = new FVoxelMaterial[DATA_CHUNK_TOTAL_SIZE];
			}

			auto Last = SaveQueue.Pop(false);
			for (int X = 0; X < DATA_CHUNK_SIZE; X++)
			{
				for (int Y = 0; Y < DATA_CHUNK_SIZE; Y++)
				{
					for (int Z = 0; Z < DATA_CHUNK_SIZE; Z++)
					{
						const uint32 Index = IndexFromCoordinates(X, Y, Z);
						Values[Index] = Last.Values[Index];
						Materials[Index] = Last.Materials[Index];
					}
				}
			}

			// Update neighbors
			const int S = Size();
			OutModifiedPositions.Add(Position);

			OutModifiedPositions.Add(Position - FIntVector(S, 0, 0));
			OutModifiedPositions.Add(Position - FIntVector(0, S, 0));
			OutModifiedPositions.Add(Position - FIntVector(S, S, 0));
			OutModifiedPositions.Add(Position - FIntVector(0, 0, S));
			OutModifiedPositions.Add(Position - FIntVector(S, 0, S));
			OutModifiedPositions.Add(Position - FIntVector(0, S, S));
			OutModifiedPositions.Add(Position - FIntVector(S, S, S));
			
			OutModifiedPositions.Add(Position + FIntVector(S, 0, 0));
			OutModifiedPositions.Add(Position + FIntVector(0, S, 0));
			OutModifiedPositions.Add(Position + FIntVector(S, S, 0));
			OutModifiedPositions.Add(Position + FIntVector(0, 0, S));
			OutModifiedPositions.Add(Position + FIntVector(S, 0, S));
			OutModifiedPositions.Add(Position + FIntVector(0, S, S));
			OutModifiedPositions.Add(Position + FIntVector(S, S, S));
		}
	}
	else
	{
		if (IsIdChild(SaveQueue.Last().Id))
		{
			if (IsLeaf())
			{
				CreateChilds();
			}
			for (auto Child : GetChilds())
			{
				Child->LoadFromSaveQueueAndGetModifiedPositions(SaveQueue, OutModifiedPositions);
			}
		}
	}
}


void FValueOctree::CreateChilds()
{
	TVoxelOctree::CreateChilds();
	
}

void FValueOctree::SetAsDirtyAndSetDefaultValues()
{
	check(!IsDirty());
	check(LOD == 0);
	check(!Values);
	check(!Materials);

	Values = new float[DATA_CHUNK_TOTAL_SIZE];
	Materials = new FVoxelMaterial[DATA_CHUNK_TOTAL_SIZE];

	FIntVector Min = GetMinimalCornerPosition();
	GetValuesAndMaterials(Values, Materials, FIntVector(Min.X, Min.Y, Min.Z), FIntVector::ZeroValue, 1, FIntVector(DATA_CHUNK_SIZE, DATA_CHUNK_SIZE, DATA_CHUNK_SIZE), FIntVector(DATA_CHUNK_SIZE, DATA_CHUNK_SIZE, DATA_CHUNK_SIZE));

	bIsDirty = true;
}

uint32 FValueOctree::IndexFromCoordinates(int X, int Y, int Z) const
{
	check(0 <= X && X < DATA_CHUNK_SIZE);
	check(0 <= Y && Y < DATA_CHUNK_SIZE);
	check(0 <= Z && Z < DATA_CHUNK_SIZE);
	int Index = X + DATA_CHUNK_SIZE * Y + DATA_CHUNK_SIZE * DATA_CHUNK_SIZE * Z;
	return Index;
}

void FValueOctree::CoordinatesFromIndex(uint32 Index, int& OutX, int& OutY, int& OutZ) const
{
	check(0 <= Index && Index < DATA_CHUNK_TOTAL_SIZE);

	OutX = Index % DATA_CHUNK_SIZE;

	Index = (Index - OutX) / DATA_CHUNK_SIZE;
	OutY = Index % DATA_CHUNK_SIZE;

	Index = (Index - OutY) / DATA_CHUNK_SIZE;
	OutZ = Index;
}

void FValueOctree::GetPositionsToUpdate(TArray<FIntVector>& OutPositions)
{
	check(LOD == 0);

	if (IsDirty())
	{
		// With neighbors
		const int S = Size();
		OutPositions.Add(Position - FIntVector(0, 0, 0));
		OutPositions.Add(Position - FIntVector(S, 0, 0));
		OutPositions.Add(Position - FIntVector(0, S, 0));
		OutPositions.Add(Position - FIntVector(S, S, 0));
		OutPositions.Add(Position - FIntVector(0, 0, S));
		OutPositions.Add(Position - FIntVector(S, 0, S));
		OutPositions.Add(Position - FIntVector(0, S, S));
		OutPositions.Add(Position - FIntVector(S, S, S));
	}
}

void FValueOctree::SetWorldGenerator(TSharedRef<FVoxelWorldGeneratorInstance> NewGenerator)
{
	WorldGenerator = NewGenerator;

	if (!IsLeaf())
	{
		for (auto& Child : GetChilds())
		{
			Child->SetWorldGenerator(NewGenerator);
		}
	}
}

void FValueOctree::DiscardValuesByPredicate(const std::function<int(const FIntBox&)>& P)
{
	const int Result = P(GetBounds());

	if (Result == -1)
	{
		// Remove
		SetEntireChunkAsNotDirty();
	}
	else if (Result == 1)
	{
		// Keep
		return;
	}
	else
	{
		// Subdivide
		check(Result == 0);

		if (IsLeaf())
		{
			if (LOD == 0)
			{
				bool bNewChunk = false;
				if (!IsDirty())
				{
					bNewChunk = true;
					SetAsDirtyAndSetDefaultValues();
				}
				check(Values);
				check(Materials);

				for (int X = 0; X < DATA_CHUNK_SIZE; X++)
				{
					for (int Y = 0; Y < DATA_CHUNK_SIZE; Y++)
					{
						for (int Z = 0; Z < DATA_CHUNK_SIZE; Z++)
						{
							FIntVector GlobalPos;
							LocalToGlobal(X, Y, Z, GlobalPos.X, GlobalPos.Y, GlobalPos.Z);
							const FIntBox B(GlobalPos);
							const int LocalResult = P(B);

							if (LocalResult == -1)
							{
								if (!bNewChunk)
								{
									const int Index = IndexFromCoordinates(X, Y, Z);
									WorldGenerator->GetValueAndMaterial(GlobalPos.X, GlobalPos.Y, GlobalPos.Z, Values[Index], Materials[Index]);
								}
							}
							else if (LocalResult == 1)
							{
								continue;
							}
							else
							{
								// This is the smallest box that we can have
								check(false);
							}
						}
					}
				}
			}
		}
		else
		{
			for (auto& Child : GetChilds())
			{
				Child->DiscardValuesByPredicate(P);
			}
		}
	}
}

void FValueOctree::SetEntireChunkAsNotDirty()
{
	if (IsLeaf())
	{
		if (LOD == 0)
		{
			SetAsNotDirty();
		}
	}
	else
	{
		for (auto Child : GetChilds())
		{
			Child->SetEntireChunkAsNotDirty();
		}
	}
}

void FValueOctree::BeginSet(const FIntBox& Box, TArray<uint64>& Ids)
{
	if (GetBounds().Intersect(Box))
	{
		MainLock.lock();
		MainLock.unlock();

		check(GetCounter.GetValue() == 0);
		check(SetCounter.GetValue() == 0);
		
		if (IsLeaf())
		{
			// Lock
			MainLock.lock();
			SetCounter.Increment();
			Ids.Add(Id);

			// Unlock transactions
			TransactionLock.unlock();
		}
		else
		{
			// First lock childs
			for (auto Child : GetChilds())
			{
				Child->LockTransactions();
			}
			// Then unlock this
			TransactionLock.unlock();
			
			// Finally propagate to childs
			for (auto Child : GetChilds())
			{
				Child->BeginSet(Box, Ids);
			}
		}
	}
	else
	{		
		TransactionLock.unlock();
	}
}

void FValueOctree::EndSet(TArray<uint64>& Ids)
{
	if (Ids.Num() > 0)
	{
		if (Ids.Last() == Id)
		{
			Ids.Pop(false);
			SetCounter.Decrement();

			check(GetCounter.GetValue() == 0);
			check(SetCounter.GetValue() == 0);

			MainLock.unlock();
		}

		if (Ids.Num() > 0)
		{
			if (IsIdChild(Ids.Last()))
			{
				for (auto Child : GetChilds())
				{
					Child->EndSet(Ids);
				}
			}
		}
	}
}

void FValueOctree::BeginGet(const FIntBox& Box, TArray<uint64>& Ids)
{
	if (GetBounds().Intersect(Box))
	{
		MainLock.lock_shared();
		MainLock.unlock_shared();

		check(SetCounter.GetValue() == 0);

		if (IsLeaf())
		{
			// Lock
			MainLock.lock_shared();
			GetCounter.Increment();
			Ids.Add(Id);

			// Unlock transactions
			TransactionLock.unlock();
		}
		else
		{
			// First lock childs
			for (auto Child : GetChilds())
			{
				Child->LockTransactions();
			}
			// Then unlock this
			TransactionLock.unlock();

			// Finally propagate to childs
			for (auto Child : GetChilds())
			{
				Child->BeginGet(Box, Ids);
			}
		}
	}
	else
	{
		TransactionLock.unlock();
	}
}

void FValueOctree::EndGet(TArray<uint64>& Ids)
{
	if (Ids.Num() > 0)
	{
		if (Ids.Last() == Id)
		{
			Ids.Pop(false);
			GetCounter.Decrement();

			check(SetCounter.GetValue() == 0);

			MainLock.unlock_shared();
		}

		if (Ids.Num() > 0)
		{
			if (IsIdChild(Ids.Last()))
			{
				for (auto Child : GetChilds())
				{
					Child->EndGet(Ids);
				}
			}
		}
	}
}

void FValueOctree::LockTransactions()
{
	TransactionLock.lock();
}
