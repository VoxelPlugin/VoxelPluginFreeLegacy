// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelSpawnerGroup.h"
#include "VoxelMessages.h"

#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"


#if WITH_EDITOR
void UVoxelSpawnerGroup::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (bNormalizeProbabilitiesOnEdit &&
		PropertyChangedEvent.Property &&
		(PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive ||
			PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet))
	{
		const int32 EditedIndex = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_STRING_CHECKED(UVoxelSpawnerGroup, Children));
		if (Children.IsValidIndex(EditedIndex))
		{
			double Sum = 0;
			for (int32 Index = 0; Index < Children.Num(); Index++)
			{
				if (Index != EditedIndex)
				{
					Sum += Children[Index].Probability;
				}
			}
			if (Sum == 0)
			{
				for (int32 Index = 0; Index < Children.Num(); Index++)
				{
					if (Index != EditedIndex)
					{
						ensure(Children[Index].Probability == 0);
						Children[Index].Probability = (1 - Children[EditedIndex].Probability) / (Children.Num() - 1);
					}
				}
			}
			else
			{
				for (int32 Index = 0; Index < Children.Num(); Index++)
				{
					if (Index != EditedIndex)
					{
						Children[Index].Probability *= (1 - Children[EditedIndex].Probability) / Sum;
					}
				}
			}
		}
	}
}
#endif