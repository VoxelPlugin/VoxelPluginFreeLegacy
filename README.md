# voxel plugin™

**voxel plugin™** allows to create fully volumetric, entirely destructible, infinite worlds in **Unreal Engine**.
It is compatible with 4.20 and 4.21.

For more info, check the website: https://voxelplugin.com

## Installation
* In your game’s root directory, create a folder named Plugins
* Copy the Voxel folder into it. You should have something like
```
    MyProject
    ├── Content
    └── Plugins
        └── Voxel
            └── Voxel.uplugin
```
* If you want to use it in your C++ project: add **"Voxel"** as public dependency in **MyProject.Build.cs**. You should have
```
    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Voxel" });
```
## Quick start
Follow [this video](https://www.youtube.com/watch?v=EpXu9kqFoSM)

## Pricing & Licensing

There are two editions of the plugin available:

* a free version (this one), with limited features
* a pro version with more features, which you can check out [on the website](https://voxelplugin.com)

Both versions can be used commercially.

## Features

### Free version
* Fully volumetric and entirely destructible terrain
* Infinite worlds with seamless LODs
* Ultra smooth edits: up to 50 times per second on a single chunk
* VR Ready: everything is done in background threads, allowing for a constant 90fps
* Up to 256 materials, with smooth transitions and texture support through Texture Arrays
* Complete Blueprint interface
* C++ World Generators to create your own worlds

### Pro version
* Easy to use custom graph system to create your own worlds
* Multiplayer
* Importers: import from meshes, heightmaps, landscapes, MagicaVox, 3D Coat and splines
* Voxel Physics: enable physics on floating voxel blocks
* Grass and Voxel Actors
* Pro Support

## Docs & Support
The docs of the plugin are hosted at https://voxel-plugin.readthedocs.io/en/latest/

To get support, you can either:
* join the discord: https://discord.gg/58ZqEbg
* leave a message on the UE forums: https://forums.unrealengine.com/community/released-projects/125045-voxel-plugin

For issues, please use https://gitlab.com/Phyronnaz/VoxelPluginIssues/issues

## Screenshots
![Materials](https://i.imgur.com/du0V5i6.png)
![50Hz Edits](https://i.imgur.com/4ouIrX9.png)
![Grass & Voxel Actors (Pro only)](https://i.imgur.com/0Ic3o6h.png)
![Infinite World](https://i.imgur.com/hvfXNob.png)
![Multiplayer (Pro only)](https://i.imgur.com/wEMPfYm.png)
![Entirely Destructible](https://i.imgur.com/v3zjXQj.png)
![Import Meshes, Heightmaps, Landscape, Splines, MagicaVox assets, 3D Coat assets (Pro only)](https://i.imgur.com/YiX8afI.png)
![Fully Volumetric](https://i.imgur.com/9tH7yVv.png)
![Voxel Physics (Pro only)](https://i.imgur.com/c6Zc4oD.png)
