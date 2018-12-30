// Copyright 2018 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class Voxel : ModuleRules
{
    public Voxel(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = false;
#if UE_4_20_OR_LATER
        bLegacyPublicIncludePaths = false;
#endif
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "ProceduralMeshComponent",
                "Landscape",
                "Sockets",
                "Networking",
                "RenderCore",
                "ShaderCore",
                "RHI"
            }
        );

        AddEngineThirdPartyPrivateStaticDependencies(Target, "nvTessLib");

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            AddEngineThirdPartyPrivateStaticDependencies(Target, "ForsythTriOptimizer");
            PrivateDependencyModuleNames.Add("ForsythTriOptimizer"); // Above line doesn't work when not in monolithic build
        }

        if (Target.Configuration == UnrealTargetConfiguration.DebugGame)
        {
            PublicDefinitions.Add("VOXEL_DEBUG=1");
        }
    }
}
