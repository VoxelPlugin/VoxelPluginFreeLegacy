// Copyright 2020 Phyronnaz

#include "VoxelModule.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"

#include "Interfaces/IPluginManager.h"
#include "ShaderCore.h"
#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"

void FVoxelModule::StartupModule()
{
	{
		FVoxelMaterial Material(ForceInit);
		const auto CheckUV = [&](int32 Tex)
		{
			for (int32 Value = 0; Value < 256; Value++)
			{
#define checkBytesEqual(A, B) checkf(A == B, TEXT("%d == %d"), A, B);
				
				Material.SetU(Tex, Value);
				checkBytesEqual(Material.GetU(Tex), Value);

				Material.SetV(Tex, 255 - Value);
				checkBytesEqual(Material.GetV(Tex), 255 - Value);

				Material.SetV(Tex, Value);
				checkBytesEqual(Material.GetV(Tex), Value);
				
				Material.SetU(Tex, 255 - Value);
				checkBytesEqual(Material.GetU(Tex), 255 - Value);

				Material.SetU_AsFloat(Tex, 0.5f);
				check(Material.GetU_AsFloat(Tex) == FVoxelUtilities::UINT8ToFloat(FVoxelUtilities::FloatToUINT8(0.5f)));

				Material.SetV_AsFloat(Tex, 0.5f);
				check(Material.GetV_AsFloat(Tex) == FVoxelUtilities::UINT8ToFloat(FVoxelUtilities::FloatToUINT8(0.5f)));

#undef checkBytesEqual

				Material.SetUV_AsFloat(Tex, FVector2D(1.f, 1.f));
				checkf(Material.GetUV_AsFloat(Tex) == FVector2D(1.f, 1.f), TEXT("%s"), *Material.GetUV_AsFloat(Tex).ToString());
			}
		};
#if VOXEL_MATERIAL_ENABLE_UV0
		CheckUV(0);
#endif
#if VOXEL_MATERIAL_ENABLE_UV1
		CheckUV(1);
#endif
#if VOXEL_MATERIAL_ENABLE_UV2
		CheckUV(2);
#endif
#if VOXEL_MATERIAL_ENABLE_UV3
		CheckUV(3);
#endif
	}
	
}

void FVoxelModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FVoxelModule, Voxel)