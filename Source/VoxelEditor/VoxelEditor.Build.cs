// Copyright Voxel Plugin SAS. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class VoxelEditor : ModuleRules
{
    public VoxelEditor(ReadOnlyTargetRules Target) : base(Target)
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

        PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Source/Editor/PropertyEditor/Private"));

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "VoxelGraphEditor",
                "AssetRegistry",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "Voxel",
                "VoxelGraph",
                "VoxelEditorDefault",
                "Engine",
                "Landscape",
                "LandscapeEditor",
                "PlacementMode",
                "AdvancedPreviewScene",
                "DesktopPlatform",
                "UnrealEd",
                "InputCore",
                "ImageWrapper",
                "Slate",
                "SlateCore",
                "PropertyEditor",
                "EditorStyle",
                "Projects",
                "RHI",
                "MessageLog",
                "RawMesh",
                "DetailCustomizations",
                "WorkspaceMenuStructure",
                "BlueprintGraph",
                "KismetCompiler",
                "ApplicationCore",
                "EngineSettings",
                "EditorFramework",
#if UE_4_26_OR_LATER
                "DeveloperSettings",
#endif
            });

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "VoxelGraphEditor"
            });
    }
}
