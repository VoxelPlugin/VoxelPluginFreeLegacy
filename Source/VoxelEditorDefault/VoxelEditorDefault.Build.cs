// Copyright 2019 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelEditorDefault : ModuleRules
{
    public VoxelEditorDefault(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;
        
        if (!Target.bUseUnityBuild)
        {
            PrivatePCHHeaderFile = "Private/VoxelEditorDefaultPCH.h";
#if UE_4_22_OR_LATER
#else
            PrivateDependencyModuleNames.Add("LivePP");
#endif
        }

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
