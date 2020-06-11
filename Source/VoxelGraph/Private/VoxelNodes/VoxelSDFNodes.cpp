// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelSDFNodes.h"
#include "Runtime/VoxelNodeType.h"
#include "VoxelContext.h"
#include "VoxelSDFNodeFunctions.h"

UVoxelSDFNode::UVoxelSDFNode()
{
	AddOutput("", "Result");
}

void UVoxelSDFNode::AddPositionInput()
{
	AddVectorInput("Position", "Position to sample the distance function at. Usually X Y Z, with possible some symmetries or perturb applied to them.");
}

void UVoxelSDFNode::AddStartInput()
{
	AddVectorInput("Start", "Start of the shape");
}

void UVoxelSDFNode::AddEndInput()
{
	AddVectorInput("End", "End of the shape");
}

void UVoxelSDFNode::AddNormalInput()
{
	AddVectorInput("Normal", "Normal/direction of the shape. Must be normalized!");
}

void UVoxelSDFNode::AddRadiusInput()
{
	AddInput("Radius", "Radius of the shape");
}

void UVoxelSDFNode::AddStartRadiusInput()
{
	AddInput("Start Radius", "Radius at the start of the shape");
}

void UVoxelSDFNode::AddEndRadiusInput()
{
	AddInput("End Radius", "Radius at the end of the shape");
}

void UVoxelSDFNode::AddLengthInput()
{
	AddInput("Length", "Length of the shape");
}

void UVoxelSDFNode::AddHeightInput()
{
	AddInput("Height", "Height of the shape");
}

void UVoxelSDFNode::AddSizeInput()
{
	AddVectorInput("Size", "Size of the shape");
}

void UVoxelSDFNode::AddSize2DInput()
{
	AddVector2DInput("Size", "Size of the shape");
}

void UVoxelSDFNode::AddSize1DInput()
{
	AddInput("Size", "Size of the shape");
}

void UVoxelSDFNode::AddSmoothnessInput()
{
	AddInput("Smoothness", "Controls the smoothness. In the same unit as the size/position input: if you have a size of 100, a smoothness of 30 will result in roughly a 30% displacement.");
}

void UVoxelSDFNode::AddThicknessInput()
{
	AddInput("Thickness", "Thickness of the shape.");
}

void UVoxelSDFNode::AddSinCosInput()
{
	AddInput("Sin Angle", "Sinus of the angle. Use the SIN COS node to get it.");
	AddInput("Cos Angle", "Cosinus of the angle. Use the SIN COS node to get it.");
}

void UVoxelSDFNode::AddDistanceAInput()
{
	AddInput("Distance A", "SDF A");
}

void UVoxelSDFNode::AddDistanceBInput()
{
	AddInput("Distance B", "SDF B");
}

///////////////////////////////////////////////////////////////////////////////

#define DEFINE_SDF_INPUTS_0 DEFINE_INPUTS()
#define DEFINE_SDF_INPUTS_1 DEFINE_INPUTS(v_flt)
#define DEFINE_SDF_INPUTS_2 DEFINE_INPUTS(v_flt, v_flt)
#define DEFINE_SDF_INPUTS_3 DEFINE_INPUTS(v_flt, v_flt, v_flt)
#define DEFINE_SDF_INPUTS_4 DEFINE_INPUTS(v_flt, v_flt, v_flt, v_flt)
#define DEFINE_SDF_INPUTS_5 DEFINE_INPUTS(v_flt, v_flt, v_flt, v_flt, v_flt)
#define DEFINE_SDF_INPUTS_6 DEFINE_INPUTS(v_flt, v_flt, v_flt, v_flt, v_flt, v_flt)
#define DEFINE_SDF_INPUTS_7 DEFINE_INPUTS(v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt)
#define DEFINE_SDF_INPUTS_8 DEFINE_INPUTS(v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt)
#define DEFINE_SDF_INPUTS_9 DEFINE_INPUTS(v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt)
#define DEFINE_SDF_INPUTS_10 DEFINE_INPUTS(v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt)
#define DEFINE_SDF_INPUTS_11 DEFINE_INPUTS(v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt)

