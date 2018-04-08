// Copyright 2018 Phyronnaz

#include "VoxelTextureTools.h"
#include "VoxelPrivate.h"
#include "Engine/Texture2DArray.h"
#include "Engine/Texture2D.h"
#include "UnrealMemory.h"
#include "Engine/Texture2D.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/StreamableManager.h"
#include "RenderUtils.h"

UTexture2DArray* UVoxelTextureTools::CreateTextureArrayFromTextures(const TArray<UTexture2D*>& Textures)
{
	if (Textures.Num() == 0)
	{
		return nullptr;
	}

	int SizeX = -1;
	int SizeY = -1;
	EPixelFormat PixelFormat = EPixelFormat::PF_Unknown;

	bool bInitialized = false;
	int MaxMipMaps = 1000;

	int i = 0;
	for (auto Texture : Textures)
	{
		if (!Texture)
		{
			UE_LOG(LogVoxel, Warning, TEXT("CreateTextureArrayFromTextures: Texture is NULL (index = %d)"), i);
			continue;
		}
		i++;

		int NewSizeX = Texture->PlatformData->Mips[Texture->LODBias].SizeX;
		int NewSizeY = Texture->PlatformData->Mips[Texture->LODBias].SizeY;
		EPixelFormat NewPixelFormat = Texture->PlatformData->PixelFormat;
		MaxMipMaps = FMath::Min(MaxMipMaps, Texture->PlatformData->Mips.Num() - Texture->LODBias);
		if (!bInitialized)
		{
			SizeX = NewSizeX;
			SizeY = NewSizeY;
			PixelFormat = NewPixelFormat;
			bInitialized = true;
		}
		else
		{
			if (SizeX != NewSizeX || SizeY != NewSizeY)
			{
				UE_LOG(LogVoxel, Error, TEXT("CreateTextureArrayFromTextures: Textures don't have the same size! (%s)"), *Texture->GetName());
				return nullptr;
			}
			else if (PixelFormat != NewPixelFormat)
			{
				UE_LOG(LogVoxel, Error, TEXT("CreateTextureArrayFromTextures: Textures don't have the same pixel format! (%s)"), *Texture->GetName());
				return nullptr;
			}
		}
	}
	if (!bInitialized)
	{
		return nullptr;
	}

	const int Slices = Textures.Num();

	auto Texture = UTexture2DArray::CreateTransient(SizeX, SizeY, Slices, PixelFormat);

	for (int MipIndex = 0; MipIndex < MaxMipMaps; MipIndex++)
	{
		if (MipIndex != 0)
		{
			int32 NumBlocksX = SizeX / GPixelFormats[PixelFormat].BlockSizeX;
			int32 NumBlocksY = SizeY / GPixelFormats[PixelFormat].BlockSizeY;
			Texture->PlatformData->Mips.Add(new FTexture2DMipMap());
			FTexture2DMipMap* Mip = &Texture->PlatformData->Mips.Last();
			Mip->SizeX = SizeX;
			Mip->SizeY = SizeY;
			Mip->BulkData.Lock(LOCK_READ_WRITE);
			Mip->BulkData.Realloc((NumBlocksX * NumBlocksY * GPixelFormats[PixelFormat].BlockBytes) * Slices);
			Mip->BulkData.Unlock();
		}
		
		const int32 SliceSize = CalculateImageBytes(SizeX, SizeY, 0, PixelFormat);

		UE_LOG(LogVoxel, Log, TEXT("Size of one slice: %d \n Total Size: %d"), SliceSize, SliceSize * Slices);

		TArray<uint8> NewData;
		NewData.SetNumUninitialized(SliceSize * Slices);
		
		// Copy textures to Data
		{
			FTexture2DMipMap& Mip = Texture->PlatformData->Mips[MipIndex];

			void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);

			{
				for (int SliceIndex = 0; SliceIndex < Slices; SliceIndex++)
				{
					UTexture2D* LocalTexture = Textures[SliceIndex];
					if (LocalTexture)
					{
						FTexture2DMipMap& LocalMip = LocalTexture->PlatformData->Mips[MipIndex + LocalTexture->LODBias];

						void* TextureData = LocalMip.BulkData.Lock(LOCK_READ_ONLY);
						FMemory::Memcpy((uint8*)Data + SliceSize * SliceIndex, TextureData, SliceSize);
						LocalMip.BulkData.Unlock();
					}
				}
			}

			Mip.BulkData.Unlock();
		}

		SizeX /= 2;
		SizeY /= 2;
		if (SizeX < 4 || SizeY < 4)
		{
			break;
		}
	}
	Texture->UpdateResource();

	return Texture;
}

void UVoxelTextureTools::GetTexturesInFolder(const FString& Path, TArray<FString>& Paths, TArray<FString>& Names)
{
	auto ObjectLibrary = UObjectLibrary::CreateLibrary(UTexture2D::StaticClass(), false, true);
	ObjectLibrary->LoadAssetDataFromPath(Path);

	TArray<FAssetData> AssetDatas;
	ObjectLibrary->GetAssetDataList(AssetDatas);
	UE_LOG(LogTemp, Warning, TEXT("Found assets: %d"), AssetDatas.Num());
	
	for (const auto& AssetData : AssetDatas)
	{
		Paths.Add(AssetData.ObjectPath.ToString());
		Names.Add(AssetData.AssetName.ToString());
	}
}

void UVoxelTextureTools::LoadTexturesFromFolder(const FString& Path, const FString& DiffuseSuffix, const FString& NormalSuffix, TArray<UTexture2D*>& Diffuses, TArray<UTexture2D*>& Normals)
{
	TArray<FString> Paths;
	TArray<FString> Names;
	GetTexturesInFolder(Path, Paths, Names);
	for (int Index = 0; Index < Paths.Num(); Index++)
	{
		const FString& TexturePath = Paths[Index];
		const FString& TextureName = Names[Index];

		if (TexturePath.EndsWith(DiffuseSuffix))
		{
			Diffuses.Add(LoadTexture(TexturePath));

			FString Name = TextureName;
			verify(Name.RemoveFromEnd(DiffuseSuffix));

			const int32 NormalIndex = Names.IndexOfByPredicate(([&](FString P) { return P.StartsWith(Name + NormalSuffix); }));

			Normals.Add(NormalIndex > 0 ? LoadTexture(Paths[NormalIndex]) : nullptr);
		}
	}
}

UTexture2D* UVoxelTextureTools::LoadTexture(const FString& Path)
{
	 FStreamableManager AssetLoader;
     FStringAssetReference AssetRef(Path);
     return CastChecked<UTexture2D>(AssetLoader.LoadSynchronous(AssetRef));
}
