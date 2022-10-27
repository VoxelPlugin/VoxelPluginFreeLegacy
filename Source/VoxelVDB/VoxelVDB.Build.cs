// Copyright 2021 Phyronnaz

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

#if UE_4_24_OR_LATER
#else
#endif

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

#if UE_5_0_OR_LATER
        // Needed to use OpenVDB, else include problems from it
        AddEngineThirdPartyPrivateStaticDependencies(Target, "IntelTBB");
#endif
    }
}