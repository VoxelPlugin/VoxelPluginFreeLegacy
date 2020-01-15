// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNode.h"
#include "VoxelLocalVariables.generated.h"

class UVoxelLocalVariableDeclaration;

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelLocalVariableBase : public UVoxelNode
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelNode Interface
	virtual FLinearColor GetColor() const override { return FColorList::BakerChocolate; }
	//~ End UVoxelNode Interface

protected:
	/**
	 * Find a variable declaration in an array of expressions
	 * @param	VariableGuid	The GUID of the variable to find
	 * @param	Nodes			The nodes to search in
	 * @return	null if not found
	 */
	 UVoxelLocalVariableDeclaration* FindDeclarationInArray(const FGuid& VariableGuid, const TArray<UVoxelNode*>& Nodes) const;
	/**
	 * Find a variable declaration in the entire graph
	 * @param	VariableGuid	The GUID of the variable to find
	 * @return	null if not found
	 */
	UVoxelLocalVariableDeclaration* FindDeclarationInGraph(const FGuid& VariableGuid) const;
};

UENUM()
enum class EVoxelPortalNodePinCategory : uint8
{
	Boolean,
	Int,
	Float,
	Material,
	Color,
	Seed
};

USTRUCT()
struct VOXELGRAPH_API FVoxelPortalNodeSelector
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UVoxelLocalVariableDeclaration> Input;
};

UCLASS(NotPlaceable)
class VOXELGRAPH_API UVoxelLocalVariableDeclaration : public UVoxelLocalVariableBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FName Name = TEXT("Name");

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelPortalNodePinCategory Category = EVoxelPortalNodePinCategory::Float;

	// The variable GUID, to support copy across graphs
	UPROPERTY()
	FGuid VariableGuid;

	EVoxelPinCategory GetCategory() const;
	void SetCategory(EVoxelPinCategory NewCategory);

public:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	//~ End UObject Interface

	//~ Begin UVoxelNode Interface
	virtual int32 GetMinInputPins() const override { return 1; }
	virtual int32 GetMaxInputPins() const override { return 1; }
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual FText GetTitle() const override { return FText::FromName(Name); }
	virtual void PostCopyNode(const TArray<UVoxelNode*>& CopiedNodes) override;
	virtual bool CanRenameNode() const override { return true; }
	virtual FString GetEditableName() const override;
	virtual void SetEditableName(const FString& NewName) override;
	//~ End UVoxelNode Interface

private:
	/**
	* Generates a GUID for the variable if one doesn't already exist
	* @param bForceGeneration	Whether we should generate a GUID even if it is already valid.
	*/
	void UpdateVariableGuid(bool bForceGeneration, bool bAllowMarkingPackageDirty);
	void MakeNameUnique();
};

UCLASS(NotPlaceable)
class VOXELGRAPH_API UVoxelLocalVariableUsage : public UVoxelLocalVariableBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVoxelPortalNodeSelector Selector_DEPRECATED;
	
	// The declaration this node is linked to
	UPROPERTY()
	UVoxelLocalVariableDeclaration* Declaration;
	
	// The variable GUID, to support copy across graphs
	UPROPERTY()
	FGuid DeclarationGuid;

public:
	//~ Begin UObject Interface
	virtual void PostLoad() override;
	//~ End UObject Interface

	//~ Begin UVoxelNode Interface
	virtual int32 GetOutputPinsCount() const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;

	virtual FText GetTitle() const override;

	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
	virtual void PostCopyNode(const TArray<UVoxelNode*>& CopiedNodes) override;
	//~ End UVoxelNode Interface

private:
	// Check that the declaration isn't deleted
	bool IsDeclarationValid() const;
};