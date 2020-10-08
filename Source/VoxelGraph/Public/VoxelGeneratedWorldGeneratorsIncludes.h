// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelGenerators/VoxelGenerator.h"
#include "VoxelGenerators/VoxelEmptyGenerator.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"

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