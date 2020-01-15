// Copyright 2020 Phyronnaz

#include "VoxelTexture.h"
#include "VoxelMessages.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"

DEFINE_STAT(STAT_VoxelTextureMemory);

#define LOCTEXT_NAMESPACE "Voxel"

template<typename T>
inline auto& GetVoxelTextureCacheMap()
{
	check(IsInGameThread());
	static TMap<TWeakObjectPtr<UTexture>, TVoxelSharedPtr<typename TVoxelTexture<T>::FTextureData>> Map;
	Map.Remove(nullptr);
	return Map;
}

inline void ExtractTextureData(UTexture* Texture, int32& OutSizeX, int32& OutSizeY, TArray<FColor>& OutData)
{
	VOXEL_FUNCTION_COUNTER();
	
	check(IsInGameThread());

	if (auto* Texture2D = Cast<UTexture2D>(Texture))
	{
		FTexture2DMipMap& Mip = Texture2D->PlatformData->Mips[0];
		OutSizeX = Mip.SizeX;
		OutSizeY = Mip.SizeY;

		const int32 Size = OutSizeX * OutSizeY;
		OutData.SetNumUninitialized(Size);

		auto& BulkData = Mip.BulkData;
		if (!ensureAlways(BulkData.GetBulkDataSize() > 0))
		{
			OutSizeX = 1;
			OutSizeY = 1;
			OutData.SetNum(1);
			return;
		}

		void* Data = BulkData.Lock(LOCK_READ_ONLY);
		if (!ensureAlways(Data))
		{
			OutSizeX = 1;
			OutSizeY = 1;
			OutData.SetNum(1);
			return;
		}

		FMemory::Memcpy(OutData.GetData(), Data, Size * sizeof(FColor));
		Mip.BulkData.Unlock();
		return;
	}

	if (auto* TextureRenderTarget = Cast<UTextureRenderTarget2D>(Texture))
	{
		FRenderTarget* RenderTarget = TextureRenderTarget->GameThread_GetRenderTargetResource();
		if (ensure(RenderTarget))
		{
			const auto Format = TextureRenderTarget->GetFormat();

			OutSizeX = TextureRenderTarget->GetSurfaceWidth();
			OutSizeY = TextureRenderTarget->GetSurfaceHeight();

			const int32 Size = OutSizeX * OutSizeY;
			OutData.SetNumUninitialized(Size);

			switch (Format)
			{
			case PF_B8G8R8A8:
			{
				if (ensure(RenderTarget->ReadPixels(OutData))) return;
				break;
			}
			case PF_R8G8B8A8:
			{
				if (ensure(RenderTarget->ReadPixels(OutData))) return;
				break;
			}
			case PF_FloatRGBA:
			{
				TArray<FLinearColor> LinearColors;
				LinearColors.SetNumUninitialized(Size);
				if (ensure(RenderTarget->ReadLinearColorPixels(LinearColors)))
				{
					for (int32 Index = 0; Index < Size; Index++)
					{
						OutData[Index] = LinearColors[Index].ToFColor(false);
					}
					return;
				}
				break;
			}
			default:
				ensure(false);
			}
		}
	}

	ensure(false);
	OutSizeX = 1;
	OutSizeY = 1;
	OutData.SetNum(1);
}

TVoxelTexture<FColor> FVoxelTextureUtilities::CreateFromTexture_Color(UTexture* Texture)
{
	VOXEL_FUNCTION_COUNTER();

	auto& Data = GetVoxelTextureCacheMap<FColor>().FindOrAdd(Texture);
	if (!Data.IsValid())
	{
		FString Error;
		if (!CanCreateFromTexture(Texture, Error))
		{
			FVoxelMessages::Error(FText::Format(LOCTEXT("CreateFromTextureError", "Can't create Voxel Texture: {0}"), FText::FromString(Error)), Texture);
			return {};
		}

		Data = MakeVoxelShared<TVoxelTexture<FColor>::FTextureData>();
		ExtractTextureData(Texture, Data->SizeX, Data->SizeY, Data->TextureData);
		Data->UpdateAllocatedSize();
	}

	return TVoxelTexture<FColor>(Data.ToSharedRef());
}

