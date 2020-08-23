// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelObjectArchive.generated.h"

USTRUCT()
struct FVoxelObjectArchiveEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Entry")
	TSoftObjectPtr<UObject> Object;
	
	// Zero is reserved for nullptr
	UPROPERTY(VisibleAnywhere, Category = "Entry")
	int32 Index = -1;
};

// Reader must be called from the game thread
// Writer can be called async, since it's working with soft object pointers
class VOXEL_API FVoxelObjectArchive
{	
public:
	static FVoxelObjectArchive MakeReader(FArchive& Ar, const TArray<FVoxelObjectArchiveEntry>& Objects);
	static FVoxelObjectArchive MakeWriter(FArchive& Ar);

	TArray<FVoxelObjectArchiveEntry> GetWriterObjects() const;

	bool IsSaving() const { return bIsSaving; }

private:
	FVoxelObjectArchive(FArchive& Ar, bool bIsSaving)
		: Ar(Ar)
		, bIsSaving(bIsSaving)
	{
		check(bIsSaving ? Ar.IsSaving() : Ar.IsLoading());
	}
	
public:
	template<typename T>
	FVoxelObjectArchive& operator<<(T& Object)
	{
		static_assert(!TIsConst<T>::Value, "");
		Ar << Object;
		return *this;
	}

	template<typename T>
	FVoxelObjectArchive& operator<<(T*& Object) = delete;
	
	template<typename T>
	FVoxelObjectArchive& operator<<(TSoftObjectPtr<T>& Object)
	{
		static_assert(TIsDerivedFrom<T, UObject>::IsDerived, "");

		if (IsSaving())
		{
			int32 ObjectIndex;
			if (Object.IsNull())
			{
				ObjectIndex = 0;
			}
			else
			{
				if (auto* ObjectIndexPtr = WriterObjects.Find(Object))
				{
					ObjectIndex = *ObjectIndexPtr;
				}
				else
				{
					ObjectIndex = ObjectCounter++;
					WriterObjects.Add(Object, ObjectIndex);
				}
			}

			Ar << ObjectIndex;
		}
		else
		{
			int32 ObjectIndex = -1;
			Ar << ObjectIndex;

			if (ObjectIndex == 0)
			{
				Object = nullptr;
				return *this;
			}
			else
			{
				TSoftObjectPtr<UObject>* FoundObjectPtr = ReaderObjects.Find(ObjectIndex);
				if (FoundObjectPtr)
				{
					UObject* FoundObject = FoundObjectPtr->LoadSynchronous();
					if (ensure(FoundObject) && ensure(FoundObject->IsA<T>()))
					{
						Object = CastChecked<T>(FoundObject);
						return *this;
					}
				}
			}

			ensure(false);

			Object = nullptr;
			Ar.SetError();
		}
		
		return *this;
	}

private:	
	FArchive& Ar;
	
	const bool bIsSaving;
	TMap<TSoftObjectPtr<UObject>, int32> WriterObjects;
	TMap<int32, TSoftObjectPtr<UObject>> ReaderObjects;

	// Zero is reserved for nullptr
	int32 ObjectCounter = 1;
};