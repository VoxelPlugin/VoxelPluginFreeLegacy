// Copyright Voxel Plugin SAS. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class VoxelGraph : ModuleRules
{
    public VoxelGraph(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;
#if UE_5_4_OR_LATER
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
                "Voxel"
            });
    }
}
