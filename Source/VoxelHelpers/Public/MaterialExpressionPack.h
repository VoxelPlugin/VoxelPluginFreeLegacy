// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionPack.generated.h"

USTRUCT()
struct FMaterialPackInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=PackInput)
	FName InputName = "DEFAULT_DO_NOT_USE"; // Assign a default value to bypass copy paste bug when items are equal to default

	UPROPERTY()
	FExpressionInput Input;
};

UCLASS(CollapseCategories, HideCategories = Object)
class UMaterialExpressionPack : public UMaterialExpression
{
	GENERATED_BODY()
		
public:
	UMaterialExpressionPack();

	UPROPERTY(EditAnywhere, Category=MaterialExpressionPack)
	TArray<FMaterialPackInput> Inputs;

	FSimpleMulticastDelegate OnPostEditChangeProperty;
	
#if WITH_EDITOR
	//~ Begin UObject Interface.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface.

	//~ Begin UMaterialExpression Interface
	virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
	virtual const TArray<FExpressionInput*> GetInputs() override;
	virtual FExpressionInput* GetInput(int32 InputIndex) override;
	virtual FName GetInputName(int32 InputIndex) const override;
	virtual uint32 GetInputType(int32 InputIndex) override { return MCT_Unknown; }
	virtual uint32 GetOutputType(int32 OutputIndex) override { return MCT_Unknown; }
	virtual bool IsResultMaterialAttributes(int32 OutputIndex) override { return true; }
	//~ End UMaterialExpression Interface
#endif
};

UCLASS(CollapseCategories, HideCategories = Object)
class UMaterialExpressionUnpack : public UMaterialExpression
{
	GENERATED_BODY()
		
public:
	UMaterialExpressionUnpack();

	UPROPERTY()
	FExpressionInput Input;

	UPROPERTY(EditAnywhere, Category=MaterialExpressionUnpack)
	bool bRefresh = false;
	
#if WITH_EDITOR
	//~ Begin UObject Interface.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface.

	//~ Begin UMaterialExpression Interface
	virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
	virtual uint32 GetInputType(int32 InputIndex) override { return MCT_Unknown; }
	virtual uint32 GetOutputType(int32 OutputIndex) override { return MCT_Unknown; }
	//~ End UMaterialExpression Interface
#endif

	void RefreshPack();
	UMaterialExpressionPack* GetPack_NotForCompile() const;
	bool MatchesPack(UMaterialExpressionPack* Pack);
};

struct FMaterialExpressionUnpackScope
{
	struct FElement
	{
		UMaterialExpressionUnpack* Unpack = nullptr;
		int32 OutputIndex = 0;
	};
	
	FMaterialExpressionUnpackScope(UMaterialExpressionUnpack* Unpack, int32 OutputIndex);
	~FMaterialExpressionUnpackScope();

	static FElement* GetQueuedElement()
	{
		return QueuedElement.Get();
	}
	static FElement Pop()
	{
		check(QueuedElement.IsValid());
		const auto Copy = *QueuedElement;
		QueuedElement.Reset();
		return Copy;
	}

private:
	static TUniquePtr<FElement> QueuedElement;
};