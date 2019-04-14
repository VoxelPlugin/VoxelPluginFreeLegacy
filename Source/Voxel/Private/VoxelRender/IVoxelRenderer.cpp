// Copyright 2019 Phyronnaz

#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelMaterialCollection.h"

#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"

#define LOCTEXT_NAMESPACE "Voxel"

UMaterialInterface* FVoxelRendererSettings::GetVoxelMaterial(const FVoxelBlendedMaterial& BlendedIndex, bool bTessellation) const
{
	if (!MaterialCollection.IsValid())
	{
		FMessageLog("PIE").Error(LOCTEXT("InvalidMaterialCollection", "Invalid Material Collection"));
		return nullptr;
	}
	auto* Value = MaterialCollection->GetVoxelMaterial(BlendedIndex, bTessellation);
	if (!Value)
	{
		for (uint8 Index : BlendedIndex.GetElements())
		{
			if (!MaterialCollection->Materials.ContainsByPredicate([&](FVoxelMaterialCollectionElement& Element)
			{
				if (Element.Children.Num() == 0)
				{
					return Element.Index == Index;
				}
				else
				{
					return Element.Children.ContainsByPredicate([&](FVoxelMaterialCollectionElementIndex& Child)
					{
						return Child.InstanceIndex == Index;
					});
				}
			}))
			{
				static TMap<const FVoxelRendererSettings*, TSet<uint8>> AlreadyWarnedIndex;
				auto& Set = AlreadyWarnedIndex.FindOrAdd(this);
				if (!Set.Contains(Index))
				{
					Set.Add(Index);
					TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Error);
					Message->AddToken(FTextToken::Create(FText::Format(LOCTEXT("MaterialCollectionInvalidIndex", "Index {0} not in "), Index)));
					Message->AddToken(FUObjectToken::Create(MaterialCollection.Get()));
					Message->AddToken(FTextToken::Create(LOCTEXT(
						"MaterialCollectionInvalidIndexHelp",
						". It's either because you painted/set an invalid index,"
						" or because you forgot to add this index in your material collection.")));
					FMessageLog("PIE").AddMessage(Message);
				}
				return nullptr;
			}
		}

		static TMap<const FVoxelRendererSettings*, TSet<FVoxelBlendedMaterial>> AlreadyWarnedIndex;
		auto& Set = AlreadyWarnedIndex.FindOrAdd(this);
		if (!Set.Contains(BlendedIndex))
		{
			Set.Add(BlendedIndex);
			TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Error);
			Message->AddToken(FTextToken::Create(LOCTEXT("MaterialCollectionMissingText", "Missing the following generated material in ")));
			Message->AddToken(FUObjectToken::Create(MaterialCollection.Get()));
			Message->AddToken(FTextToken::Create(
				FText::Format(LOCTEXT("MaterialCollectionMissing", ": {0} {1}. You need to open the asset and click the Generate {2} button. {3}"),
					FText::FromString(bTessellation ? "Tessellation" : ""),
					FText::FromString(BlendedIndex.ToString()),
					FText::FromString(BlendedIndex.KindToString()),
					FText::FromString(bTessellation && !MaterialCollection->bEnableTessellation ?
						"You also need to tick Enable Tessellation." :
						""))));
			FMessageLog("PIE").AddMessage(Message);
		}
		return nullptr;
	}
	return Value;
}

UMaterialInterface* FVoxelRendererSettings::GetVoxelMaterial(bool bTessellation) const
{
	const auto& Material = bTessellation ? VoxelMaterialWithTessellation : VoxelMaterialWithoutTessellation;
	if (!Material.IsValid())
	{
		FMessageLog("PIE").Error(bTessellation ?
			LOCTEXT("InvalidVoxelMaterialWithTessellation", "Invalid Tessellated Voxel Material") :
			LOCTEXT("InvalidVoxelMaterial", "Invalid Voxel Material"));
		return nullptr;
	}
	return Material.Get();
}
#undef LOCTEXT_NAMESPACE