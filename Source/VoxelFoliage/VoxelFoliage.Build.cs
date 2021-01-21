// Copyright 2021 Phyronnaz

#define VOXEL_PLUGIN_PRO

using System.IO;
using UnrealBuildTool;

public class VoxelFoliage : ModuleRules
{
    public VoxelFoliage(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

#if UE_4_24_OR_LATER
#else
#endif

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "RenderCore",
                "Voxel",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
            }
        );
    }
}
