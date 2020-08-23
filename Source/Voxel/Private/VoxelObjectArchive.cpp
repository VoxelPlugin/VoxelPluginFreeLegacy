// Copyright 2020 Phyronnaz

#include "VoxelObjectArchive.h"
#include "VoxelMinimal.h"

FVoxelObjectArchive FVoxelObjectArchive::MakeReader(FArchive& Ar, const TArray<FVoxelObjectArchiveEntry>& Objects)
{
	check(IsInGameThread());
	
	FVoxelObjectArchive Result(Ar, false);
	Result.ReaderObjects.Reserve(Objects.Num());
	for (auto& Object : Objects)
	{
		if (Object.Index == 0)
		{
			ensure(!Object.Object.IsValid());
			continue;
		}

		UObject* ObjectPtr = Object.Object.Get();
		if (!ensure(ObjectPtr))
		{
			LOG_VOXEL(Error, TEXT("Failed to load %s"), *Object.Object.ToString());
		}
		Result.ReaderObjects.Add(Object.Index, ObjectPtr);
	}
	return Result;
}

FVoxelObjectArchive FVoxelObjectArchive::MakeWriter(FArchive& Ar)
{
	return FVoxelObjectArchive(Ar, true);
}

TArray<FVoxelObjectArchiveEntry> FVoxelObjectArchive::GetWriterObjects() const
{
	check(IsSaving());

	TArray<FVoxelObjectArchiveEntry> Objects;
	Objects.Reserve(WriterObjects.Num());
	for (auto& It : WriterObjects)
	{
		const TSoftObjectPtr<UObject>& Object = It.Key;
		const int32 Index = It.Value;

		if (Index == 0)
		{
			ensure(Object.IsNull());
			Objects.Add(FVoxelObjectArchiveEntry{ {}, 0 });
		}
		else
		{
			ensure(!Object.IsNull());
			Objects.Add(FVoxelObjectArchiveEntry{ Object, Index });
		}
	}

	return Objects;
}