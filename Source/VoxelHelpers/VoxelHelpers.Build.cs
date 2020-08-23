// Copyright 2020 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelHelpers : ModuleRules
{
    public VoxelHelpers(ReadOnlyTargetRules Target) : base(Target)
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
                "UMG",
                "Slate",
                "SlateCore",
                "ProceduralMeshComponent"
            }
        );
    }
}
