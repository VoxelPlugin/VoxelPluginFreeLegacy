Docs: http://wiki.voxelplugin.com
Issues: https://gitlab.com/Phyronnaz/VoxelPluginIssues/issues

Support:
- UE Forums: https://forums.unrealengine.com/community/released-projects/125045-voxel-plugin
- Discord: https://discord.gg/58ZqEbg

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

you are most likely missing VCRUNTIME140_1.dll; you can confirm that by looking at Saved/Logs/Log.txt and checking if it contains something like "Missing import: VCRUNTIME140_1.dll."

To fix this, head up to https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads and download & install vc_redist.x64.exe.