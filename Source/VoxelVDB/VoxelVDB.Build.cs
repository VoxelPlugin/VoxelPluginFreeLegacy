// Copyright 2020 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelVDB : ModuleRules
{
    public VoxelVDB(ReadOnlyTargetRules Target) : base(Target)
{
        // OpenVDB/boost requires to have both of these on
        // RTTI is a global setting in packaged games: as such, this module is editor only by default
        // Set it to Runtime in Voxel.uplugin if you wish to use VDB at runtime
        bUseRTTI = true;
        bEnableExceptions = true;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;


        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Voxel",
                "OpenVDB",
                "UEOpenExr",
                "Core",
                "CoreUObject",
                "Engine"
            }
        );
    }
}