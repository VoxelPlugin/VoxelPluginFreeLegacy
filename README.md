# UE4 Voxels

<b>Requires 4.17 (async collision cooking)</b>

Based on [transvoxel](http://transvoxel.org/)

https://www.youtube.com/playlist?list=PLjYKpKPK8E-_lxP5EuRxizhNuNBDYnT15

https://forums.unrealengine.com/showthread.php?151940-Voxel-plugin-for-UE4

## Features:
* Real time collisions
* Smooth LOD
* Fast (update of a chunk cost less than 1ms)
* Blueprint interface
* Textures
* Multithreaded
* Easy generation of custom worlds
* Free and open source
* Multiplayer
* Landscape import
* Easy load/save
* Up to 256 materials

## TODO:
* Some holes still appears
* ~~Better mesh import~~
* Material tuto
* ~~16 textures max per material -> TextureArrays?~~ Shared texture should do
* ~~Better quality settings~~

## Example project
Download repo and launch .uproject

## Plugin
* Create a `Plugins` folder in your game directory
* Download latest Voxel.zip from [releases](https://github.com/Phyronnaz/MarchingCubes/releases) and extract it in the `Plugins` folder
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
