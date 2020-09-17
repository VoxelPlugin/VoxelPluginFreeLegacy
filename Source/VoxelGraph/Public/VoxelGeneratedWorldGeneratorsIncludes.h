// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorPicker.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.h"

#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"

#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.inl"
#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "VoxelAssets/VoxelHeightmapAssetSamplerWrapper.h"
#include "VoxelTools/VoxelHardnessHandler.h"

#include "NodeFunctions/VoxelNodeFunctions.h"
#include "NodeFunctions/VoxelSDFNodeFunctions.h"
#include "NodeFunctions/VoxelMathNodeFunctions.h"
#include "NodeFunctions/VoxelDeprecatedNodeFunctions.h"
#include "NodeFunctions/VoxelPlaceableItemsNodeFunctions.h"

#include "VoxelContext.h"
#include "VoxelMessages.h"
#include "VoxelMaterialBuilder.h"
#include "FastNoise/VoxelFastNoise.h"
#include "FastNoise/VoxelFastNoise.inl"
#include "VoxelGraphGeneratorHelpers.h"
#include "VoxelGeneratedWorldGeneratorsDefinitions.h"

#include "Curves/RichCurve.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "Engine/Texture2D.h"
#include "Containers/StaticArray.h"