// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelMathNodes.generated.h"

// Max
UCLASS(DisplayName = "Max (float)", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_FMax : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_FMax();
};

// Min
UCLASS(DisplayName = "Min (float)", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_FMin : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_FMin();
};

// Max
UCLASS(DisplayName = "Max (int)", Category = "Math|Integer")
class VOXELGRAPH_API UVoxelNode_IMax : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_IMax();
};

// Min
UCLASS(DisplayName = "Min (int)", Category = "Math|Integer")
class VOXELGRAPH_API UVoxelNode_IMin : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_IMin();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Add
UCLASS(DisplayName = "float + float", Category = "Math|Float", meta = (Keywords = "+ add plus"))
class VOXELGRAPH_API UVoxelNode_FAdd : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("+")

	UVoxelNode_FAdd();
};

// Multiply
UCLASS(DisplayName = "float * float", Category = "Math|Float", meta = (Keywords = "* multiply"))
class VOXELGRAPH_API UVoxelNode_FMultiply : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("*")

	UVoxelNode_FMultiply();
};

//////////////////////////////////////////////////////////////////////////////////////

// Subtract
UCLASS(DisplayName = "float - float", Category = "Math|Float", meta = (Keywords = "- subtract minus"))
class VOXELGRAPH_API UVoxelNode_FSubstract : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("-")

	UVoxelNode_FSubstract();
};

// Divide
UCLASS(DisplayName = "float / float", Category = "Math|Float", meta = (Keywords = "/ divide division"))
class VOXELGRAPH_API UVoxelNode_FDivide : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("/")

	UVoxelNode_FDivide();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Add
UCLASS(DisplayName = "int + int", Category = "Math|Integer", meta = (Keywords = "+ add plus"))
class VOXELGRAPH_API UVoxelNode_IAdd : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("+")

	UVoxelNode_IAdd();
};

// Multiply
UCLASS(DisplayName = "int * int", Category = "Math|Integer", meta = (Keywords = "* multiply"))
class VOXELGRAPH_API UVoxelNode_IMultiply : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("*")

	UVoxelNode_IMultiply();
};

//////////////////////////////////////////////////////////////////////////////////////

// Subtract
UCLASS(DisplayName = "int - int", Category = "Math|Integer", meta = (Keywords = "- subtract minus"))
class VOXELGRAPH_API UVoxelNode_ISubstract : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("-")

	UVoxelNode_ISubstract();
};

// Divide
UCLASS(DisplayName = "int / int", Category = "Math|Integer", meta = (Keywords = "/ divide division"))
class VOXELGRAPH_API UVoxelNode_IDivide : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("/")

	UVoxelNode_IDivide();
};

// Left bit shift
UCLASS(DisplayName = "<<", Category = "Math|Integer", meta = (Keywords = "<< left bit shift"))
class VOXELGRAPH_API UVoxelNode_ILeftBitShift : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("<<")

	UVoxelNode_ILeftBitShift();
};

// Right bit shift
UCLASS(DisplayName = ">>", Category = "Math|Integer", meta = (Keywords = ">> right bit shift"))
class VOXELGRAPH_API UVoxelNode_IRightBitShift : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE(">>")

	UVoxelNode_IRightBitShift();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Cast to float
UCLASS(DisplayName = "int to float", Category = "Math|Integer", meta = (Keywords = "cast convert"))
class VOXELGRAPH_API UVoxelNode_FloatOfInt : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("float")

	UVoxelNode_FloatOfInt();
};

// Round to int32
UCLASS(DisplayName = "Round", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Round : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("Round")

	UVoxelNode_Round();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Lerp between A and B. Warning: Alpha not clamped! Lerp(0, 20, 2) = 40!
UCLASS(DisplayName = "Lerp", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Lerp : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_Lerp();
};

// Lerp between A and B, with a clamped alpha
UCLASS(DisplayName = "Safe Lerp", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_SafeLerp : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SafeLerp();
};

// Clamp Value between Min and Max
UCLASS(DisplayName = "Clamp", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Clamp : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_Clamp();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// And
UCLASS(DisplayName = "AND Boolean", Category = "Math|Boolean", meta = (Keywords = "& and"))
class VOXELGRAPH_API UVoxelNode_BAnd : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("AND")

	UVoxelNode_BAnd();
};

// Or
UCLASS(DisplayName = "OR Boolean", Category = "Math|Boolean", meta = (Keywords = "| or"))
class VOXELGRAPH_API UVoxelNode_BOr : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("OR")

	UVoxelNode_BOr();
};