TVoxelTexture<float> FVoxelTextureUtilities::CreateFromTexture_Float(UTexture* Texture, EVoxelRGBA Channel)
{
	VOXEL_FUNCTION_COUNTER();

	auto& Data = GetVoxelTextureCacheMap<float>().FindOrAdd(Texture);
	if (!Data.IsValid())
	{
		FString Error;
		if (!CanCreateFromTexture(Texture, Error))
		{
			FVoxelMessages::Error(FText::Format(LOCTEXT("CreateFromTextureError", "Can't create Voxel Texture: {0}"), FText::FromString(Error)), Texture);
			return {};
		}

		const auto ColorTexture = CreateFromTexture_Color(Texture);

		Data = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
		Data->SizeX = ColorTexture.GetSizeX();
		Data->SizeY = ColorTexture.GetSizeY();
		Data->TextureData.SetNumUninitialized(Data->SizeX * Data->SizeY);

#if VOXEL_DEBUG
		const auto& ColorTextureData = ColorTexture.GetTextureData();
		auto& FloatTextureData = Data->TextureData;
#else
		const auto* RESTRICT ColorTextureData = ColorTexture.GetTextureData().GetData();
		auto* RESTRICT FloatTextureData = Data->TextureData.GetData();
#endif
		for (int32 Index = 0; Index < Data->TextureData.Num(); Index++)
		{
			const FColor Color = ColorTextureData[Index];
			uint8 Result = 0;
			switch (Channel)
			{
			case EVoxelRGBA::R:
				Result = Color.R;
				break;
			case EVoxelRGBA::G:
				Result = Color.G;
				break;
			case EVoxelRGBA::B:
				Result = Color.B;
				break;
			case EVoxelRGBA::A:
				Result = Color.A;
				break;
			}
			const float Value = FVoxelUtilities::UINT8ToFloat(Result);
			FloatTextureData[Index] = Value;
			Data->Min = FMath::Min(Data->Min, Value);
			Data->Max = FMath::Max(Data->Max, Value);
		}
	}
	return TVoxelTexture<float>(Data.ToSharedRef());
}

bool FVoxelTextureUtilities::CanCreateFromTexture(UTexture* Texture, FString& OutError)
{
	if (!Texture)
	{
		OutError = "Invalid texture";
		return false;
	}
	if (auto* Texture2D = Cast<UTexture2D>(Texture))
	{
#if WITH_EDITORONLY_DATA
		if (Texture2D->MipGenSettings != TextureMipGenSettings::TMGS_NoMipmaps)
		{
			OutError = "Texture MipGenSettings must be NoMipmaps";
			return false;
		}
#endif
		if (Texture2D->CompressionSettings != TextureCompressionSettings::TC_VectorDisplacementmap)
		{
			OutError = "Texture CompressionSettings must be VectorDisplacementmap";
			return false;
		}
		return true;
	}
	if (auto* TextureRenderTarget = Cast<UTextureRenderTarget2D>(Texture))
	{
		const auto Format = TextureRenderTarget->GetFormat();
		if (Format != EPixelFormat::PF_R8G8B8A8 && 
			Format != EPixelFormat::PF_FloatRGBA &&
			Format != EPixelFormat::PF_B8G8R8A8)
		{
			OutError = "Render Target PixelFormat must be R8G8B8A8, B8G8R8A8 or R8G8B8A8 (is " + FString(GPixelFormats[Format].Name) + ")";
			return false;
		}
		if (!TextureRenderTarget->GameThread_GetRenderTargetResource())
		{
			OutError = "Render Target resource must be created";
			return false;
		}
		return true;
	}
	OutError = "Texture must be a Texture2D or a TextureRenderTarget2D";
	return false;
}

void FVoxelTextureUtilities::FixTexture(UTexture* Texture)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!ensure(Texture))
	{
		return;
	}
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
	Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	Texture->UpdateResource();
	Texture->MarkPackageDirty();
}

void FVoxelTextureUtilities::ClearCache()
{
	VOXEL_FUNCTION_COUNTER();
	
	GetVoxelTextureCacheMap<FColor>().Empty();
	GetVoxelTextureCacheMap<float>().Empty();
}

void FVoxelTextureUtilities::ClearCache(UTexture* Texture)
{
	VOXEL_FUNCTION_COUNTER();
	
	GetVoxelTextureCacheMap<FColor>().Remove(Texture);
	GetVoxelTextureCacheMap<float>().Remove(Texture);
}

void FVoxelFloatTexture::CreateOrUpdateTexture(UTexture2D*& InTexture) const
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!InTexture || 
		!InTexture->PlatformData || 
		InTexture->PlatformData->Mips.Num() == 0 || 
		InTexture->PlatformData->PixelFormat != EPixelFormat::PF_R32_FLOAT ||
		InTexture->GetSizeX() != Texture.GetSizeX() || 
		InTexture->GetSizeY() != Texture.GetSizeY())
	{
		InTexture = UTexture2D::CreateTransient(Texture.GetSizeX(), Texture.GetSizeY(), EPixelFormat::PF_R32_FLOAT);
		InTexture->CompressionSettings = TC_HDR;
		InTexture->SRGB = false;
		InTexture->Filter = TF_Bilinear;
	}
	
	FTexture2DMipMap& Mip = InTexture->PlatformData->Mips[0];
	float* Data = static_cast<float*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
	if (!ensureAlways(Data)) return;

	FMemory::Memcpy(Data, Texture.GetTextureData().GetData(), Texture.GetSizeX() * Texture.GetSizeY() * sizeof(float));

	Mip.BulkData.Unlock();
	
	InTexture->UpdateResource();
}
#undef LOCTEXT_NAMESPACE