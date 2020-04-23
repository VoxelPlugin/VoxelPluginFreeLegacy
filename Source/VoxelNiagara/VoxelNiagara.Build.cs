// Copyright 2020 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelNiagara : ModuleRules
{
    public VoxelNiagara(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;


        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Niagara",
                "NiagaraCore",
                "Voxel"
            }
        );
    }
}
