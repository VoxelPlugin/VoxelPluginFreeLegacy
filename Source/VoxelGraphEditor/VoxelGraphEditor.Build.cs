// Copyright 2020 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelGraphEditor : ModuleRules
{
    public VoxelGraphEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;


        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        
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