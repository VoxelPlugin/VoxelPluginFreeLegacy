// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelMathNodes.h"
#include "Runtime/VoxelNodeType.h"
#include "VoxelContext.h"
#include "VoxelNodeFunctions.h"

UVoxelNode_FMax::UVoxelNode_FMax()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}

GENERATED_VOXELNODE_IMPL_PREFIXOPLOOP(UVoxelNode_FMax, FVoxelNodeFunctions::Max<v_flt>, v_flt)

UVoxelNode_FMin::UVoxelNode_FMin()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}
GENERATED_VOXELNODE_IMPL_PREFIXOPLOOP(UVoxelNode_FMin, FVoxelNodeFunctions::Min<v_flt>, v_flt)

UVoxelNode_IMax::UVoxelNode_IMax()
{
	SetInputs(EC::Int);
	SetOutputs(EC::Int);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}
GENERATED_VOXELNODE_IMPL_PREFIXOPLOOP(UVoxelNode_IMax, FVoxelNodeFunctions::Max<int32>, int32)

UVoxelNode_IMin::UVoxelNode_IMin()
{
	SetInputs(EC::Int);
	SetOutputs(EC::Int);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}
GENERATED_VOXELNODE_IMPL_PREFIXOPLOOP(UVoxelNode_IMin, FVoxelNodeFunctions::Min<int32>, int32)

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_FAdd::UVoxelNode_FAdd()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}
GENERATED_VOXELNODE_IMPL_INFIXOPLOOP(UVoxelNode_FAdd, +, v_flt)

UVoxelNode_FMultiply::UVoxelNode_FMultiply()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}
GENERATED_VOXELNODE_IMPL_INFIXOPLOOP(UVoxelNode_FMultiply, *, v_flt)

