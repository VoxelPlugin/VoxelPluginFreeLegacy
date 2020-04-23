// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

using EC = EVoxelPinCategory;

#define GENERATED_VOXELNODE_BODY()
#define GENERATED_VOXELNODE_IMPL_IMPL(Name, ...)
#define GENERATED_DATA_COMPUTE_NODE_BODY()
#define GENERATED_COMPUTE(InputsDef, OutputsDef, ...)
#define GENERATED_COMPUTENODE(InputsDef, OutputsDef, ...)
#define GENERATED_VOXELNODE_IMPL(Name, InputsDef, OutputsDef, Code)
#define GENERATED_COMPUTENODE_PREFIXOPLOOP(Op, Type)
#define GENERATED_VOXELNODE_IMPL_PREFIXOPLOOP(Name, Op, Type)
#define GENERATED_COMPUTENODE_INFIXOPLOOP(Op, Type)

#define GENERATED_VOXELNODE_IMPL_INFIXOPLOOP(Name, Op, Type) GENERATED_VOXELNODE_IMPL_IMPL(Name, GENERATED_COMPUTENODE_INFIXOPLOOP(Op, Type))

#define GENERATED_BINARY_VOXELNODE(Name, Op, ECType, RealType) \
Name::Name() \
{ \
	SetInputs(EC::ECType, EC::ECType); \
	SetOutputs(EC::Boolean); \
} \
GENERATED_VOXELNODE_IMPL_IMPL(Name, \
GENERATED_COMPUTENODE\
(\
	DEFINE_INPUTS(RealType, RealType),\
	DEFINE_OUTPUTS(bool),\
	_O0 = _I0 Op _I1;\
))

#define GENERATED_VOXELNODE_IMPL_BINARY_FLOAT(Name, Op) GENERATED_BINARY_VOXELNODE(Name, Op, Float, v_flt)
#define GENERATED_VOXELNODE_IMPL_BINARY_INT(Name, Op) GENERATED_BINARY_VOXELNODE(Name, Op, Int, int32)

#define COMPACT_VOXELNODE(Text) \
bool IsCompact() const override { return true; } \
FText GetTitle() const override { return VOXEL_LOCTEXT(Text); }

#define SET_VOXELNODE_TITLE(Text) \
FText GetTitle() const override { return VOXEL_LOCTEXT(Text); }

#define DEPRECATED_VOXELNODE(Message) \
void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) \
{ \
	Super::LogErrors(ErrorReporter); \
	ErrorReporter.AddMessageToNode(this, "deprecated node: " Message, EVoxelGraphNodeMessageType::FatalError); \
}