// Not
UCLASS(DisplayName = "NOT Boolean", Category = "Math|Boolean", meta = (Keywords = "! not"))
class VOXELGRAPH_API UVoxelNode_BNot : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("NOT")

	UVoxelNode_BNot();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Switch: if bool is true, then A is returned, else B is
UCLASS(DisplayName = "Switch (int)", Category = "Math|Integer", meta = (Keywords = "if branch select"))
class VOXELGRAPH_API UVoxelNode_SwitchInt : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SwitchInt();
};

// Switch: if bool is true, then A is returned, else B is
UCLASS(DisplayName = "Switch (float)", Category = "Math|Float", meta = (Keywords = "if branch select"))
class VOXELGRAPH_API UVoxelNode_SwitchFloat : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SwitchFloat();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// 1 - X
UCLASS(DisplayName = "1 - X", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_1MinusX : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("1 - X")

	UVoxelNode_1MinusX();
};

// 1 / X
UCLASS(DisplayName = "1 / X", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_OneOverX : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("1 / X")

	UVoxelNode_OneOverX();
};

// -X
UCLASS(DisplayName = "* -1", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_MinusX : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("* -1")

	UVoxelNode_MinusX();
};

// Square root
UCLASS(DisplayName = "Sqrt", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Sqrt : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("SQRT")

	UVoxelNode_Sqrt();
};

// Pow
UCLASS(DisplayName = "Pow", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Pow : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("POW")

	UVoxelNode_Pow();
};

// Modulo
UCLASS(DisplayName = "% (int)", Category = "Math|Integer", meta = (Keywords = "% modulus"))
class VOXELGRAPH_API UVoxelNode_IMod : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("%")

	UVoxelNode_IMod();
};

/**
 * Returns the floating-point remainder of X / Y
 * Warning: Always returns remainder toward 0, not toward the smaller multiple of Y.
 *			So for example Fmod(2.8f, 2) gives .8f as you would expect, however, Fmod(-2.8f, 2) gives -.8f, NOT 1.2f
 * Use Floor instead when snapping positions that can be negative to a grid
 */
UCLASS(DisplayName = "FMod", Category = "Math|Float", meta = (Keywords = "% modulus fmod"))
class VOXELGRAPH_API UVoxelNode_FMod : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("FMOD")

	UVoxelNode_FMod();
};

// Absolute value
UCLASS(DisplayName = "Absolute (float)", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_FAbs : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("ABS")

	UVoxelNode_FAbs();
};

// Absolute value
UCLASS(DisplayName = "Absolute (int)", Category = "Math|Integer")
class VOXELGRAPH_API UVoxelNode_IAbs : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("ABS")

	UVoxelNode_IAbs();
};

// Returns the smallest integer greater than or equal to the input
UCLASS(DisplayName = "Ceil", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Ceil : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("CEIL")

	UVoxelNode_Ceil();
};

// Returns the largest integer less than or equal to the input
UCLASS(DisplayName = "Floor", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Floor : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("FLOOR")

	UVoxelNode_Floor();
};

// Return the length of (X, Y, Z)
UCLASS(DisplayName = "Vector Length", Category = "Math|Vector")
class VOXELGRAPH_API UVoxelNode_VectorLength : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_VectorLength();
};

// Return the signed fractional part of the input (ie 9.45 -> 0.45). Negative if the input is negative
UCLASS(DisplayName = "Fraction", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Fraction : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("FRACT")

	UVoxelNode_Fraction();
};

// Return the sign of the input (1, 0 or -1)
UCLASS(DisplayName = "Sign (float)", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_FSign : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("SIGN")

	UVoxelNode_FSign();
};

