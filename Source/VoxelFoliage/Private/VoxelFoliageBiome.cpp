// Copyright 2021 Phyronnaz

#include "VoxelFoliageBiome.h"

#include "VoxelFoliage.h"

#if WITH_EDITOR
void UVoxelFoliageBiomeType::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		TSet<FName> Names;
		for (FVoxelFoliageBiomeTypeEntry& Entry : Entries)
		{
			while (Names.Contains(Entry.Name))
			{
				Entry.Name.SetNumber(Entry.Name.GetNumber() + 1);
			}
			Names.Add(Entry.Name);
		}
		
		for (FVoxelFoliageBiomeTypeEntry& Entry : Entries)
		{
			if (Entry.Seed == 0)
			{
				Entry.Seed = FMath::Rand();
			}
		}

		OnPostEditChangeProperty.Broadcast();
	}
}
#endif

#if WITH_EDITOR
bool UVoxelFoliageBiomeType::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const
{
	if (Object == this)
	{
		return true;
	}
	
	return false;
}
#endif

#if WITH_EDITOR
void UVoxelFoliageBiome::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	Fixup();
	BindDelegate();
}
#endif

void UVoxelFoliageBiome::PostInitProperties()
{
	Super::PostInitProperties();

	BindDelegate();
}

void UVoxelFoliageBiome::PostLoad()
{
	Super::PostLoad();
	
	BindDelegate();
}

void UVoxelFoliageBiome::Fixup()
{
	if (!Type)
	{
		return;
	}

	if (Entries.Num() == Type->Entries.Num())
	{
		for (int32 Index = 0; Index < Entries.Num(); Index++)
		{
			Entries[Index].Name = Type->Entries[Index].Name;
		}
	}
	else
	{
		TMap<FName, UVoxelFoliage*> Map;
		for (const FVoxelFoliageBiomeEntry& Entry : Entries)
		{
			Map.Add(Entry.Name, Entry.Foliage);
		}
		Entries.Reset();

		for (const FVoxelFoliageBiomeTypeEntry& Entry : Type->Entries)
		{
			UVoxelFoliage* Foliage = nullptr;
			Map.RemoveAndCopyValue(Entry.Name, Foliage);
			Entries.Add({ Entry.Name, Foliage });
		}

		for (auto& It : Map)
		{
			const FString Name = "STALE:" + It.Key.ToString();
			Entries.Add({ *Name, It.Value });
		}
	}
}

void UVoxelFoliageBiome::BindDelegate()
{
#if WITH_EDITOR
	if (Type)
	{
		Type->OnPostEditChangeProperty.RemoveAll(this);
		Type->OnPostEditChangeProperty.AddWeakLambda(this, [this, OldType = Type]()
		{
			if (Type == OldType)
			{
				Fixup();
			}
		});
	}
#endif
}

#if WITH_EDITOR
bool UVoxelFoliageBiome::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const
{
	if (Object == this)
	{
		return true;
	}
	
	if (Type && Type->NeedsToRebuild(Object, PropertyChangedEvent))
	{
		return true;
	}

	for (const FVoxelFoliageBiomeEntry& Entry : Entries)
	{
		if (Entry.Foliage && Entry.Foliage->NeedsToRebuild(Object, PropertyChangedEvent))
		{
			return true;
		}
	}

	return false;
}
#endif