#define __DEFINE_INPUTS_0(A) auto& _I0 = Inputs[0].Get<A>();
#define __DEFINE_INPUTS_1(A, ...) auto& _I1 = Inputs[1].Get<A>(); EXPAND(__DEFINE_INPUTS_0(__VA_ARGS__))
#define __DEFINE_INPUTS_2(A, ...) auto& _I2 = Inputs[2].Get<A>(); EXPAND(__DEFINE_INPUTS_1(__VA_ARGS__))
#define __DEFINE_INPUTS_3(A, ...) auto& _I3 = Inputs[3].Get<A>(); EXPAND(__DEFINE_INPUTS_2(__VA_ARGS__))
#define __DEFINE_INPUTS_4(A, ...) auto& _I4 = Inputs[4].Get<A>(); EXPAND(__DEFINE_INPUTS_3(__VA_ARGS__))
#define __DEFINE_INPUTS_5(A, ...) auto& _I5 = Inputs[5].Get<A>(); EXPAND(__DEFINE_INPUTS_4(__VA_ARGS__))
#define __DEFINE_INPUTS_6(A, ...) auto& _I6 = Inputs[6].Get<A>(); EXPAND(__DEFINE_INPUTS_5(__VA_ARGS__))
#define __DEFINE_INPUTS_7(A, ...) auto& _I7 = Inputs[7].Get<A>(); EXPAND(__DEFINE_INPUTS_6(__VA_ARGS__))
#define __DEFINE_INPUTS_8(A, ...) auto& _I8 = Inputs[8].Get<A>(); EXPAND(__DEFINE_INPUTS_7(__VA_ARGS__))
#define __DEFINE_INPUTS_9(A, ...) auto& _I9 = Inputs[9].Get<A>(); EXPAND(__DEFINE_INPUTS_8(__VA_ARGS__))
#define __DEFINE_INPUTS_10(A, ...) auto& _I10 = Inputs[10].Get<A>(); EXPAND(__DEFINE_INPUTS_9(__VA_ARGS__))
#define __DEFINE_INPUTS_11(A, ...) auto& _I11 = Inputs[11].Get<A>(); EXPAND(__DEFINE_INPUTS_10(__VA_ARGS__))
#define __DEFINE_INPUTS_12(A, ...) auto& _I12 = Inputs[12].Get<A>(); EXPAND(__DEFINE_INPUTS_11(__VA_ARGS__))
#define __DEFINE_INPUTS_13(A, ...) auto& _I13 = Inputs[13].Get<A>(); EXPAND(__DEFINE_INPUTS_12(__VA_ARGS__))
#define __DEFINE_INPUTS_14(A, ...) auto& _I14 = Inputs[14].Get<A>(); EXPAND(__DEFINE_INPUTS_13(__VA_ARGS__))
#define __DEFINE_INPUTS_15(A, ...) auto& _I15 = Inputs[15].Get<A>(); EXPAND(__DEFINE_INPUTS_14(__VA_ARGS__))
#define __DEFINE_INPUTS_16(A, ...) auto& _I16 = Inputs[16].Get<A>(); EXPAND(__DEFINE_INPUTS_15(__VA_ARGS__))
#define __DEFINE_INPUTS_17(A, ...) auto& _I17 = Inputs[17].Get<A>(); EXPAND(__DEFINE_INPUTS_16(__VA_ARGS__))
#define __DEFINE_INPUTS_18(A, ...) auto& _I18 = Inputs[18].Get<A>(); EXPAND(__DEFINE_INPUTS_17(__VA_ARGS__))
#define __DEFINE_INPUTS_19(A, ...) auto& _I19 = Inputs[19].Get<A>(); EXPAND(__DEFINE_INPUTS_18(__VA_ARGS__))
#define __DEFINE_INPUTS_20(A, ...) auto& _I20 = Inputs[20].Get<A>(); EXPAND(__DEFINE_INPUTS_19(__VA_ARGS__))
#define __DEFINE_INPUTS_21(A, ...) auto& _I21 = Inputs[21].Get<A>(); EXPAND(__DEFINE_INPUTS_20(__VA_ARGS__))
#define __DEFINE_INPUTS_22(A, ...) auto& _I22 = Inputs[22].Get<A>(); EXPAND(__DEFINE_INPUTS_21(__VA_ARGS__))
#define __DEFINE_INPUTS_23(A, ...) auto& _I23 = Inputs[23].Get<A>(); EXPAND(__DEFINE_INPUTS_22(__VA_ARGS__))
#define __DEFINE_INPUTS_24(A, ...) auto& _I24 = Inputs[24].Get<A>(); EXPAND(__DEFINE_INPUTS_23(__VA_ARGS__))
#define __DEFINE_INPUTS_25(A, ...) auto& _I25 = Inputs[25].Get<A>(); EXPAND(__DEFINE_INPUTS_24(__VA_ARGS__))
#define __DEFINE_INPUTS_26(A, ...) auto& _I26 = Inputs[26].Get<A>(); EXPAND(__DEFINE_INPUTS_25(__VA_ARGS__))
#define __DEFINE_INPUTS_27(A, ...) auto& _I27 = Inputs[27].Get<A>(); EXPAND(__DEFINE_INPUTS_26(__VA_ARGS__))
#define __DEFINE_INPUTS_28(A, ...) auto& _I28 = Inputs[28].Get<A>(); EXPAND(__DEFINE_INPUTS_27(__VA_ARGS__))
#define __DEFINE_INPUTS_29(A, ...) auto& _I29 = Inputs[29].Get<A>(); EXPAND(__DEFINE_INPUTS_28(__VA_ARGS__))
#define __DEFINE_INPUTS_30(A, ...) auto& _I30 = Inputs[30].Get<A>(); EXPAND(__DEFINE_INPUTS_29(__VA_ARGS__))
#define __DEFINE_INPUTS_31(A, ...) auto& _I31 = Inputs[31].Get<A>(); EXPAND(__DEFINE_INPUTS_30(__VA_ARGS__))
#define __DEFINE_INPUTS_32(A, ...) auto& _I32 = Inputs[32].Get<A>(); EXPAND(__DEFINE_INPUTS_31(__VA_ARGS__))

