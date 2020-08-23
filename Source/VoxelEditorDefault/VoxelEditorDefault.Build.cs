// Copyright 2020 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelEditorDefault : ModuleRules
{
    public VoxelEditorDefault(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

#if UE_4_24_OR_LATER
#else
#endif

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

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