UVoxelNode_FSubstract::UVoxelNode_FSubstract()
{
	SetInputs(EC::Float, EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_FSubstract,
	DEFINE_INPUTS(v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = _I0 - _I1;
)

UVoxelNode_FDivide::UVoxelNode_FDivide()
{
	SetInputs({ "", EC::Float, "", "0" }, { "", EC::Float, "", "1" });
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_FDivide,
	DEFINE_INPUTS(v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = _I0 / _I1;
)

UVoxelNode_IAdd::UVoxelNode_IAdd()
{
	SetInputs(EC::Int);
	SetOutputs(EC::Int);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}
GENERATED_VOXELNODE_IMPL_INFIXOPLOOP(UVoxelNode_IAdd, +, int32)

UVoxelNode_IMultiply::UVoxelNode_IMultiply()
{
	SetInputs(EC::Int);
	SetOutputs(EC::Int);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}
GENERATED_VOXELNODE_IMPL_INFIXOPLOOP(UVoxelNode_IMultiply, *, int32)

UVoxelNode_ISubstract::UVoxelNode_ISubstract()
{
	SetInputs(EC::Int, EC::Int);
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_ISubstract,
	DEFINE_INPUTS(int32, int32),
	DEFINE_OUTPUTS(int32),
	_O0 = _I0 - _I1;
)

UVoxelNode_IDivide::UVoxelNode_IDivide()
{
	SetInputs({ "", EC::Int, "", "0" }, { "", EC::Int, "", "1" });
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_IDivide,
	DEFINE_INPUTS(int32, int32),
	DEFINE_OUTPUTS(int32),
	_O0 = _I1 == 0 ? 0 : _I0 / _I1;
)

UVoxelNode_ILeftBitShift::UVoxelNode_ILeftBitShift()
{
	SetInputs({ "", EC::Int, "", "0" }, { "", EC::Int, "", "1" });
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_ILeftBitShift,
	DEFINE_INPUTS(int32, int32),
	DEFINE_OUTPUTS(int32),
	_O0 = FVoxelNodeFunctions::LeftShift(_I0, _I1);
)

UVoxelNode_IRightBitShift::UVoxelNode_IRightBitShift()
{
	SetInputs({ "", EC::Int, "", "0" }, { "", EC::Int, "", "1" });
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_IRightBitShift,
	DEFINE_INPUTS(int32, int32),
	DEFINE_OUTPUTS(int32),
	_O0 = FVoxelNodeFunctions::RightShift(_I0, _I1);
)

UVoxelNode_FloatOfInt::UVoxelNode_FloatOfInt()
{
	SetInputs(EC::Int);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_FloatOfInt,
	DEFINE_INPUTS(int32),
	DEFINE_OUTPUTS(v_flt),
	_O0 = _I0;
)

UVoxelNode_Round::UVoxelNode_Round()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Round,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(int32),
	_O0 = FVoxelNodeFunctions::RoundToInt(_I0);
)

UVoxelNode_Lerp::UVoxelNode_Lerp()
{
	SetInputs(
		{ "A", EC::Float, "A" },
		{ "B", EC::Float, "B" },
		{ "Alpha", EC::Float, "Alpha" });
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Lerp,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Lerp(_I0, _I1, _I2);
)

UVoxelNode_SafeLerp::UVoxelNode_SafeLerp()
{
	SetInputs(
		{ "A", EC::Float, "A" },
		{ "B", EC::Float, "B" },
		{ "Alpha", EC::Float, "Alpha" });
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_SafeLerp,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::SafeLerp(_I0, _I1, _I2);
)

UVoxelNode_Clamp::UVoxelNode_Clamp()
{
	SetInputs(
		{ "Value", EC::Float, "Value to clamp" },
		{ "Min", EC::Float, "Min value", "0" },
		{ "Max", EC::Float, "Max value", "1" });
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Clamp,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Clamp(_I0, _I1, _I2);
)

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_BAnd::UVoxelNode_BAnd()
{
	SetInputs(EC::Boolean);
	SetOutputs(EC::Boolean);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}
GENERATED_VOXELNODE_IMPL_INFIXOPLOOP(UVoxelNode_BAnd, &&, bool)

UVoxelNode_BOr::UVoxelNode_BOr()
{
	SetInputs(EC::Boolean);
	SetOutputs(EC::Boolean);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}
GENERATED_VOXELNODE_IMPL_INFIXOPLOOP(UVoxelNode_BOr, || , bool)

UVoxelNode_BNot::UVoxelNode_BNot()
{
	SetInputs(EC::Boolean);
	SetOutputs(EC::Boolean);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_BNot,
	DEFINE_INPUTS(bool),
	DEFINE_OUTPUTS(bool),
	_O0 = !_I0;
)

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_SwitchInt::UVoxelNode_SwitchInt()
{
	SetInputs(
		{ "A", EC::Int, "A" },
		{ "B", EC::Int, "B" },
		{ "Pick A", EC::Boolean, "Condition" });
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_SwitchInt,
	DEFINE_INPUTS(int32, int32, bool),
	DEFINE_OUTPUTS(int32),
	_O0 = FVoxelNodeFunctions::Switch(_I0, _I1, _I2);
)

UVoxelNode_SwitchFloat::UVoxelNode_SwitchFloat()
{
	SetInputs(
		{ "A", EC::Float, "A" },
		{ "B", EC::Float, "B" },
		{ "Pick A", EC::Boolean, "Condition" });
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_SwitchFloat,
	DEFINE_INPUTS(v_flt, v_flt, bool),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Switch(_I0, _I1, _I2);
)

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_1MinusX::UVoxelNode_1MinusX()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_1MinusX,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = 1 - _I0;
)

UVoxelNode_OneOverX::UVoxelNode_OneOverX()
{
	SetInputs({ "", EC::Float, "", "1" });
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_OneOverX,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::OneOverX(_I0);
)

UVoxelNode_MinusX::UVoxelNode_MinusX()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_MinusX,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = _I0 * -1;
)

UVoxelNode_Sqrt::UVoxelNode_Sqrt()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Sqrt,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Sqrt(_I0);
)

