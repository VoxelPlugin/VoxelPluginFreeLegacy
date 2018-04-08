// Copyright 2018 Phyronnaz

using System.IO;


namespace UnrealBuildTool.Rules
{
    public class Voxel : ModuleRules
    {
        public Voxel(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            bEnforceIWYU = false;

            PublicIncludePaths.AddRange(
                new string[] {
                    "Voxel/Classes/VoxelWorldGenerators",
                    "Voxel/Classes/VoxelAssets",
                    "Voxel/Classes/VoxelImporters",
                    }
                );

            PrivateIncludePaths.AddRange(
                new string[] {
                    "Voxel/Private",
                    "Voxel/Private/VoxelAssets",
                    "Voxel/Private/VoxelData",
                    "Voxel/Private/VoxelImporters",
                    "Voxel/Private/VoxelRender",
                    "Voxel/Private/VoxelWorldGenerators",
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

				}
                );
        }
    }
}
