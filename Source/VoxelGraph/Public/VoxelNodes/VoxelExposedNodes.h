// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelExposedNodes.generated.h"

UCLASS(Abstract, Category = "Parameters")
class VOXELGRAPH_API UVoxelExposedNode : public UVoxelNodeHelper
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	FString DisplayName;

	UPROPERTY(VisibleAnywhere, Category = "Parameter Settings")
	FName UniqueName;

	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	FString Category;

	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	FString Tooltip;

	// Lowest values on top
	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	int32 Priority;
	
	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	FString UIMin;
	
	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	FString UIMax;
	
	UPROPERTY(EditAnywhere, Category = "Parameter Settings")
	TMap<FName, FString> CustomMetaData;

	//~ Begin UVoxelExposedNode Interface
	virtual FName GetParameterPropertyName() const { ensure(false); return {}; }
	virtual TMap<FName, FString> GetMetaData() const;
	//~ End UVoxelExposedNode Interface

	//~ Begin UVoxelNode Interface
	virtual FLinearColor GetColor() const override;
	virtual FText GetTitle() const override;
	virtual bool CanRenameNode() const override;
	virtual FString GetEditableName() const override;
	virtual void SetEditableName(const FString& NewName) override;
	virtual void ApplyParameters(const TMap<FName, FString>& Parameters) override;
	virtual void GetParameters(TArray<FVoxelGeneratorParameter>& OutParameters) const override;
	//~ End UVoxelNode Interface

public:
	struct FDummy
	{
		static UScriptStruct* Get() { return nullptr; }
	};
	
	template<typename T>
	T GetParameter() const
	{
		T Temp{};
		const void* Result = GetParameterInternal(static_cast<void*>(&Temp), TChooseClass<TOr<TIsFundamentalType<T>, TIsPointer<T>>::Value, FDummy, TBaseStructure<T>>::Result::Get());
		check(Result);
		
		return *static_cast<const T*>(Result);
	}

protected:
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostEditImport() override;
	virtual void PostLoad() override;
	//~ End UObject Interface

private:
	// Only allow renaming on creation, else the name is wrong (GetTitle never called)
	UPROPERTY()
	bool bCanBeRenamed = true;

	void MakeNameUnique();
	const void* GetParameterInternal(void* Temp, UScriptStruct* Struct) const;
};