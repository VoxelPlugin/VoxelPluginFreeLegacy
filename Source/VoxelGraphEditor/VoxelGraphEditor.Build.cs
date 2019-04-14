// Copyright 2019 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelGraphEditor : ModuleRules
{
    public VoxelGraphEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

        if (!Target.bUseUnityBuild)
        {
            PrivatePCHHeaderFile = "Private/VoxelGraphEditorPCH.h";
#if UE_4_22_OR_LATER
#else
            PrivateDependencyModuleNames.Add("LivePP");
#endif
        }

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "PropertyEditor",
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
                "GameProjectGeneration"
            });

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
            });

        if (Target.Configuration == UnrealTargetConfiguration.DebugGame)
        {
            PublicDefinitions.Add("VOXEL_DEBUG=1");
        }
    }
}