UVoxelNode_Pow::UVoxelNode_Pow()
{
	SetInputs(EC::Float, EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Pow,
	DEFINE_INPUTS(v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Pow(_I0, _I1);
)

UVoxelNode_IMod::UVoxelNode_IMod()
{
	SetInputs(EC::Int, EC::Int);
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_IMod,
	DEFINE_INPUTS(int32, int32),
	DEFINE_OUTPUTS(int32),
	_O0 = FVoxelNodeFunctions::Mod(_I0, _I1);
)

UVoxelNode_FMod::UVoxelNode_FMod()
{
	SetInputs(EC::Float, EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_FMod,
	DEFINE_INPUTS(v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Fmod(_I0, _I1);
)

UVoxelNode_FAbs::UVoxelNode_FAbs()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_FAbs,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Abs(_I0);
)

UVoxelNode_IAbs::UVoxelNode_IAbs()
{
	SetInputs(EC::Int);
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_IAbs,
	DEFINE_INPUTS(int32),
	DEFINE_OUTPUTS(int32),
	_O0 = FVoxelNodeFunctions::Abs(_I0);
)

UVoxelNode_Ceil::UVoxelNode_Ceil()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Ceil,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(int32),
	_O0 = FVoxelNodeFunctions::CeilToInt(_I0);
)

UVoxelNode_Floor::UVoxelNode_Floor()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Floor,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(int32),
	_O0 = FVoxelNodeFunctions::FloorToInt(_I0);
)

UVoxelNode_VectorLength::UVoxelNode_VectorLength()
{
	SetInputs(
		{ "X", EC::Float, "Z" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" });
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_VectorLength,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::VectorLength(_I0, _I1, _I2);
)

UVoxelNode_Fraction::UVoxelNode_Fraction()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Fraction,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Fractional(_I0);
)

UVoxelNode_FSign::UVoxelNode_FSign()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_FSign,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Sign(_I0);
)

UVoxelNode_ISign::UVoxelNode_ISign()
{
	SetInputs(EC::Int);
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_ISign,
	DEFINE_INPUTS(int32),
	DEFINE_OUTPUTS(int32),
	_O0 = FVoxelNodeFunctions::Sign(_I0);
)

UVoxelNode_InvSqrt::UVoxelNode_InvSqrt()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_InvSqrt,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::InvSqrt(_I0);
)

UVoxelNode_Loge::UVoxelNode_Loge()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Loge,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Loge(_I0);
)

UVoxelNode_Exp::UVoxelNode_Exp()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Exp,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Exp(_I0);
)

UVoxelNode_Sin::UVoxelNode_Sin()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Sin,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Sin(_I0);
)

UVoxelNode_Asin::UVoxelNode_Asin()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Asin,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Asin(_I0);
)

UVoxelNode_Sinh::UVoxelNode_Sinh()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Sinh,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Sinh(_I0);
)

UVoxelNode_Cos::UVoxelNode_Cos()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Cos,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Cos(_I0);
)

UVoxelNode_Acos::UVoxelNode_Acos()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Acos,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Acos(_I0);
)

UVoxelNode_Tan::UVoxelNode_Tan()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Tan,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Tan(_I0);
)

UVoxelNode_Atan::UVoxelNode_Atan()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Atan,
	DEFINE_INPUTS(v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Atan(_I0);
)

