// Copyright 2019 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelEditorDefault : ModuleRules
{
    public VoxelEditorDefault(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#if UE_4_20_OR_LATER
        bLegacyPublicIncludePaths = false;
#endif

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
            });

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "Voxel",
                "UnrealEd"
            });

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
            });
    }
}
