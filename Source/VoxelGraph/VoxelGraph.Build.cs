// Copyright 2019 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelGraph : ModuleRules
{
    public VoxelGraph(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

        if (!Target.bUseUnityBuild)
        {
            PrivatePCHHeaderFile = "Private/VoxelGraphPCH.h";
#if UE_4_22_OR_LATER
#else
            PrivateDependencyModuleNames.Add("LivePP");
#endif
        }

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Voxel"
            });

        if (Target.Configuration == UnrealTargetConfiguration.DebugGame)
        {
            PublicDefinitions.Add("VOXEL_DEBUG=1");
        }
    }
}