#define SDF_ARGS_0
#define SDF_ARGS_1 Input0
#define SDF_ARGS_2 Input0, Input1
#define SDF_ARGS_3 Input0, Input1, Input2
#define SDF_ARGS_4 Input0, Input1, Input2, Input3
#define SDF_ARGS_5 Input0, Input1, Input2, Input3, Input4
#define SDF_ARGS_6 Input0, Input1, Input2, Input3, Input4, Input5
#define SDF_ARGS_7 Input0, Input1, Input2, Input3, Input4, Input5, Input6
#define SDF_ARGS_8 Input0, Input1, Input2, Input3, Input4, Input5, Input6, Input7
#define SDF_ARGS_9 Input0, Input1, Input2, Input3, Input4, Input5, Input6, Input7, Input8
#define SDF_ARGS_10 Input0, Input1, Input2, Input3, Input4, Input5, Input6, Input7, Input8, Input9
#define SDF_ARGS_11 Input0, Input1, Input2, Input3, Input4, Input5, Input6, Input7, Input8, Input9, Input10

#define GENERATED_SDF_NODE_IMPL_EX(Name, ClassName, Count) \
	GENERATED_VOXELNODE_IMPL \
	( \
		ClassName, \
		DEFINE_SDF_INPUTS_##Count, \
		DEFINE_OUTPUTS(v_flt), \
		Output0 = FVoxelSDFNodeFunctions::Name(SDF_ARGS_##Count); \
	)

#define GENERATED_SDF_NODE_IMPL(Name, Count) GENERATED_SDF_NODE_IMPL_EX(Name, UVoxelNode_##Name##SDF, Count)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SphereSDF::UVoxelNode_SphereSDF()
{
	AddPositionInput();
	AddRadiusInput();
}
GENERATED_SDF_NODE_IMPL(Sphere, 4)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_BoxSDF::UVoxelNode_BoxSDF()
{
	AddPositionInput();
	AddSizeInput();
}
GENERATED_SDF_NODE_IMPL(Box, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_RoundBoxSDF::UVoxelNode_RoundBoxSDF()
{
	AddPositionInput();
	AddSizeInput();
	AddSmoothnessInput();
}
GENERATED_SDF_NODE_IMPL(RoundBox, 7)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_TorusSDF::UVoxelNode_TorusSDF()
{
	AddPositionInput();
	AddRadiusInput();
	AddThicknessInput();
}
GENERATED_SDF_NODE_IMPL(Torus, 5)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_CappedTorusSDF::UVoxelNode_CappedTorusSDF()
{
	AddPositionInput();
	AddSinCosInput();
	AddRadiusInput();
	AddThicknessInput();
}
GENERATED_SDF_NODE_IMPL(CappedTorus, 7)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_LinkSDF::UVoxelNode_LinkSDF()
{
	AddPositionInput();
	AddLengthInput();
	AddRadiusInput();
	AddThicknessInput();
}
GENERATED_SDF_NODE_IMPL(Link, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_CylinderSDF::UVoxelNode_CylinderSDF()
{
	AddPositionInput();
	AddSize2DInput();
	AddThicknessInput();
}
GENERATED_SDF_NODE_IMPL(Cylinder, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_ConeSDF::UVoxelNode_ConeSDF()
{
	AddPositionInput();
	AddSinCosInput();
	AddHeightInput();
}
GENERATED_SDF_NODE_IMPL(Cone, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_ConeFastSDF::UVoxelNode_ConeFastSDF()
{
	AddPositionInput();
	AddSinCosInput();
	AddHeightInput();
}
GENERATED_SDF_NODE_IMPL(ConeFast, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_InfiniteConeSDF::UVoxelNode_InfiniteConeSDF()
{
	AddPositionInput();
	AddSinCosInput();
}
GENERATED_SDF_NODE_IMPL_EX(Cone, UVoxelNode_InfiniteConeSDF, 5)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_PlaneSDF::UVoxelNode_PlaneSDF()
{
	AddPositionInput();
	AddNormalInput();
}
GENERATED_SDF_NODE_IMPL(Plane, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_HexPrismSDF::UVoxelNode_HexPrismSDF()
{
	AddPositionInput();
	AddRadiusInput();
	AddThicknessInput();
}
GENERATED_SDF_NODE_IMPL(HexPrism, 5)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_TriPrismSDF::UVoxelNode_TriPrismSDF()
{
	AddPositionInput();
	AddRadiusInput();
	AddThicknessInput();
}
GENERATED_SDF_NODE_IMPL(TriPrism, 5)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_CapsuleSDF::UVoxelNode_CapsuleSDF()
{
	AddPositionInput();
	AddStartInput();
	AddEndInput();
	AddRadiusInput();
}
GENERATED_SDF_NODE_IMPL(Capsule, 10)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_VerticalCapsuleSDF::UVoxelNode_VerticalCapsuleSDF()
{
	AddPositionInput();
	AddHeightInput();
	AddRadiusInput();
}
GENERATED_SDF_NODE_IMPL(VerticalCapsule, 5)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_VerticalCappedCylinderSDF::UVoxelNode_VerticalCappedCylinderSDF()
{
	AddPositionInput();
	AddHeightInput();
	AddRadiusInput();
}
GENERATED_SDF_NODE_IMPL_EX(CappedCylinder, UVoxelNode_VerticalCappedCylinderSDF, 5)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_CappedCylinderSDF::UVoxelNode_CappedCylinderSDF()
{
	AddPositionInput();
	AddStartInput();
	AddEndInput();
	AddRadiusInput();
}
GENERATED_SDF_NODE_IMPL(CappedCylinder, 10)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_RoundedCylinderSDF::UVoxelNode_RoundedCylinderSDF()
{
	AddPositionInput();
	AddRadiusInput();
	AddSmoothnessInput();
	AddHeightInput();
}
GENERATED_SDF_NODE_IMPL(RoundedCylinder, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_VerticalCappedConeSDF::UVoxelNode_VerticalCappedConeSDF()
{
	AddPositionInput();
	AddHeightInput();
	AddStartInput();
	AddEndInput();
}
GENERATED_SDF_NODE_IMPL_EX(CappedCone, UVoxelNode_VerticalCappedConeSDF, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_CappedConeSDF::UVoxelNode_CappedConeSDF()
{
	AddPositionInput();
	AddStartInput();
	AddEndInput();
	AddStartRadiusInput();
	AddEndRadiusInput();
}
GENERATED_SDF_NODE_IMPL(CappedCone, 11)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SolidAngleSDF::UVoxelNode_SolidAngleSDF()
{
	AddPositionInput();
	AddSinCosInput();
	AddRadiusInput();
}
GENERATED_SDF_NODE_IMPL(SolidAngle, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_VerticalRoundConeSDF::UVoxelNode_VerticalRoundConeSDF()
{
	AddPositionInput();
	AddStartRadiusInput();
	AddEndRadiusInput();
	AddHeightInput();
}
GENERATED_SDF_NODE_IMPL_EX(RoundCone, UVoxelNode_VerticalRoundConeSDF, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_RoundConeSDF::UVoxelNode_RoundConeSDF()
{
	AddPositionInput();
	AddStartInput();
	AddEndInput();
	AddStartRadiusInput();
	AddEndRadiusInput();
}
GENERATED_SDF_NODE_IMPL(RoundCone, 11)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_EllipsoidSDF::UVoxelNode_EllipsoidSDF()
{
	AddPositionInput();
	AddSizeInput();
}
GENERATED_SDF_NODE_IMPL(Ellipsoid, 6)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_OctahedronSDF::UVoxelNode_OctahedronSDF()
{
	AddPositionInput();
	AddSize1DInput();
}
GENERATED_SDF_NODE_IMPL(Octahedron, 4)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_OctahedronFastSDF::UVoxelNode_OctahedronFastSDF()
{
	AddPositionInput();
	AddSize1DInput();
}
GENERATED_SDF_NODE_IMPL(OctahedronFast, 4)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_PyramidSDF::UVoxelNode_PyramidSDF()
{
	AddPositionInput();
	AddHeightInput();
}
GENERATED_SDF_NODE_IMPL(Pyramid, 4)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SmoothUnion::UVoxelNode_SmoothUnion()
{
	AddDistanceAInput();
	AddDistanceBInput();
	AddSmoothnessInput();
}
GENERATED_SDF_NODE_IMPL_EX(SmoothUnion, UVoxelNode_SmoothUnion, 3)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SmoothSubtraction::UVoxelNode_SmoothSubtraction()
{
	AddDistanceAInput();
	AddDistanceBInput();
	AddSmoothnessInput();
}
GENERATED_SDF_NODE_IMPL_EX(SmoothSubtraction, UVoxelNode_SmoothSubtraction, 3)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SmoothIntersection::UVoxelNode_SmoothIntersection()
{
	AddDistanceAInput();
	AddDistanceBInput();
	AddSmoothnessInput();
}
GENERATED_SDF_NODE_IMPL_EX(SmoothIntersection, UVoxelNode_SmoothIntersection, 3)