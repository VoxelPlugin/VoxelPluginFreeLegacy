// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelMinimal.h"

// Fixup for ensure in FDomainAssetReferenceFilter::PassesFilterImpl
#define final
#define private public
#include "Runtime/Projects/Private/PluginManager.h"
#undef final
#undef private

#if !IS_MONOLITHIC
FPlugin::FPlugin(const FString& InFileName, const FPluginDescriptor& InDescriptor, EPluginType InType)
	: Name(FPaths::GetBaseFilename(InFileName))
	, FileName(InFileName)
	, Descriptor(InDescriptor)
	, Type(InType)
	, bEnabled(false)
{

}

FPlugin::~FPlugin()
{
}

FString FPlugin::GetBaseDir() const
{
	return FPaths::GetPath(FileName);
}

TArray<FString> FPlugin::GetExtensionBaseDirs() const
{
	TArray<FString> OutDirs;
	OutDirs.Reserve(PluginExtensionFileNameList.Num());
	for (const FString& ExtensionFileName : PluginExtensionFileNameList)
	{
		OutDirs.Emplace(FPaths::GetPath(ExtensionFileName));
	}
	return OutDirs;
}

FString FPlugin::GetContentDir() const
{
	return FPaths::GetPath(FileName) / TEXT("Content");
}

FString FPlugin::GetMountedAssetPath() const
{
	FString Path;
	Path.Reserve(Name.Len() + 2);
	Path.AppendChar('/');
	Path.Append(Name);
	Path.AppendChar('/');
	return Path;
}

bool FPlugin::IsEnabledByDefault(const bool bAllowEnginePluginsEnabledByDefault) const
{
	if (Descriptor.EnabledByDefault == EPluginEnabledByDefault::Enabled)
	{
		return (GetLoadedFrom() == EPluginLoadedFrom::Project ? true : bAllowEnginePluginsEnabledByDefault);
	}
	else if (Descriptor.EnabledByDefault == EPluginEnabledByDefault::Disabled)
	{
		return false;
	}
	else
	{
		return GetLoadedFrom() == EPluginLoadedFrom::Project;
	}
}

EPluginLoadedFrom FPlugin::GetLoadedFrom() const
{
	if (Type == EPluginType::Engine || Type == EPluginType::Enterprise)
	{
		return EPluginLoadedFrom::Engine;
	}
	else
	{
		return EPluginLoadedFrom::Project;
	}
}

const FPluginDescriptor& FPlugin::GetDescriptor() const
{
	return Descriptor;
}

bool FPlugin::UpdateDescriptor(const FPluginDescriptor& NewDescriptor, FText& OutFailReason)
{
	if (!NewDescriptor.UpdatePluginFile(FileName, OutFailReason))
	{
		return false;
	}

	Descriptor = NewDescriptor;

	IPluginManager& PluginManager = IPluginManager::Get();
	if (PluginManager.OnPluginEdited().IsBound())
	{
		PluginManager.OnPluginEdited().Broadcast(*this);
	}

	return true;
}

#if WITH_EDITOR
const TSharedPtr<FJsonObject>& FPlugin::GetDescriptorJson()
{
	return Descriptor.CachedJson;
}
#endif // WITH_EDITOR
#endif

class FVoxelPlugin : public FPlugin
{
public:
	const TSharedRef<IPlugin> Plugin;
	FString MountedAssetPath;

	explicit FVoxelPlugin(const TSharedRef<FPlugin>& Plugin)
		: FPlugin(Plugin->FileName, Plugin->Descriptor, Plugin->Type)
		, Plugin(Plugin)
	{
		Name = Plugin->Name;
		FileName = Plugin->FileName;
		Descriptor = Plugin->Descriptor;
		Type = Plugin->Type;
		bEnabled = Plugin->bEnabled;
	}

	virtual const FString& GetName() const override
	{
		return Plugin->GetName();
	}
	virtual const FString& GetFriendlyName() const override
	{
		return Plugin->GetFriendlyName();
	}
	virtual const FString& GetDescriptorFileName() const override
	{
		return Plugin->GetDescriptorFileName();
	}
	virtual FString GetBaseDir() const override
	{
		return Plugin->GetBaseDir();
	}
	virtual FString GetContentDir() const override
	{
		return Plugin->GetContentDir();
	}
	virtual FString GetMountedAssetPath() const override
	{
		return "/Voxel/";
	}
	virtual bool IsEnabled() const override
	{
		return Plugin->IsEnabled();
	}
	virtual bool IsEnabledByDefault(bool bAllowEnginePluginsEnabledByDefault) const override
	{
		return Plugin->IsEnabledByDefault(bAllowEnginePluginsEnabledByDefault);
	}
	virtual bool IsHidden() const override
	{
		return Plugin->IsHidden();
	}
	virtual bool CanContainContent() const override
	{
		return Plugin->CanContainContent();
	}
	virtual bool CanContainVerse() const override
	{
		return Plugin->CanContainVerse();
	}
	virtual EPluginType GetType() const override
	{
		return Plugin->GetType();
	}

	virtual EPluginLoadedFrom GetLoadedFrom() const override
	{
		return Plugin->GetLoadedFrom();
	}
	virtual const FPluginDescriptor& GetDescriptor() const override
	{
		return Plugin->GetDescriptor();
	}
	virtual bool UpdateDescriptor(const FPluginDescriptor& NewDescriptor, FText& OutFailReason) override
	{
		return Plugin->UpdateDescriptor(NewDescriptor, OutFailReason);
	}
#if WITH_EDITOR
	virtual const TSharedPtr<FJsonObject>& GetDescriptorJson() override
	{
		return Plugin->GetDescriptorJson();
	}
#endif
};

FDelayedAutoRegisterHelper GVoxelFixupPluginHelper(EDelayedRegisterRunPhase::EarliestPossiblePluginsLoaded, []
{
	TSharedRef<FPlugin>& VoxelPlugin = static_cast<FPluginManager&>(IPluginManager::Get()).AllPlugins.FindChecked(VOXEL_PLUGIN_NAME)[0];
	VoxelPlugin = MakeShared<FVoxelPlugin>(VoxelPlugin);
});