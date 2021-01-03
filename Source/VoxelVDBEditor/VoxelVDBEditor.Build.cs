// Copyright 2021 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelVDBEditor : ModuleRules
{
    public VoxelVDBEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

#if UE_4_24_OR_LATER
#else
#endif

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "SlateCore",
                "Slate",
                "EditorStyle",
                "Voxel",
                "VoxelEditor",
                "VoxelVDB",
                "DesktopPlatform",
                "UnrealEd"
            });
    }
}
