// Copyright 2020 Phyronnaz

#define VOXEL_PLUGIN_PRO

using System.IO;
using UnrealBuildTool;

public class Voxel : ModuleRules
{
    public Voxel(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

#if UE_4_24_OR_LATER
#else
#endif

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        // For raytracing
        PrivateIncludePaths.Add(EngineDirectory + "/Shaders/Shared");

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Networking",
                "Sockets",
                "RHI",
#if UE_4_23_OR_LATER
                "PhysicsCore",
#endif
                "RenderCore",
                "Landscape",
                "PhysX",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "nvTessLib",
                "HTTP",
                "Projects",
                "Slate",
                "SlateCore",
                //"VHACD", // Not used, too slow
            }
        );

        SetupModulePhysicsSupport(Target);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateDependencyModuleNames.Add("ForsythTriOptimizer");
        }
        PrivateDependencyModuleNames.Add("zlib");

        if (Target.Configuration == UnrealTargetConfiguration.DebugGame ||
			Target.Configuration == UnrealTargetConfiguration.Debug)
        {
            PublicDefinitions.Add("VOXEL_DEBUG=1");
        }

        PublicDefinitions.Add("VOXEL_PLUGIN_NAME=TEXT(\"VoxelFree\")");
    }
}
