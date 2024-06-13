// Copyright Voxel Plugin SAS. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class VoxelExamples : ModuleRules
{
    public VoxelExamples(ReadOnlyTargetRules Target) : base(Target)
{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;
		CppStandard = CppStandardVersion.Cpp17;

#if UE_4_24_OR_LATER
        bUseUnity = true;
#else
#endif

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Voxel",
                "Core",
                "CoreUObject",
                "Engine"
            }
        );
    }
}