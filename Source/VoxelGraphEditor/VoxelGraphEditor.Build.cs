// Copyright Voxel Plugin SAS. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class VoxelGraphEditor : ModuleRules
{
    public VoxelGraphEditor(ReadOnlyTargetRules Target) : base(Target)
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
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "AssetRegistry"
            });

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "Voxel",
                "VoxelGraph",
                "KismetWidgets",
                "AdvancedPreviewScene",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "InputCore",
                "ApplicationCore",
                "GraphEditor",
                "EditorStyle",
                "Projects",
                "BlueprintGraph",
                "DesktopPlatform",
                "Json",
                "GameProjectGeneration",
                "MessageLog",
                "AppFramework",
                "PropertyEditor",
                "EditorFramework",
#if UE_4_24_OR_LATER
                "ToolMenus"
#endif
            });

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "VoxelEditor"
            });
    }
}