#define __DEFINE_OUTPUTS_0(A) auto& _O0 = Outputs[0].Get<A>();
#define __DEFINE_OUTPUTS_1(A, ...) auto& _O1 = Outputs[1].Get<A>(); EXPAND(__DEFINE_OUTPUTS_0(__VA_ARGS__))
#define __DEFINE_OUTPUTS_2(A, ...) auto& _O2 = Outputs[2].Get<A>(); EXPAND(__DEFINE_OUTPUTS_1(__VA_ARGS__))
#define __DEFINE_OUTPUTS_3(A, ...) auto& _O3 = Outputs[3].Get<A>(); EXPAND(__DEFINE_OUTPUTS_2(__VA_ARGS__))
#define __DEFINE_OUTPUTS_4(A, ...) auto& _O4 = Outputs[4].Get<A>(); EXPAND(__DEFINE_OUTPUTS_3(__VA_ARGS__))
#define __DEFINE_OUTPUTS_5(A, ...) auto& _O5 = Outputs[5].Get<A>(); EXPAND(__DEFINE_OUTPUTS_4(__VA_ARGS__))
#define __DEFINE_OUTPUTS_6(A, ...) auto& _O6 = Outputs[6].Get<A>(); EXPAND(__DEFINE_OUTPUTS_5(__VA_ARGS__))
#define __DEFINE_OUTPUTS_7(A, ...) auto& _O7 = Outputs[7].Get<A>(); EXPAND(__DEFINE_OUTPUTS_6(__VA_ARGS__))
#define __DEFINE_OUTPUTS_8(A, ...) auto& _O8 = Outputs[8].Get<A>(); EXPAND(__DEFINE_OUTPUTS_7(__VA_ARGS__))
#define __DEFINE_OUTPUTS_9(A, ...) auto& _O9 = Outputs[9].Get<A>(); EXPAND(__DEFINE_OUTPUTS_8(__VA_ARGS__))
#define __DEFINE_OUTPUTS_10(A, ...) auto& _O10 = Outputs[10].Get<A>(); EXPAND(__DEFINE_OUTPUTS_9(__VA_ARGS__))
#define __DEFINE_OUTPUTS_11(A, ...) auto& _O11 = Outputs[11].Get<A>(); EXPAND(__DEFINE_OUTPUTS_10(__VA_ARGS__))
#define __DEFINE_OUTPUTS_12(A, ...) auto& _O12 = Outputs[12].Get<A>(); EXPAND(__DEFINE_OUTPUTS_11(__VA_ARGS__))
#define __DEFINE_OUTPUTS_13(A, ...) auto& _O13 = Outputs[13].Get<A>(); EXPAND(__DEFINE_OUTPUTS_12(__VA_ARGS__))
#define __DEFINE_OUTPUTS_14(A, ...) auto& _O14 = Outputs[14].Get<A>(); EXPAND(__DEFINE_OUTPUTS_13(__VA_ARGS__))
#define __DEFINE_OUTPUTS_15(A, ...) auto& _O15 = Outputs[15].Get<A>(); EXPAND(__DEFINE_OUTPUTS_14(__VA_ARGS__))
#define __DEFINE_OUTPUTS_16(A, ...) auto& _O16 = Outputs[16].Get<A>(); EXPAND(__DEFINE_OUTPUTS_15(__VA_ARGS__))
#define __DEFINE_OUTPUTS_17(A, ...) auto& _O17 = Outputs[17].Get<A>(); EXPAND(__DEFINE_OUTPUTS_16(__VA_ARGS__))
#define __DEFINE_OUTPUTS_18(A, ...) auto& _O18 = Outputs[18].Get<A>(); EXPAND(__DEFINE_OUTPUTS_17(__VA_ARGS__))
#define __DEFINE_OUTPUTS_19(A, ...) auto& _O19 = Outputs[19].Get<A>(); EXPAND(__DEFINE_OUTPUTS_18(__VA_ARGS__))
#define __DEFINE_OUTPUTS_20(A, ...) auto& _O20 = Outputs[20].Get<A>(); EXPAND(__DEFINE_OUTPUTS_19(__VA_ARGS__))
#define __DEFINE_OUTPUTS_21(A, ...) auto& _O21 = Outputs[21].Get<A>(); EXPAND(__DEFINE_OUTPUTS_20(__VA_ARGS__))
#define __DEFINE_OUTPUTS_22(A, ...) auto& _O22 = Outputs[22].Get<A>(); EXPAND(__DEFINE_OUTPUTS_21(__VA_ARGS__))
#define __DEFINE_OUTPUTS_23(A, ...) auto& _O23 = Outputs[23].Get<A>(); EXPAND(__DEFINE_OUTPUTS_22(__VA_ARGS__))
#define __DEFINE_OUTPUTS_24(A, ...) auto& _O24 = Outputs[24].Get<A>(); EXPAND(__DEFINE_OUTPUTS_23(__VA_ARGS__))
#define __DEFINE_OUTPUTS_25(A, ...) auto& _O25 = Outputs[25].Get<A>(); EXPAND(__DEFINE_OUTPUTS_24(__VA_ARGS__))
#define __DEFINE_OUTPUTS_26(A, ...) auto& _O26 = Outputs[26].Get<A>(); EXPAND(__DEFINE_OUTPUTS_25(__VA_ARGS__))
#define __DEFINE_OUTPUTS_27(A, ...) auto& _O27 = Outputs[27].Get<A>(); EXPAND(__DEFINE_OUTPUTS_26(__VA_ARGS__))
#define __DEFINE_OUTPUTS_28(A, ...) auto& _O28 = Outputs[28].Get<A>(); EXPAND(__DEFINE_OUTPUTS_27(__VA_ARGS__))
#define __DEFINE_OUTPUTS_29(A, ...) auto& _O29 = Outputs[29].Get<A>(); EXPAND(__DEFINE_OUTPUTS_28(__VA_ARGS__))
#define __DEFINE_OUTPUTS_30(A, ...) auto& _O30 = Outputs[30].Get<A>(); EXPAND(__DEFINE_OUTPUTS_29(__VA_ARGS__))
#define __DEFINE_OUTPUTS_31(A, ...) auto& _O31 = Outputs[31].Get<A>(); EXPAND(__DEFINE_OUTPUTS_30(__VA_ARGS__))
#define __DEFINE_OUTPUTS_32(A, ...) auto& _O32 = Outputs[32].Get<A>(); EXPAND(__DEFINE_OUTPUTS_31(__VA_ARGS__))

