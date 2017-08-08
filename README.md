# UE4 Voxels

<b>Requires 4.17 (async collision cooking)</b>

Demo: https://www.youtube.com/watch?v=oLDlM8-SnoI

## Example project
Download repo and launch .uproject

## Plugin
* Create a `Plugins` folder in your game directory
* Download [Voxel.zip](https://github.com/Phyronnaz/MarchingCubes/files/1209648/Voxel.zip) and extract it in the `Plugins` folder
* Make sure you have `MyProject/Plugins/Voxel/Voxel.uplugin` and not `MyProject/Plugins/Voxel/Voxel/Voxel.uplugin`
* If C++: add `"Voxel"` as public dependency in `MyProject.Build.cs` (should be `PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Voxel" });`)
* Enable the plugin in the editor and restart

## Content
Download [VoxelContent.zip](https://github.com/Phyronnaz/MarchingCubes/files/1209649/VoxelContent.zip) and put the .uassets in your content folder (with editor closed)

## Quick start
* Add a Voxel World actor to your scene (with the left panel)
* Set the material of the world
* Create a new GameMode and set the player controller to VoxelPlayerController

## Generate custom world
### Blueprint
Create a blueprint subclass of FlatWorldGenerator or SphereWorldGenerator

### C++
Create a subclass of UVoxelWorldGenerator and implement GetDefaultValue and GetDefaultColor. You can look at [FlatWorldGenerator.h](https://github.com/Phyronnaz/MarchingCubes/blob/768563eaaf43d47c4cb73723b907ccd520f0d9ac/Plugins/Voxel/Source/Voxel/Classes/FlatWorldGenerator.h) for an example.

![alt text](https://raw.githubusercontent.com/Phyronnaz/MarchingCubes/6cbf169a242dc769330a19b985601c40b6d6a2af/Screenshot.png)
