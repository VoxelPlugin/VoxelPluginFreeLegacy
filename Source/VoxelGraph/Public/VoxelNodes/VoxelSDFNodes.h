// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelSDFNodes.generated.h"

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelSDFNode : public UVoxelPureNode
{
	GENERATED_BODY()

protected:
	UVoxelSDFNode();
	
	void AddPositionInput();
	void AddStartInput();
	void AddEndInput();
	
	void AddNormalInput();

	void AddRadiusInput();
	void AddStartRadiusInput();
	void AddEndRadiusInput();

	void AddLengthInput();
	void AddHeightInput();

	void AddSizeInput();
	void AddSize2DInput();
	void AddSize1DInput();

	void AddSmoothnessInput();
	void AddThicknessInput();

	void AddSinCosInput();
	void AddDistanceAInput();
	void AddDistanceBInput();
};

// Sphere - exact
UCLASS(DisplayName = "Sphere SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_SphereSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SphereSDF();
};

// Box - exact
UCLASS(DisplayName = "Box SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_BoxSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_BoxSDF();
};

// Round Box - exact
UCLASS(DisplayName = "Round Box SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_RoundBoxSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_RoundBoxSDF();
};

// Torus - exact
UCLASS(DisplayName = "Torus SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_TorusSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_TorusSDF();
};

// Capped Torus - exact
UCLASS(DisplayName = "Capped Torus SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_CappedTorusSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_CappedTorusSDF();
};

// Link - exact
UCLASS(DisplayName = "Link SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_LinkSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_LinkSDF();
};

// Infinite Cylinder - exact
UCLASS(DisplayName = "Cylinder SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_CylinderSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_CylinderSDF();
};

// Cone - exact
UCLASS(DisplayName = "Cone SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_ConeSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_ConeSDF();
};

// Cone - bound(not exact!)
UCLASS(DisplayName = "Fast Cone SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_ConeFastSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_ConeFastSDF();
};

// Infinite Cone - exact
UCLASS(DisplayName = "Infinite Cone SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_InfiniteConeSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_InfiniteConeSDF();
};

// Plane - exact
UCLASS(DisplayName = "Plane SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_PlaneSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_PlaneSDF();
};

// Hexagonal Prism - exact
UCLASS(DisplayName = "Hexagonal Prism SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_HexPrismSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_HexPrismSDF();
};

// Triangular Prism - bound
UCLASS(DisplayName = "Triangular Prism SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_TriPrismSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_TriPrismSDF();
};

// Capsule / Line - exact
UCLASS(DisplayName = "Capsule SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_CapsuleSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_CapsuleSDF();
};

// Capsule / Line - exact
UCLASS(DisplayName = "Vertical Capsule SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_VerticalCapsuleSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_VerticalCapsuleSDF();
};

// Capped Cylinder - exact
UCLASS(DisplayName = "Vertical Capped Cylinder SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_VerticalCappedCylinderSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_VerticalCappedCylinderSDF();
};

// Capped Cylinder - exact
UCLASS(DisplayName = "Capped Cylinder SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_CappedCylinderSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_CappedCylinderSDF();
};

// Rounded Cylinder - exact
UCLASS(DisplayName = "Rounded Cylinder SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_RoundedCylinderSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_RoundedCylinderSDF();
};

// Capped Cone - exact
UCLASS(DisplayName = "Vertical Capped Cone SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_VerticalCappedConeSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_VerticalCappedConeSDF();
};

// Capped Cone - exact
UCLASS(DisplayName = "Capped Cone SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_CappedConeSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_CappedConeSDF();
};

// Solid Angle - exact
UCLASS(DisplayName = "Solid Angle SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_SolidAngleSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SolidAngleSDF();
};

// Round cone - exact
UCLASS(DisplayName = "Vertical Round Cone SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_VerticalRoundConeSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_VerticalRoundConeSDF();
};

// Round Cone - exact
UCLASS(DisplayName = "Round Cone SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_RoundConeSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_RoundConeSDF();
};

// Ellipsoid - bound(not exact!)
UCLASS(DisplayName = "Ellipsoid SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_EllipsoidSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_EllipsoidSDF();
};

// Octahedron - exact
UCLASS(DisplayName = "Octahedron SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_OctahedronSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_OctahedronSDF();
};

// Octahedron - bound(not exact)
UCLASS(DisplayName = "Fast Octahedron SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_OctahedronFastSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_OctahedronFastSDF();
};

// Pyramid - exact
UCLASS(DisplayName = "Pyramid SDF", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_PyramidSDF : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_PyramidSDF();
};

///////////////////////////////////////////////////////////////////////////////

// Smooth Union of two SDFs
UCLASS(DisplayName = "Smooth Union", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_SmoothUnion : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SmoothUnion();
};

// Smooth Subtraction of two SDFs
UCLASS(DisplayName = "Smooth Subtraction", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_SmoothSubtraction : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SmoothSubtraction();
};

// Smooth Intersection of two SDFs
UCLASS(DisplayName = "Smooth Intersection", Category = "Distance Field")
class VOXELGRAPH_API UVoxelNode_SmoothIntersection : public UVoxelSDFNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SmoothIntersection();
};