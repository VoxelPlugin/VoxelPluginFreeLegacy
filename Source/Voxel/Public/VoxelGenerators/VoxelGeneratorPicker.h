// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Templates/SubclassOf.h"
#include "VoxelGenerators/VoxelGenerator.h"
#include "VoxelGeneratorPicker.generated.h"

UENUM(BlueprintType)
enum class EVoxelGeneratorPickerType : uint8
{
	Class,
	Object
};

template<typename TGenerator>
struct TVoxelGeneratorPicker
{
public:
	using UGenerator = TGenerator;
#if VOXEL_ENGINE_VERSION >= 504
	using FGeneratorInstance = typename std::conditional_t<std::is_same_v<UGenerator, UVoxelGenerator>, FVoxelGeneratorInstance, FVoxelTransformableGeneratorInstance>;
#else
	using FGeneratorInstance = typename TChooseClass<std::is_same_v<UGenerator, UVoxelGenerator>, FVoxelGeneratorInstance, FVoxelTransformableGeneratorInstance>::Result;
#endif
	
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

	template<typename TOther>
	TVoxelGeneratorPicker(TVoxelGeneratorPicker<TOther> Picker)
	{
		Type = Picker.Type;
		Class = Picker.Class.Get();
		Object = Cast<UGenerator>(Picker.Object);
	}
	
public:
	EVoxelGeneratorPickerType Type = EVoxelGeneratorPickerType::Class;
	TSubclassOf<UGenerator> Class;
	TObjectPtr<UGenerator> Object = nullptr;
	TMap<FName, FString> Parameters;
#if WITH_EDITORONLY_DATA
	TObjectPtr<UObject> EditorData = nullptr;
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

	bool operator==(const TVoxelGeneratorPicker<UGenerator>& Other) const
	{
		// We ignore editor data here
		return
			Type == Other.Type &&
			(Type == EVoxelGeneratorPickerType::Class ? (Class == Other.Class) : (Object == Other.Object)) &&
			Parameters.OrderIndependentCompareEqual(Other.Parameters);
	}
};

template<typename TGenerator>
uint32 GetTypeHash(const TVoxelGeneratorPicker<TGenerator>& Key)
{
	return HashCombine(GetTypeHash(Key.GetObject()), GetTypeHash(Key.Parameters.Num()));
}

USTRUCT(BlueprintType, meta=(HasNativeMake="/Script/Voxel.VoxelGeneratorTools:MakeGeneratorPickerFromObject"))
struct VOXEL_API FVoxelGeneratorPicker
#if CPP
	: TVoxelGeneratorPicker<UVoxelGenerator>
#endif
{
	GENERATED_BODY()
	
	using TVoxelGeneratorPicker<UVoxelGenerator>::TVoxelGeneratorPicker;
	
	// Will default to EmptyGenerator if null
	TVoxelSharedRef<FGeneratorInstance> GetInstance(bool bSilent) const;
	
#if !CPP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelGeneratorPickerType Type = EVoxelGeneratorPickerType::Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TSubclassOf<UVoxelGenerator> Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (DisallowedClasses = "/Script/VoxelGraph.VoxelGraphMacro"))
	TObjectPtr<UVoxelGenerator> Object = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TMap<FName, FString> Parameters;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	TObjectPtr<UObject> EditorData = nullptr;
#endif
#endif
};

USTRUCT(BlueprintType, meta=(HasNativeMake="/Script/Voxel.VoxelGeneratorTools:MakeTransformableGeneratorPickerFromObject"))
struct FVoxelTransformableGeneratorPicker
#if CPP
	: TVoxelGeneratorPicker<UVoxelTransformableGenerator>
#endif
{
	GENERATED_BODY()
	
	using TVoxelGeneratorPicker<UVoxelTransformableGenerator>::TVoxelGeneratorPicker;
	
	// Will default to EmptyGenerator if null
	TVoxelSharedRef<FGeneratorInstance> GetInstance(bool bSilent) const;

#if !CPP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelGeneratorPickerType Type = EVoxelGeneratorPickerType::Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TSubclassOf<UVoxelTransformableGenerator> Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (DisallowedClasses = "/Script/VoxelGraph.VoxelGraphMacro"))
	TObjectPtr<UVoxelTransformableGenerator> Object = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TMap<FName, FString> Parameters;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	TObjectPtr<UObject> EditorData = nullptr;
#endif
#endif
};

VOXEL_DEPRECATED(1.2, "Use FVoxelGeneratorPicker instead of FVoxelWorldGeneratorPicker")
typedef FVoxelGeneratorPicker FVoxelWorldGeneratorPicker;

VOXEL_DEPRECATED(1.2, "Use FVoxelTransformableGeneratorPicker instead of FVoxelTransformableWorldGeneratorPicker")
typedef FVoxelTransformableGeneratorPicker FVoxelTransformableWorldGeneratorPicker;