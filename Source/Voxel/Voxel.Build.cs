// Copyright 2019 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class Voxel : ModuleRules
{
    public Voxel(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

        if (!Target.bUseUnityBuild)
        {
            PrivatePCHHeaderFile = "Private/VoxelPCH.h";
#if UE_4_22_OR_LATER
#else
            PrivateDependencyModuleNames.Add("LivePP");
#endif
        }

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Networking",
                "Sockets",
                "RHI",
                "RenderCore",
                "Slate",
                "SlateCore",
                "Landscape",
                "HTTP"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "nvTessLib",
                "Steamworks"
            }
        );

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateDependencyModuleNames.Add("ForsythTriOptimizer");
        }

        if (Target.Configuration == UnrealTargetConfiguration.DebugGame)
        {
            PublicDefinitions.Add("VOXEL_DEBUG=1");
        }

        // Steam
        {
            //string SteamVersion = "v139";
            //PublicDefinitions.Add("STEAM_SDK_VER=TEXT(\"1.39\")");
            //PublicDefinitions.Add("STEAM_SDK_VER_PATH=TEXT(\"Steam" + SteamVersion + "\")");

            //string SdkBase = Target.UEThirdPartySourceDirectory + "Steamworks/Steam" + SteamVersion + "/sdk";
            //PublicDefinitions.Add("USE_STEAM=" + (Directory.Exists(SdkBase) ? "1" : "0"));
            PublicDefinitions.Add("USE_STEAM=0");
        }

        // Embree
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string SDKDir = Target.UEThirdPartySourceDirectory + "IntelEmbree/Embree2140/Win64/";

            PublicIncludePaths.Add(SDKDir + "include");
            PublicLibraryPaths.Add(SDKDir + "lib");
            PublicAdditionalLibraries.Add("embree.2.14.0.lib");
            RuntimeDependencies.Add("$(EngineDir)/Binaries/Win64/embree.2.14.0.dll");
            PublicDelayLoadDLLs.Add("embree.2.14.0.dll");
            RuntimeDependencies.Add("$(EngineDir)/Binaries/Win64/tbb.dll");
            RuntimeDependencies.Add("$(EngineDir)/Binaries/Win64/tbbmalloc.dll");
            PublicDefinitions.Add("USE_EMBREE=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string SDKDir = Target.UEThirdPartySourceDirectory + "IntelEmbree/Embree2140/MacOSX/";

            PublicIncludePaths.Add(SDKDir + "include");
            PublicAdditionalLibraries.Add(SDKDir + "lib/libembree.2.14.0.dylib");
            PublicAdditionalLibraries.Add(SDKDir + "lib/libtbb.dylib");
            PublicAdditionalLibraries.Add(SDKDir + "lib/libtbbmalloc.dylib");
            PublicDefinitions.Add("USE_EMBREE=1");
        }
        else
        {
            PublicDefinitions.Add("USE_EMBREE=0");
        }
    }
}
