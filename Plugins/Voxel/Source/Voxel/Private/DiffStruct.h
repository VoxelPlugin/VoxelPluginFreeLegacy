#pragma once
#include "CoreMinimal.h"
#include <forward_list>
#include "DiffStruct.generated.h"

USTRUCT()
struct FVoxelValueDiff
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint32 Id;

	UPROPERTY(EditAnywhere)
		int Index;

	UPROPERTY(EditAnywhere)
		float Value;

	FVoxelValueDiff() : Id(-1), Index(-1), Value(0)
	{
	};

	FVoxelValueDiff(uint32 Id, int Index, float Value) : Id(Id), Index(Index), Value(Value)
	{
	};
};

USTRUCT()
struct FVoxelColorDiff
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint32 Id;

	UPROPERTY(EditAnywhere)
		int Index;

	UPROPERTY(EditAnywhere)
		FColor Color;

	FVoxelColorDiff() : Id(-1), Index(-1), Color(FColor::Black)
	{
	};

	FVoxelColorDiff(uint32 Id, int Index, FColor Color) : Id(Id), Index(Index), Color(Color)
	{
	};
};

struct VoxelValueDiffArray
{
	std::forward_list<uint32> Ids;
	std::forward_list<int> Indexes;
	std::forward_list<float> Values;
	int Size = 0;

	void Add(uint32 Id, int Index, float Value)
	{
		Ids.push_front(Id);
		Indexes.push_front(Index);
		Values.push_front(Value);
		Size++;
	}

	void AddPackets(std::forward_list<TArray<FVoxelValueDiff>>& List, const int MaxSize = 2048)
	{
		const int MaxLength = MaxSize / 3;

		while (!Indexes.empty())
		{
			const int Count = FMath::Min(Size, MaxLength);

			TArray<FVoxelValueDiff> DiffArray;
			DiffArray.SetNumUninitialized(Count);

			for (int i = 0; i < Count; i++)
			{
				if (Ids.empty() || Indexes.empty() || Values.empty())
				{
					break;
				}

				DiffArray[Count - 1 - i] = *new FVoxelValueDiff(Ids.front(), Indexes.front(), Values.front());

				Ids.pop_front();
				Indexes.pop_front();
				Values.pop_front();
				Size--;
			}

			List.push_front(DiffArray);
		}
	}
};

struct VoxelColorDiffArray
{
	std::forward_list<uint32> Ids;
	std::forward_list<int> Indexes;
	std::forward_list<FColor> Colors;
	int Size = 0;

	void Add(uint32 Id, int Index, FColor Color)
	{
		Ids.push_front(Id);
		Indexes.push_front(Index);
		Colors.push_front(Color);
		Size++;
	}

	void AddPackets(std::forward_list<TArray<FVoxelColorDiff>>& List, const int MaxSize = 2048)
	{
		const int MaxLength = MaxSize / 3;

		while (!Indexes.empty())
		{
			const int Count = FMath::Min(Size, MaxLength);

			TArray<FVoxelColorDiff> DiffArray;
			DiffArray.SetNumUninitialized(Count);

			for (int i = 0; i < Count; i++)
			{
				if (Ids.empty() || Indexes.empty() || Colors.empty())
				{
					break;
				}

				DiffArray[Count - 1 - i] = *new FVoxelColorDiff(Ids.front(), Indexes.front(), Colors.front());

				Ids.pop_front();
				Indexes.pop_front();
				Colors.pop_front();
				Size--;
			}

			List.push_front(DiffArray);
		}
	}
};