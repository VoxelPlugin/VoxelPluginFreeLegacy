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

            PublicIncludePaths.Add(engine_path + "Source/Runtime/Engine/Private");
            PublicIncludePaths.Add(engine_path + "Source/ThirdParty/nvtesslib/inc");

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
                    "Networking",
                    "RenderCore",
                    "ShaderCore",
                    "RHI",
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
