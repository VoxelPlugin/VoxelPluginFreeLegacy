// Copyright 2020 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelVDBEditor : ModuleRules
{
    public VoxelVDBEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

        
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
