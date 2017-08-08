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
* Download [VoxelContent.zip](https://github.com/Phyronnaz/MarchingCubes/files/1209649/VoxelContent.zip)
* Put the .uassets in your content folder (with editor closed)

![alt text](https://raw.githubusercontent.com/Phyronnaz/MarchingCubes/6cbf169a242dc769330a19b985601c40b6d6a2af/Screenshot.png)
