# Voxel Plugin

**Voxel Plugin** allows to create fully volumetric, entirely destructible, infinite worlds in **Unreal Engine**.
It is compatible with 4.23, 4.24 and 4.25.

## Links

- Unreal Marketplace: https://www.unrealengine.com/marketplace/product/voxel-plugin-free
- Docs: http://wiki.voxelplugin.com
- Forums: http://forums.voxelplugin.com
- Website: https://voxelplugin.com
- Twitter: https://twitter.com/voxelplugin
- Youtube: http://youtube.com/voxelplugin
- Discord: https://discord.voxelplugin.com
- UE Forums: https://forums.unrealengine.com/community/released-projects/125045-voxel-plugin

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
            └── VoxelFree.uplugin
```
* If you want to call voxel functions from C++: add **"Voxel"** as public dependency in **MyProject.Build.cs**. You should have
```
    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Voxel" });
```

## Screenshots
![Materials](https://i.imgur.com/XW4o2g3.png)
![Fast Edits](https://i.imgur.com/4ouIrX9.png)
![Infinite World](https://i.imgur.com/hvfXNob.png)
![Multiplayer](https://i.imgur.com/wEMPfYm.png)
![Entirely Destructible](https://i.imgur.com/v3zjXQj.png)
![Import Meshes, Heightmaps, Landscape, MagicaVox assets, 3D Coat assets (Pro only)](https://i.imgur.com/YiX8afI.png)
![Fully Volumetric](https://i.imgur.com/9tH7yVv.png)
![Voxel Physics (Pro only)](https://i.imgur.com/c6Zc4oD.png)
