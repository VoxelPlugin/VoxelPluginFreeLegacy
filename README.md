# voxel plugin

**voxel plugin** allows to create fully volumetric, entirely destructible, infinite worlds in **Unreal Engine**.
It is compatible with 4.22, 4.23 and 4.24.

## Links

You can get a binary build here: https://gumroad.com/voxelplugin

- Docs: http://wiki.voxelplugin.com
- Issues: https://gitlab.com/Phyronnaz/VoxelPluginIssues/issues

Support:
- UE Forums: https://forums.unrealengine.com/community/released-projects/125045-voxel-plugin
- Discord: https://discord.voxelplugin.com

For more info, check the website: https://voxelplugin.com

## Installation

* In your project's directory, create a folder named Plugins
* Copy the Voxel folder into it. You should have something like
```
    MyProject
    ├── Content
    └── Plugins
        └── Voxel
            └── Binaries
            └── Config
            └── Content
            └── Resources
            └── Source
            └── Voxel.uplugin
```
* If you want to call voxel functions from C++: add **"Voxel"** as public dependency in **MyProject.Build.cs**. You should have
```
    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Voxel" });
```

## Quick start

* Add a **VoxelWorld** to your scene
* Set your player controller to the **VoxelComplexController** (need to toggle *Show Plugin Content*)
* Hit play

## Plugin Voxel failed to load

If you are getting:
> Plugin 'Voxel' failed to load because module 'Voxel' could not be loaded. There may be an operating system error or the module may not be properly set up.

you are most likely missing **VCRUNTIME140_1.dll**; you can confirm that by looking at Saved/Logs/Log.txt and checking if it contains something like _Missing import: VCRUNTIME140_1.dll_.

To fix this, head up to https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads and download & install **vc_redist.x64.exe**.

## Pricing & Licensing

There are two editions of the plugin available:

* a free version (this one), with limited features
* a pro version with more features, which you can check out [on the website](https://voxelplugin.com)

Both versions can be used commercially. The API of the two versions is the same, so files created in one can be used in another.

## Features

### Free version
* Fully volumetric and entirely destructible terrain
* Infinite worlds with seamless LODs
* Ultra smooth edits, with hundreds of edits per seconds
* VR Ready: everything is done in background threads, allowing for a constant 90fps
* Up to 256 materials, with smooth transitions and texture support
* Complete Blueprint interface
* C++ World Generators to create your own worlds
* Basic Multiplayer
* Powerful editor tools

### Pro version
* Easy to use custom graph system to create your own worlds
* TCP Multiplayer
* Importers: import from meshes, heightmaps, landscapes, MagicaVox, 3D Coat and splines
* Voxel Physics: enable physics on floating voxel blocks
* Powerful procedural spawning system
* Mesh tool: use meshes as stamps to sculpt
* Pro Support

## Screenshots
![Materials](https://i.imgur.com/du0V5i6.png)
![Fast Edits](https://i.imgur.com/4ouIrX9.png)
![Procedural Spawning (Pro only)](https://i.imgur.com/0Ic3o6h.png)
![Infinite World](https://i.imgur.com/hvfXNob.png)
![Multiplayer](https://i.imgur.com/wEMPfYm.png)
![Entirely Destructible](https://i.imgur.com/v3zjXQj.png)
![Import Meshes, Heightmaps, Landscape, MagicaVox assets, 3D Coat assets (Pro only)](https://i.imgur.com/YiX8afI.png)
![Fully Volumetric](https://i.imgur.com/9tH7yVv.png)
![Voxel Physics (Pro only)](https://i.imgur.com/c6Zc4oD.png)
