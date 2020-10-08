// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelMinimal.h"

#include "UnrealEd.h"
#include "IDetailGroup.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "IPropertyUtilities.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SNumericEntryBox.h"

#include "UserInterface/PropertyEditor/PropertyEditorConstants.h"

namespace FVoxelEditorUtilities
{
	FSimpleDelegate MakeRefreshDelegate(const IPropertyTypeCustomizationUtils& CustomizationUtils);
};