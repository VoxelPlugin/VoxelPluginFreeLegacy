// Copyright Voxel Plugin SAS. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class VoxelHelpers : ModuleRules
{
    public VoxelHelpers(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;
#if UE_5_5_OR_LATER
        CppStandard = CppStandardVersion.Cpp20;
#else
		CppStandard = CppStandardVersion.Cpp17;
#endif

#if UE_4_24_OR_LATER
        bUseUnity = true;
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
                "ProceduralMeshComponent",
                "Voxel"
            }
        );
    }
}
