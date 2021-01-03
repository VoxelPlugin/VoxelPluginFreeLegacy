// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"

template<typename T>
struct TGetProperty;

template<>
struct TGetProperty<int32>
{
	using Type = FIntProperty;
};

template<>
struct TGetProperty<float>
{
	using Type = FFloatProperty;
};

template<>
struct TGetProperty<bool>
{
	using Type = FBoolProperty;
};

template<>
struct TGetProperty<FName>
{
	using Type = FNameProperty;
};

struct FVoxelGeneratorParametersUtilities
{
	template<typename T>
	static T GetFundamentalParameter(FProperty& Property, const UObject* This, const FString* Override)
	{
		using TProperty = typename TGetProperty<T>::Type;
		
		if (!ensure(CastField<TProperty>(&Property)))
		{
			return T{};
		}

		const T Default = *Property.ContainerPtrToValuePtr<T>(This);

		if (!Override)
		{
			return Default;
		}

		T Temp{};
		if (!ensure(Property.ImportText(**Override, &Temp, PPF_None, GetTransientPackage())))
		{
			return Default;
		}

		return Temp;
	}
	template<typename T>
	static T GetStructParameter(FProperty& Property, const UObject* This, const FString* Override)
	{
		UStruct* Struct = TBaseStructure<T>::Get();

		auto* StructProperty = CastField<FStructProperty>(&Property);
		if (!ensure(StructProperty) ||
			!ensure(Struct == StructProperty->Struct))
		{
			return {};
		}

		const T& Default = *Property.ContainerPtrToValuePtr<T>(This);

		if (!Override)
		{
			return Default;
		}

		T Temp;
		Property.InitializeValue_InContainer(&Temp);

		if (!ensure(Property.ImportText(**Override, &Temp, PPF_None, GetTransientPackage())))
		{
			return Default;
		}

		return Temp;
	}
	template<typename T>
	static T* GetObjectParameter(FProperty& Property, const UObject* This, const FString* Override)
	{
		static_assert(TIsDerivedFrom<T, UObject>::IsDerived, "");

		auto* ObjectProperty = UE_25_SWITCH(CastChecked, CastFieldChecked)<FObjectProperty>(&Property);
		if (!ensure(ObjectProperty) ||
			!ensure(ObjectProperty->PropertyClass->IsChildOf<T>()))
		{
			return nullptr;
		}

		T* const Default = *Property.ContainerPtrToValuePtr<T*>(This);

		if (!Override)
		{
			return Default;
		}

		UObject* Temp = nullptr;
		if (!ensure(Property.ImportText(**Override, &Temp, PPF_None, GetTransientPackage())))
		{
			return Default;
		}
		ensure(!Temp || Temp->GetClass()->IsChildOf<T>());
		
		return Cast<T>(Temp);
	}
	template<typename T>
	static T GetArrayParameter(FProperty& Property, const UObject* This, const FString* Override)
	{
		const T& Default = *Property.ContainerPtrToValuePtr<T>(This);

		if (!Override)
		{
			return Default;
		}

		T Temp;
		Property.InitializeValue_InContainer(&Temp);
		
		if (!ensure(Property.ImportText(**Override, &Temp, PPF_None, GetTransientPackage())))
		{
			return Default;
		}
		
		return Temp;
	}

public:
	// Default to structs as there's no way to differentiate them
	template<typename T, typename = void>
	struct TChooseParameter
	{
		static T GetParameter(FProperty& Property, const UObject* This, const FString* Override)
		{
			return GetStructParameter<T>(Property, This, Override);
		}
	};

	template<typename T>
	struct TChooseParameter<T, typename TEnableIf<TIsTArray<T>::Value>::Type>
	{
		static T GetParameter(FProperty& Property, const UObject* This, const FString* Override)
		{
			return GetArrayParameter<T>(Property, This, Override);
		}
	};

	template<typename T>
	struct TChooseParameter<T, typename TEnableIf<TOr<TIsFundamentalType<T>, TIsSame<T, FName>>::Value>::Type>
	{
		static T GetParameter(FProperty& Property, const UObject* This, const FString* Override)
		{
			return GetFundamentalParameter<T>(Property, This, Override);
		}
	};

	template<typename T>
	struct TChooseParameter<T, typename TEnableIf<TIsDerivedFrom<T, UObject>::IsDerived>::Type>
	{
		static T* GetParameter(FProperty& Property, const UObject* This, const FString* Override)
		{
			return GetObjectParameter<T>(Property, This, Override);
		}
	};
	
public:
	template<typename T>
	static auto GetParameter(FProperty& Property, const UObject* This, const FString* Override)
	{
		return TChooseParameter<T>::GetParameter(Property, This, Override);
	}
};