UVoxelNode_Atan2::UVoxelNode_Atan2()
{
	SetInputs(
		{ "Y", EC::Float, "Y" },
		{ "X", EC::Float, "X" });
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Atan2,
	DEFINE_INPUTS(v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::Atan2(_I0, _I1);
)

UVoxelNode_VectorRotateAngleAxis::UVoxelNode_VectorRotateAngleAxis()
{
	SetInputs(
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" },
		{ "Axis X", EC::Float, "Axis X" },
		{ "Axis Y", EC::Float, "Axis Y" },
		{ "Axis Z", EC::Float, "Axis Z" },
		{ "Angle", EC::Float, "Angle in degrees" }
	);
	SetOutputs(
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" }
	);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_VectorRotateAngleAxis,
	DEFINE_INPUTS(v_flt, v_flt, v_flt, v_flt, v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt, v_flt, v_flt),
	FVoxelNodeFunctions::VectorRotateAngleAxis(_I0, _I1, _I2, _I3, _I4, _I5, _I6, _O0, _O1, _O2);
)

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_BreakColorInt::UVoxelNode_BreakColorInt()
{
	SetInputs({ "Color", EC::Color, "Color" });
	SetOutputs(
		{ "R", EC::Int, "Red between 0 and 255"   },
		{ "G", EC::Int, "Green between 0 and 255" },
		{ "B", EC::Int, "Blue between 0 and 255"  },
		{ "A", EC::Int, "Alpha between 0 and 255" }
	);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_BreakColorInt,
	DEFINE_INPUTS(FColor),
	DEFINE_OUTPUTS(int32, int32, int32, int32),
	FVoxelNodeFunctions::BreakColor(_I0, _O0, _O1, _O2, _O3);
)

UVoxelNode_BreakColorFloat::UVoxelNode_BreakColorFloat()
{
	SetInputs({ "Color", EC::Color, "Color" });
	SetOutputs(
		{ "R",EC::Float, "Red between 0 and 1"    },
		{ "G",EC::Float, "Green between 0 and 1"  },
		{ "B",EC::Float, "Blue between 0 and 1"   },
		{ "A",EC::Float, "Alpha between 0 and 1"  }
	);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_BreakColorFloat,
	DEFINE_INPUTS(FColor),
	DEFINE_OUTPUTS(v_flt, v_flt, v_flt, v_flt),
	FVoxelNodeFunctions::BreakColorFloat(_I0, _O0, _O1, _O2, _O3);
)

UVoxelNode_MakeColorInt::UVoxelNode_MakeColorInt()
{
	SetInputs(
		{ "R",EC::Int , "Red between 0 and 255"  , "", {0, 255} },
		{ "G",EC::Int , "Green between 0 and 255", "", {0, 255} },
		{ "B",EC::Int , "Blue between 0 and 255" , "", {0, 255} },
		{ "A",EC::Int , "Alpha between 0 and 255", "", {0, 255} });
	SetOutputs({ "Color", EC::Color, "Color" });
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_MakeColorInt,
	DEFINE_INPUTS(int32, int32, int32, int32),
	DEFINE_OUTPUTS(FColor),
	_O0 = FVoxelNodeFunctions::MakeColor(_I0, _I1, _I2, _I3);
)

UVoxelNode_MakeColorFloat::UVoxelNode_MakeColorFloat()
{
	SetInputs(
		{ "R",EC::Float , "Red between 0 and 1"  , "", {0, 1} },
		{ "G",EC::Float , "Green between 0 and 1", "", {0, 1} },
		{ "B",EC::Float , "Blue between 0 and 1" , "", {0, 1} },
		{ "A",EC::Float , "Alpha between 0 and 1", "", {0, 1} });
	SetOutputs({ "Color", EC::Color, "Color" });
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_MakeColorFloat,
	DEFINE_INPUTS(v_flt, v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(FColor),
	_O0 = FVoxelNodeFunctions::MakeColorFloat(_I0, _I1, _I2, _I3);
)

UVoxelNode_RGBToHSV::UVoxelNode_RGBToHSV()
{
	SetInputs(
		{ "R", EC::Float , "Red" },
		{ "G", EC::Float , "Green" },
		{ "B", EC::Float , "Blue" });
	SetOutputs(
		{ "H", EC::Float , "Hue" },
		{ "S", EC::Float , "Saturation" },
		{ "V", EC::Float , "Value" });
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_RGBToHSV,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt, v_flt, v_flt),
	FVoxelNodeFunctions::RGBToHSV(_I0, _I1, _I2, _O0, _O1, _O2);
)

UVoxelNode_HSVToRGB::UVoxelNode_HSVToRGB()
{
	SetInputs(
		{ "H", EC::Float , "Hue between 0 and 360", "0", { 0, 360 } },
		{ "S", EC::Float , "Saturation between 0 and 1", "1", { 0, 1 } },
		{ "V", EC::Float , "Value", "1" });
	SetOutputs(
		{ "R", EC::Float , "Red" },
		{ "G", EC::Float , "Green" },
		{ "B", EC::Float , "Blue" });
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_HSVToRGB,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt, v_flt, v_flt),
	FVoxelNodeFunctions::HSVToRGB(_I0, _I1, _I2, _O0, _O1, _O2);
)

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_Pi::UVoxelNode_Pi()
{
	SetOutputs(EC::Float);
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_Pi,
	NO_INPUTS,
	DEFINE_OUTPUTS(v_flt),
	_O0 = v_flt(3.1415926535897932384626433832795);
)

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_NormalizeSum::UVoxelNode_NormalizeSum()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}

void UVoxelNode_NormalizeSum::OnInputPinCountModified()
{
#if WITH_EDITOR
	if (GraphNode)
	{
		GraphNode->ReconstructNode();
	}
#endif
}

int32 UVoxelNode_NormalizeSum::GetOutputPinsCount() const
{
	return FMath::Max(2, InputPinCount); // Else the default object has 0 output pins and doesn't show up in context sensitive menu
}