#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, N, ...) EXPAND(N)

#define EXPAND( x ) x

#define DEFINE_INPUTS_REVERSED(...) \
	EXPAND(_GET_NTH_ARG(__VA_ARGS__, __DEFINE_INPUTS_32, __DEFINE_INPUTS_31, __DEFINE_INPUTS_30, __DEFINE_INPUTS_29, __DEFINE_INPUTS_28, __DEFINE_INPUTS_27, __DEFINE_INPUTS_26, __DEFINE_INPUTS_25, __DEFINE_INPUTS_24, __DEFINE_INPUTS_23, __DEFINE_INPUTS_22, __DEFINE_INPUTS_21, __DEFINE_INPUTS_20, __DEFINE_INPUTS_19, __DEFINE_INPUTS_18, __DEFINE_INPUTS_17, __DEFINE_INPUTS_16, __DEFINE_INPUTS_15, __DEFINE_INPUTS_14, __DEFINE_INPUTS_13, __DEFINE_INPUTS_12, __DEFINE_INPUTS_11, __DEFINE_INPUTS_10, __DEFINE_INPUTS_9, __DEFINE_INPUTS_8, __DEFINE_INPUTS_7, __DEFINE_INPUTS_6, __DEFINE_INPUTS_5, __DEFINE_INPUTS_4, __DEFINE_INPUTS_3, __DEFINE_INPUTS_2, __DEFINE_INPUTS_1, __DEFINE_INPUTS_0)(__VA_ARGS__))