// Return the sign of the input (1, 0 or -1)
UCLASS(DisplayName = "Sign (int)", Category = "Math|Integer")
class VOXELGRAPH_API UVoxelNode_ISign : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("SIGN")

	UVoxelNode_ISign();
};

// Return the inverse square root of the input
UCLASS(DisplayName = "InvSqrt", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_InvSqrt : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("INVSQRT")

	UVoxelNode_InvSqrt();
};

// Return the loge of the input
UCLASS(DisplayName = "Loge", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Loge : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("LOGE")

	UVoxelNode_Loge();
};

// Return the exponential of the input
UCLASS(DisplayName = "Exp", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Exp : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("EXP")

	UVoxelNode_Exp();
};

// Return the sine of the input
UCLASS(DisplayName = "Sin", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Sin : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("SIN")

	UVoxelNode_Sin();
};

// Return the asin (inverse of sine) of the input
UCLASS(DisplayName = "Asin", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Asin : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("ASIN")

	UVoxelNode_Asin();
};

// Return the sinh (hyperbolic sine) of the input
UCLASS(DisplayName = "Sinh", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Sinh : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("SINH")

	UVoxelNode_Sinh();
};

// Return the cosine of the input
UCLASS(DisplayName = "Cos", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Cos : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("COS")

	UVoxelNode_Cos();
};

// Return the acos (inverse of cosine) of the input
UCLASS(DisplayName = "Acos", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Acos : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("ACOS")

	UVoxelNode_Acos();
};

// Return the tan of the input
UCLASS(DisplayName = "Tan", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Tan : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("TAN")

	UVoxelNode_Tan();
};

// Return the atan of the input
UCLASS(DisplayName = "Atan", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Atan : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("ATAN")

	UVoxelNode_Atan();
};

// Return atan2(Y, X)
UCLASS(DisplayName = "Atan2", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_Atan2 : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_Atan2();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Rotates around Axis (assumes Axis.Size() == 1). Angle is in degrees
UCLASS(DisplayName = "Vector Rotate Angle Axis", Category = "Math|Vector")
class VOXELGRAPH_API UVoxelNode_VectorRotateAngleAxis : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_VectorRotateAngleAxis();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Break a color into its 4 integer RGBA components
UCLASS(DisplayName = "Break Color Int", Category = "Math|Color")
class VOXELGRAPH_API UVoxelNode_BreakColorInt : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_BreakColorInt();
};

// Break a color into its 4 RGBA components, and convert them to floats between 0 and 1
UCLASS(DisplayName = "Break Color", Category = "Math|Color")
class VOXELGRAPH_API UVoxelNode_BreakColorFloat : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_BreakColorFloat();
};

// Make a color from its 4 integer RGBA components
UCLASS(DisplayName = "Make Color Int", Category = "Math|Color")
class VOXELGRAPH_API UVoxelNode_MakeColorInt : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_MakeColorInt();
};

// Make a color into its 4 RGBA components as floats between 0 and 1
UCLASS(DisplayName = "Make Color", Category = "Math|Color")
class VOXELGRAPH_API UVoxelNode_MakeColorFloat : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_MakeColorFloat();
};

// Convert a color in RGB space to HSV
UCLASS(DisplayName = "RGB to HSV", Category = "Math|Color")
class VOXELGRAPH_API UVoxelNode_RGBToHSV : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_RGBToHSV();
};

// Convert a color in HSV space to RGB
UCLASS(DisplayName = "HSV to RGB", Category = "Math|Color")
class VOXELGRAPH_API UVoxelNode_HSVToRGB : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_HSVToRGB();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Pi = 3.1415926535897932384626433832795
UCLASS(DisplayName = "Pi", Category = "Math|Constants")
class VOXELGRAPH_API UVoxelNode_Pi : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("PI");

	UVoxelNode_Pi();
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Normalize an arbitrary number of values so that their sum is 1.
// Not the same as normalizing a vector!
// Only works on positive values
UCLASS(DisplayName = "Normalize Sum", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_NormalizeSum : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_NormalizeSum();

	virtual void OnInputPinCountModified() override;
	virtual int32 GetOutputPinsCount() const override;
};