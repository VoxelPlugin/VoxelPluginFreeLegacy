// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System.IO;


namespace UnrealBuildTool.Rules
{
    public class Voxel : ModuleRules
    {
        public Voxel(ReadOnlyTargetRules Target) : base(Target)
        {
            // Hack to include engine private folder
            // Get the engine path. Ends with "Engine/"
            string engine_path = Path.GetFullPath(BuildConfiguration.RelativeEnginePath);
            // Now get the base of UE4's modules dir (could also be Developer, Editor, ThirdParty)
            string srcrt_path = engine_path + "Source/Runtime/";
            PublicIncludePaths.Add(srcrt_path + "Engine/Private");

            PublicIncludePaths.AddRange(
                new string[] { }
                );

            PrivateIncludePaths.AddRange(
                new string[] {
                    "Voxel/Private",
                    "Voxel/Private/VoxelData",
                    "Voxel/Private/VoxelRender",

                    "Voxel/Private/VoxelWorldGenerators",
                    "Voxel/Private/VoxelAssets",
                    "Voxel/Private/VoxelModifiers",

                    "Voxel/Classes/VoxelWorldGenerators",
                    "Voxel/Classes/VoxelAssets",
                    "Voxel/Classes/VoxelModifiers",
                }
                );

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "ProceduralMeshComponent",
                    "Landscape",
                    "Sockets",
                    "Networking"
                }
                );


            PrivateDependencyModuleNames.AddRange(
                new string[]
                {

                }
                );

            DynamicallyLoadedModuleNames.AddRange(
                new string[]
                {
					// ... add any modules that your module loads dynamically here ...
				}
                );
        }
    }
}