#define DEFINE_OUTPUTS_REVERSED(...) \
	EXPAND(_GET_NTH_ARG(__VA_ARGS__, __DEFINE_OUTPUTS_32, __DEFINE_OUTPUTS_31, __DEFINE_OUTPUTS_30, __DEFINE_OUTPUTS_29, __DEFINE_OUTPUTS_28, __DEFINE_OUTPUTS_27, __DEFINE_OUTPUTS_26, __DEFINE_OUTPUTS_25, __DEFINE_OUTPUTS_24, __DEFINE_OUTPUTS_23, __DEFINE_OUTPUTS_22, __DEFINE_OUTPUTS_21, __DEFINE_OUTPUTS_20, __DEFINE_OUTPUTS_19, __DEFINE_OUTPUTS_18, __DEFINE_OUTPUTS_17, __DEFINE_OUTPUTS_16, __DEFINE_OUTPUTS_15, __DEFINE_OUTPUTS_14, __DEFINE_OUTPUTS_13, __DEFINE_OUTPUTS_12, __DEFINE_OUTPUTS_11, __DEFINE_OUTPUTS_10, __DEFINE_OUTPUTS_9, __DEFINE_OUTPUTS_8, __DEFINE_OUTPUTS_7, __DEFINE_OUTPUTS_6, __DEFINE_OUTPUTS_5, __DEFINE_OUTPUTS_4, __DEFINE_OUTPUTS_3, __DEFINE_OUTPUTS_2, __DEFINE_OUTPUTS_1, __DEFINE_OUTPUTS_0)(__VA_ARGS__))

