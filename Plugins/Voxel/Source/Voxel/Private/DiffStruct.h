#pragma once
#include "CoreMinimal.h"
#include <forward_list>
#include "DiffStruct.generated.h"

USTRUCT()
struct FSingleDiffStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint32 Id;

	UPROPERTY(EditAnywhere)
		int Index;

	UPROPERTY(EditAnywhere)
		float Value;

	FSingleDiffStruct() : Id(-1), Index(-1), Value(0)
	{
	};

	FSingleDiffStruct(uint32 Id, int Index, float Value) : Id(Id), Index(Index), Value(Value)
	{
	};
};

struct DiffSaveStruct
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

	std::forward_list<TArray<FSingleDiffStruct>> GetPackets(const int MaxSize = 2048)
	{
		const int MaxLength = MaxSize / 3;

		std::forward_list<TArray<FSingleDiffStruct>> ReturnList;

		while (!Indexes.empty())
		{
			const int Count = FMath::Min(Size, MaxLength);

			TArray<FSingleDiffStruct> DiffArray;
			DiffArray.SetNumUninitialized(Count);

			for (int i = 0; i < Count; i++)
			{
				if (Ids.empty() || Indexes.empty() || Values.empty())
				{
					break;
				}

				DiffArray[Count - 1 - i] = *new FSingleDiffStruct(Ids.front(), Indexes.front(), Values.front());

				Ids.pop_front();
				Indexes.pop_front();
				Values.pop_front();
				Size--;
			}

			ReturnList.push_front(DiffArray);
		}

		return ReturnList;
	}
};