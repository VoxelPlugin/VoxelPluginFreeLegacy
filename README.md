# UE4 Voxels

<b>Requires 4.17 (async collision cooking)</b>

Demo: https://www.youtube.com/watch?v=oLDlM8-SnoI

Performance test: https://youtu.be/KD1MkJtnL_A

## Example project
Download repo and launch .uproject

## Plugin
* Create a `Plugins` folder in your game directory
* Download [Voxel.zip](https://github.com/Phyronnaz/MarchingCubes/files/1265244/Voxel.zip) and extract it in the `Plugins` folder
* Make sure you have `MyProject/Plugins/Voxel/Voxel.uplugin` and not `MyProject/Plugins/Voxel/Voxel/Voxel.uplugin`
* If C++: add `"Voxel"` as public dependency in `MyProject.Build.cs` (should be `PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Voxel" });`)
* Enable the plugin in the editor and restart

## Assets
Download repo and 
[migrate](https://docs.unrealengine.com/latest/INT/Engine/Content/Browser/UserGuide/Migrate/)
them

## Quick start
* Add a Voxel World actor to your scene (with the left panel)
* Set the material of the world
* Create a new GameMode and set the player controller to VoxelPlayerController

## Generate custom world
### Blueprint
Create a blueprint and in Class Settings, add VoxelWorldGenerator as interface (see [here](https://docs.unrealengine.com/latest/INT/Engine/Blueprints/UserGuide/Types/Interface/UsingInterfaces/))

### C++
Create a subclass of UObject and implement IVoxelWorldGenerator. You can look at 
[CirclesWorldGenerator.h](https://github.com/Phyronnaz/MarchingCubes/blob/master/Source/Procedural/CirclesWorldGenerator.h) and
[CirclesWorldGenerator.cpp](https://github.com/Phyronnaz/MarchingCubes/blob/master/Source/Procedural/CirclesWorldGenerator.cpp)
for an example.

![alt text](https://raw.githubusercontent.com/Phyronnaz/MarchingCubes/master/Screenshot2.png)
![alt text](https://raw.githubusercontent.com/Phyronnaz/MarchingCubes/master/Screenshot.png)