#define __REVERSE_0(a) a
#define __REVERSE_1(a,b) b,a
#define __REVERSE_2(a,...) EXPAND(__REVERSE_1(__VA_ARGS__)),a
#define __REVERSE_3(a,...) EXPAND(__REVERSE_2(__VA_ARGS__)),a
#define __REVERSE_4(a,...) EXPAND(__REVERSE_3(__VA_ARGS__)),a
#define __REVERSE_5(a,...) EXPAND(__REVERSE_4(__VA_ARGS__)),a
#define __REVERSE_6(a,...) EXPAND(__REVERSE_5(__VA_ARGS__)),a
#define __REVERSE_7(a,...) EXPAND(__REVERSE_6(__VA_ARGS__)),a
#define __REVERSE_8(a,...) EXPAND(__REVERSE_7(__VA_ARGS__)),a
#define __REVERSE_9(a,...) EXPAND(__REVERSE_8(__VA_ARGS__)),a
#define __REVERSE_10(a,...) EXPAND(__REVERSE_9(__VA_ARGS__)),a
#define __REVERSE_11(a,...) EXPAND(__REVERSE_10(__VA_ARGS__)),a
#define __REVERSE_12(a,...) EXPAND(__REVERSE_11(__VA_ARGS__)),a
#define __REVERSE_13(a,...) EXPAND(__REVERSE_12(__VA_ARGS__)),a
#define __REVERSE_14(a,...) EXPAND(__REVERSE_13(__VA_ARGS__)),a
#define __REVERSE_15(a,...) EXPAND(__REVERSE_14(__VA_ARGS__)),a
#define __REVERSE_16(a,...) EXPAND(__REVERSE_15(__VA_ARGS__)),a
#define __REVERSE_17(a,...) EXPAND(__REVERSE_16(__VA_ARGS__)),a
#define __REVERSE_18(a,...) EXPAND(__REVERSE_17(__VA_ARGS__)),a
#define __REVERSE_19(a,...) EXPAND(__REVERSE_18(__VA_ARGS__)),a
#define __REVERSE_20(a,...) EXPAND(__REVERSE_19(__VA_ARGS__)),a
#define __REVERSE_21(a,...) EXPAND(__REVERSE_20(__VA_ARGS__)),a
#define __REVERSE_22(a,...) EXPAND(__REVERSE_21(__VA_ARGS__)),a
#define __REVERSE_23(a,...) EXPAND(__REVERSE_22(__VA_ARGS__)),a
#define __REVERSE_24(a,...) EXPAND(__REVERSE_23(__VA_ARGS__)),a
#define __REVERSE_25(a,...) EXPAND(__REVERSE_24(__VA_ARGS__)),a
#define __REVERSE_26(a,...) EXPAND(__REVERSE_25(__VA_ARGS__)),a
#define __REVERSE_27(a,...) EXPAND(__REVERSE_26(__VA_ARGS__)),a
#define __REVERSE_28(a,...) EXPAND(__REVERSE_27(__VA_ARGS__)),a
#define __REVERSE_29(a,...) EXPAND(__REVERSE_28(__VA_ARGS__)),a
#define __REVERSE_30(a,...) EXPAND(__REVERSE_29(__VA_ARGS__)),a
#define __REVERSE_31(a,...) EXPAND(__REVERSE_30(__VA_ARGS__)),a
#define __REVERSE_32(a,...) EXPAND(__REVERSE_31(__VA_ARGS__)),a

#define DEFINE_INPUTS(...) DEFINE_INPUTS_REVERSED(EXPAND(_GET_NTH_ARG(__VA_ARGS__, __REVERSE_32, __REVERSE_31, __REVERSE_30, __REVERSE_29, __REVERSE_28, __REVERSE_27, __REVERSE_26, __REVERSE_25, __REVERSE_24, __REVERSE_23, __REVERSE_22, __REVERSE_21, __REVERSE_20, __REVERSE_19, __REVERSE_18, __REVERSE_17, __REVERSE_16, __REVERSE_15, __REVERSE_14, __REVERSE_13, __REVERSE_12, __REVERSE_11, __REVERSE_10, __REVERSE_9, __REVERSE_8, __REVERSE_7, __REVERSE_6, __REVERSE_5, __REVERSE_4, __REVERSE_3, __REVERSE_2, __REVERSE_1, __REVERSE_0)(__VA_ARGS__)))
#define DEFINE_OUTPUTS(...) DEFINE_OUTPUTS_REVERSED(EXPAND(_GET_NTH_ARG(__VA_ARGS__, __REVERSE_32, __REVERSE_31, __REVERSE_30, __REVERSE_29, __REVERSE_28, __REVERSE_27, __REVERSE_26, __REVERSE_25, __REVERSE_24, __REVERSE_23, __REVERSE_22, __REVERSE_21, __REVERSE_20, __REVERSE_19, __REVERSE_18, __REVERSE_17, __REVERSE_16, __REVERSE_15, __REVERSE_14, __REVERSE_13, __REVERSE_12, __REVERSE_11, __REVERSE_10, __REVERSE_9, __REVERSE_8, __REVERSE_7, __REVERSE_6, __REVERSE_5, __REVERSE_4, __REVERSE_3, __REVERSE_2, __REVERSE_1, __REVERSE_0)(__VA_ARGS__)))

#define NO_INPUTS
#define NO_OUTPUTS