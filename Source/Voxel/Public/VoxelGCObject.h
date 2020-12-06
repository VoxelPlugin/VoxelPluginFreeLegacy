// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"

class FVoxelGCObject
{
public:
	virtual ~FVoxelGCObject() = default;

	//~ Begin FVoxelGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) = 0;
	virtual FString GetReferencerName() const = 0;
	//~ End FVoxelGCObject Interface

	void DestroyGCObject()
	{
		ensure(Impl);
		Impl.Reset();
	}

private:
	class FImpl : public FGCObject
	{
	public:
		FVoxelGCObject& Owner;
		
		explicit FImpl(FVoxelGCObject& Owner)
			: Owner(Owner)
		{
		}
		
		//~ Begin FGCObject Interface
		virtual void AddReferencedObjects(FReferenceCollector& Collector) override
		{
			return Owner.AddReferencedObjects(Collector);
		}
		virtual FString GetReferencerName() const override
		{
			return Owner.GetReferencerName();
		}
		//~ End FGCObject Interface
	};
	TUniquePtr<FImpl> Impl = MakeUnique<FImpl>(*this);
};