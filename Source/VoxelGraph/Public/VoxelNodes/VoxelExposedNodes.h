// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelExposedNodes.generated.h"

UCLASS(Abstract, Category = "Parameters")
class VOXELGRAPH_API UVoxelExposedNode : public UVoxelNodeHelper
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Parameter Settings", meta = (DisplayName = "Name"))
	FName UniqueName = "";

	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	FString Category;

	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	FString Tooltip;

	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	int32 Priority;

	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	TMap<FName, FString> CustomMetaData = { {"UIMin", ""}, {"UIMax", ""} };

	//~ Begin UVoxelExposedNode Interface
#if WITH_EDITOR
	virtual bool TryImportFromProperty(UProperty* Property, UObject* Object);
#endif
	//~ End UVoxelExposedNode Interface

	//~ Begin UVoxelNode Interface
	virtual FLinearColor GetColor() const override;
	virtual FText GetTitle() const override;
	virtual bool CanRenameNode() const override;
	virtual FString GetEditableName() const override;
	virtual void SetEditableName(const FString& NewName) override;
	//~ End UVoxelNode Interface

protected:
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostEditImport() override;
	//~ End UObject Interface

	template<typename T>
	static bool TryImportObject(UProperty* Property, UObject* Object, T*& NodeAsset)
	{
		if (auto* ObjectProp = UE_25_SWITCH(Cast, CastField)<USoftObjectProperty>(Property))
		{
			auto* AssetPtr = ObjectProp->ContainerPtrToValuePtr<const TSoftObjectPtr<UObject>>(Object);
			if (AssetPtr)
			{
				auto* Asset = AssetPtr->LoadSynchronous();
				if (!Asset || Asset->IsA<T>())
				{
					NodeAsset = Cast<T>(Asset);
					return true;
				}
			}
		}
		return false;
	}

private:
	// Only allow renaming on creation, else the name is wrong (GetTitle never called)
	UPROPERTY()
	bool bCanBeRenamed = true;

	void MakeNameUnique();
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelOptionallyExposedNode : public UVoxelExposedNode
{
	GENERATED_BODY()

public:
	// If false, this node is a constant
	// If true, you can change it to configure your graph
	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	bool bExposeToBP;
	
	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	virtual FLinearColor GetColor()	const override;
	virtual void SetEditableName(const FString& NewName) override;
	//~ End UVoxelNode Interface

	//~ Begin UVoxelOptionallyExposedNode Interface
	virtual FString GetValueString() const { unimplemented(); return {}; }
	virtual FLinearColor GetNotExposedColor() const { unimplemented(); return FColor::Black; }
	//~ End UVoxelOptionallyExposedNode Interface
};