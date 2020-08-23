// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Templates/SubclassOf.h"
#include "Framework/Commands/InputChord.h"
#include "VoxelGraphShortcuts.generated.h"

class UVoxelNode;

USTRUCT()
struct FVoxelGraphEditorKeyBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bCtrlDown = false;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bAltDown = false;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bShiftDown = false;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FKey Key;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSubclassOf<UVoxelNode> Class;

	FVoxelGraphEditorKeyBinding() = default;

	FVoxelGraphEditorKeyBinding(FKey Key, TSubclassOf<UVoxelNode> Class)
		: Key(Key)
		, Class(Class)
	{
	}

	inline bool IsSameAs(const FInputChord& Chord)
	{
		return bCtrlDown == Chord.bCtrl && bAltDown == Chord.bAlt && bShiftDown == Chord.bShift && Key == Chord.Key;
	}
};

UCLASS(Config = EditorKeyBindings)
class UVoxelGraphShortcuts : public UObject
{
	GENERATED_BODY()

public:
	UVoxelGraphShortcuts();

	UPROPERTY(Config, EditAnywhere, Category = "Voxel")
	TArray<FVoxelGraphEditorKeyBinding> Shortcuts;
	
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};