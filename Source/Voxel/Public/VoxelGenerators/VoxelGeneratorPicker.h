// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Templates/SubclassOf.h"
#include "VoxelGenerators/VoxelGenerator.h"
#include "VoxelGeneratorPicker.generated.h"

class UVoxelGeneratorInstanceWrapper;
class UVoxelTransformableGeneratorInstanceWrapper;

UENUM(BlueprintType)
enum class EVoxelGeneratorPickerType : uint8
{
	Class,
	Object
};

template<typename TGenerator, bool bWeakPtr>
struct TVoxelGeneratorPicker
{
public:
	static constexpr bool bIsTransformable = TIsSame<TGenerator, UVoxelTransformableGenerator>::Value;
	
	using UGenerator = TGenerator;
	using FInstance = typename TChooseClass<bIsTransformable, FVoxelTransformableGeneratorInstance, FVoxelGeneratorInstance>::Result;
	using UWrapper = typename TChooseClass<bIsTransformable, UVoxelTransformableGeneratorInstanceWrapper, UVoxelGeneratorInstanceWrapper>::Result;

	template<typename T>
	using TPtr = typename TChooseClass<bWeakPtr, TWeakObjectPtr<T>, T*>::Result;

public:
	TVoxelGeneratorPicker() = default;
	TVoxelGeneratorPicker(TYPE_OF_NULLPTR) {}
	TVoxelGeneratorPicker(UClass* InClass)
	{
		Type = EVoxelGeneratorPickerType::Class;
		Class = InClass;
	}
	TVoxelGeneratorPicker(TSubclassOf<UGenerator> InClass)
		: TVoxelGeneratorPicker(InClass.Get())
	{
	}
	TVoxelGeneratorPicker(UGenerator* InObject)
	{
		Type = EVoxelGeneratorPickerType::Object;
		Object = InObject;
	}
	TVoxelGeneratorPicker(TSoftClassPtr<UGenerator> InClass)
		: TVoxelGeneratorPicker(InClass.LoadSynchronous())
	{
	}
	TVoxelGeneratorPicker(TSoftObjectPtr<UGenerator> InObject)
		: TVoxelGeneratorPicker(InObject.LoadSynchronous())
	{
	}
	
public:
	template<typename TOther, bool bOtherWeakPtr>
	TVoxelGeneratorPicker(const TVoxelGeneratorPicker<TOther, bOtherWeakPtr>& Picker)
	{
		// TWeakObjectPtr casts: so we support bWeakPtr being false or true
		
		Type = Picker.Type;
		Class = Picker.Class.Get();
		Object = Cast<UGenerator>(TWeakObjectPtr<TOther>(Picker.Object).Get());
		Parameters = Picker.Parameters;
#if WITH_EDITORONLY_DATA
		EditorData = TWeakObjectPtr<UObject>(Picker.EditorData).Get();
#endif
	}
	
public:
	EVoxelGeneratorPickerType Type = EVoxelGeneratorPickerType::Class;
	TSubclassOf<UGenerator> Class;
	TPtr<UGenerator> Object = nullptr;
	TMap<FName, FString> Parameters;
#if WITH_EDITORONLY_DATA
	TPtr<UObject> EditorData = nullptr;
#endif
	
	// Might return nullptr!
	UGenerator* GetGenerator() const
	{
		if (Type == EVoxelGeneratorPickerType::Class)
		{
			return Class ? Class->template GetDefaultObject<UGenerator>() : nullptr;
		}
		else
		{
			return Object;
		}
	}
	UObject* GetObject() const
	{
		if (Type == EVoxelGeneratorPickerType::Class)
		{
			return Class;
		}
		else
		{
			return Object;
		}
	}
	
	bool IsValid() const { return GetObject() != nullptr; }
	bool IsClass() const { return Type == EVoxelGeneratorPickerType::Class; }
	bool IsObject() const { return Type == EVoxelGeneratorPickerType::Object; }

	bool operator==(const TVoxelGeneratorPicker& Other) const
	{
		// We ignore editor data here
		return
			Type == Other.Type &&
			(Type == EVoxelGeneratorPickerType::Class ? (Class == Other.Class) : (Object == Other.Object)) &&
			Parameters.OrderIndependentCompareEqual(Other.Parameters);
	}
};

template<typename TGenerator, bool bWeakPtr>
uint32 GetTypeHash(const TVoxelGeneratorPicker<TGenerator, bWeakPtr>& Key)
{
	return HashCombine(GetTypeHash(Key.GetObject()), GetTypeHash(Key.Parameters.Num()));
}

struct FVoxelWeakGeneratorPicker : TVoxelGeneratorPicker<UVoxelGenerator, true>
{
	using TVoxelGeneratorPicker<UVoxelGenerator, true>::TVoxelGeneratorPicker;
};

struct FVoxelWeakTransformableGeneratorPicker : TVoxelGeneratorPicker<UVoxelTransformableGenerator, true>
{
	using TVoxelGeneratorPicker<UVoxelTransformableGenerator, true>::TVoxelGeneratorPicker;
};

USTRUCT(BlueprintType, meta=(HasNativeMake="Voxel.VoxelGeneratorTools.MakeGeneratorPickerFromObject"))
struct VOXEL_API FVoxelGeneratorPicker
#if CPP
	: TVoxelGeneratorPicker<UVoxelGenerator, false>
#endif
{
	GENERATED_BODY()
	
	using TVoxelGeneratorPicker<UVoxelGenerator, false>::TVoxelGeneratorPicker;
	
	// Will default to EmptyGenerator if null
	TVoxelSharedRef<FInstance> GetInstance() const;
	
#if !CPP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelGeneratorPickerType Type = EVoxelGeneratorPickerType::Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TSubclassOf<UVoxelGenerator> Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (DisallowedClasses = "VoxelGraphMacro"))
	UVoxelGenerator* Object = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TMap<FName, FString> Parameters;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	UObject* EditorData = nullptr;
#endif
#endif
};

USTRUCT(BlueprintType, meta=(HasNativeMake="Voxel.VoxelGeneratorTools.MakeTransformableGeneratorPickerFromObject"))
struct FVoxelTransformableGeneratorPicker
#if CPP
	: TVoxelGeneratorPicker<UVoxelTransformableGenerator, false>
#endif
{
	GENERATED_BODY()
	
	using TVoxelGeneratorPicker<UVoxelTransformableGenerator, false>::TVoxelGeneratorPicker;
	
	// Will default to EmptyGenerator if null
	TVoxelSharedRef<FInstance> GetInstance() const;

#if !CPP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelGeneratorPickerType Type = EVoxelGeneratorPickerType::Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TSubclassOf<UVoxelTransformableGenerator> Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (DisallowedClasses = "VoxelGraphMacro"))
	UVoxelTransformableGenerator* Object = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TMap<FName, FString> Parameters;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	UObject* EditorData = nullptr;
#endif
#endif
};

VOXEL_DEPRECATED(1.2, "Use FVoxelGeneratorPicker instead of FVoxelWorldGeneratorPicker")
typedef FVoxelGeneratorPicker FVoxelWorldGeneratorPicker;

VOXEL_DEPRECATED(1.2, "Use FVoxelTransformableGeneratorPicker instead of FVoxelTransformableWorldGeneratorPicker")
typedef FVoxelTransformableGeneratorPicker FVoxelTransformableWorldGeneratorPicker;