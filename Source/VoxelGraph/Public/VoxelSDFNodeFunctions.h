// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelUtilities/VoxelSDFUtilities.h"
#include "VoxelUtilities/VoxelSDFRangeUtilities.h"

namespace FVoxelSDFNodeFunctions
{
#define DEFINE_SDF_FUNCTION_EX(Name, Prefix, Args, sdArgs) \
	template<typename T> \
	FORCEINLINE typename TEnableIf<TIsSame<T, v_flt>::Value, T>::Type Name Args { return FVoxelSDFUtilities::Prefix##Name sdArgs; } \
	template<typename T> \
	FORCEINLINE typename TEnableIf<TIsSame<T, TVoxelRange<v_flt>>::Value, T>::Type Name Args { return FVoxelSDFRangeUtilities::Prefix##Name sdArgs; }

#define DEFINE_SDF_FUNCTION(Name, Args, sdArgs) DEFINE_SDF_FUNCTION_EX(Name, sd, Args, sdArgs)

// Put the args in parenthesis, else they are expanded and DEFINE_SDF_FUNCTION is called with too many arguments
// DEFINE_SDF_FUNCTION_EX accounts for the args being in parenthesis
// NOTE: this is only an issue on Clang, MSVC compiles fine :/
#define ARG(...) (__VA_ARGS__)

#define VECTOR_A(Name) T Name##X, T Name##Y, T Name##Z
// NOTE: We swap Y and Z to match the axis of shadertoy vs unreal
#define VECTOR_B(Name) { Name##X, Name##Z, Name##Y }
	
#define VECTOR2_A(Name) T Name##X, T Name##Y
#define VECTOR2_B(Name) { Name##X, Name##Y }

#define SCALAR_A(Name) T Name
#define SCALAR_B(Name) Name

	// Sphere - exact
	DEFINE_SDF_FUNCTION(
		Sphere,
		ARG(VECTOR_A(p), SCALAR_A(s)),
		ARG(VECTOR_B(p), SCALAR_B(s)));

	// Box - exact
	DEFINE_SDF_FUNCTION(
		Box,
		ARG(VECTOR_A(p), VECTOR_A(b)),
		ARG(VECTOR_B(p), VECTOR_B(b)));

	// Round Box - exact
	DEFINE_SDF_FUNCTION(
		RoundBox,
		ARG(VECTOR_A(p), VECTOR_A(b), SCALAR_A(r)),
		ARG(VECTOR_B(p), VECTOR_B(b), SCALAR_B(r)));

	// Torus - exact
	DEFINE_SDF_FUNCTION(
		Torus,
		ARG(VECTOR_A(p), VECTOR2_A(t)),
		ARG(VECTOR_B(p), VECTOR2_B(t)));

	// Capped Torus - exact
	DEFINE_SDF_FUNCTION(
		CappedTorus,
		ARG(VECTOR_A(p), VECTOR2_A(sc), SCALAR_A(ra), SCALAR_A(rb)),
		ARG(VECTOR_B(p), VECTOR2_B(sc), SCALAR_B(ra), SCALAR_B(rb)));

	// Link - exact
	DEFINE_SDF_FUNCTION(
		Link,
		ARG(VECTOR_A(p), SCALAR_A(le), SCALAR_A(r1), SCALAR_A(r2)),
		ARG(VECTOR_B(p), SCALAR_B(le), SCALAR_B(r1), SCALAR_B(r2)));

	// Infinite Cylinder - exact
	DEFINE_SDF_FUNCTION(
		Cylinder,
		ARG(VECTOR_A(p), VECTOR_A(c)),
		ARG(VECTOR_B(p), VECTOR_B(c)));

	// Cone - exact
	DEFINE_SDF_FUNCTION(
		Cone,
		ARG(VECTOR_A(p), VECTOR2_A(c), SCALAR_A(h)),
		ARG(VECTOR_B(p), VECTOR2_B(c), SCALAR_B(h)));

	// Cone - bound(not exact!)
	DEFINE_SDF_FUNCTION(
		ConeFast,
		ARG(VECTOR_A(p), VECTOR2_A(c), SCALAR_A(h)),
		ARG(VECTOR_B(p), VECTOR2_B(c), SCALAR_B(h)));

	// Infinite Cone - exact
	DEFINE_SDF_FUNCTION(
		Cone,
		ARG(VECTOR_A(p), VECTOR2_A(c)),
		ARG(VECTOR_B(p), VECTOR2_B(c)));

	// Plane - exact
	DEFINE_SDF_FUNCTION(
		Plane,
		ARG(VECTOR_A(p), VECTOR_A(n)),
		ARG(VECTOR_B(p), VECTOR_B(n)));

	// Hexagonal Prism - exact
	DEFINE_SDF_FUNCTION(
		HexPrism,
		ARG(VECTOR_A(p), VECTOR2_A(h)),
		ARG(VECTOR_B(p), VECTOR2_B(h)));

	// Triangular Prism - bound
	DEFINE_SDF_FUNCTION(
		TriPrism,
		ARG(VECTOR_A(p), VECTOR2_A(h)),
		ARG(VECTOR_B(p), VECTOR2_B(h)));

	// Capsule / Line - exact
	DEFINE_SDF_FUNCTION(
		Capsule,
		ARG(VECTOR_A(p), VECTOR_A(a), VECTOR_A(b), SCALAR_A(r)),
		ARG(VECTOR_B(p), VECTOR_B(a), VECTOR_B(b), SCALAR_B(r)));

	// Capsule / Line - exact
	DEFINE_SDF_FUNCTION(
		VerticalCapsule,
		ARG(VECTOR_A(p), SCALAR_A(h), SCALAR_A(r)),
		ARG(VECTOR_B(p), SCALAR_B(h), SCALAR_B(r)));

	// Capped Cylinder - exact
	DEFINE_SDF_FUNCTION(
		CappedCylinder,
		ARG(VECTOR_A(p), SCALAR_A(h), SCALAR_A(r)),
		ARG(VECTOR_B(p), SCALAR_B(h), SCALAR_B(r)));

	// Capped Cylinder - exact
	DEFINE_SDF_FUNCTION(
		CappedCylinder,
		ARG(VECTOR_A(p), VECTOR_A(a), VECTOR_A(b), SCALAR_A(r)),
		ARG(VECTOR_B(p), VECTOR_B(a), VECTOR_B(b), SCALAR_B(r)));

	// Rounded Cylinder - exact
	DEFINE_SDF_FUNCTION(
		RoundedCylinder,
		ARG(VECTOR_A(p), SCALAR_A(ra), SCALAR_A(rb), SCALAR_A(h)),
		ARG(VECTOR_B(p), SCALAR_B(ra), SCALAR_B(rb), SCALAR_B(h)));

	// Capped Cone - exact
	DEFINE_SDF_FUNCTION(
		CappedCone,
		ARG(VECTOR_A(p), SCALAR_A(h), SCALAR_A(r1), SCALAR_A(r2)),
		ARG(VECTOR_B(p), SCALAR_B(h), SCALAR_B(r1), SCALAR_B(r2)));

	// Capped Cone - exact
	DEFINE_SDF_FUNCTION(
		CappedCone,
		ARG(VECTOR_A(p), VECTOR_A(a), VECTOR_A(b), SCALAR_A(ra), SCALAR_A(rb)),
		ARG(VECTOR_B(p), VECTOR_B(a), VECTOR_B(b), SCALAR_B(ra), SCALAR_B(rb)));

	// Solid Angle - exact
	DEFINE_SDF_FUNCTION(
		SolidAngle,
		ARG(VECTOR_A(p), VECTOR2_A(c), SCALAR_A(ra)),
		ARG(VECTOR_B(p), VECTOR2_B(c), SCALAR_B(ra)));

	// Round cone - exact
	DEFINE_SDF_FUNCTION(
		RoundCone,
		ARG(VECTOR_A(p), SCALAR_A(r1), SCALAR_A(r2), SCALAR_A(h)),
		ARG(VECTOR_B(p), SCALAR_B(r1), SCALAR_B(r2), SCALAR_B(h)));

	// Round Cone - exact
	DEFINE_SDF_FUNCTION(
		RoundCone,
		ARG(VECTOR_A(p), VECTOR_A(a), VECTOR_A(b), SCALAR_A(r1), SCALAR_A(r2)),
		ARG(VECTOR_B(p), VECTOR_B(a), VECTOR_B(b), SCALAR_B(r1), SCALAR_B(r2)));

	// Ellipsoid - bound(not exact!)
	DEFINE_SDF_FUNCTION(
		Ellipsoid,
		ARG(VECTOR_A(p), VECTOR_A(r)),
		ARG(VECTOR_B(p), VECTOR_B(r)));

	// Octahedron - exact
	DEFINE_SDF_FUNCTION(
		Octahedron,
		ARG(VECTOR_A(p), SCALAR_A(s)),
		ARG(VECTOR_B(p), SCALAR_B(s)));

	// Octahedron - bound(not exact)
	DEFINE_SDF_FUNCTION(
		OctahedronFast,
		ARG(VECTOR_A(p), SCALAR_A(s)),
		ARG(VECTOR_B(p), SCALAR_B(s)));

	// Pyramid - exact
	DEFINE_SDF_FUNCTION(
		Pyramid,
		ARG(VECTOR_A(p), SCALAR_A(h)),
		ARG(VECTOR_B(p), SCALAR_B(h)));

	//////////////////////////////////////////////////////////////////////////////
	
	DEFINE_SDF_FUNCTION_EX(
		SmoothUnion,
		op,
		ARG(SCALAR_A(d1), SCALAR_A(d2), SCALAR_A(k)),
		ARG(SCALAR_B(d1), SCALAR_B(d2), SCALAR_B(k)));
	
	DEFINE_SDF_FUNCTION_EX(
		SmoothSubtraction,
		op,
		ARG(SCALAR_A(d1), SCALAR_A(d2), SCALAR_A(k)),
		ARG(SCALAR_B(d1), SCALAR_B(d2), SCALAR_B(k)));
	
	DEFINE_SDF_FUNCTION_EX(
		SmoothIntersection,
		op,
		ARG(SCALAR_A(d1), SCALAR_A(d2), SCALAR_A(k)),
		ARG(SCALAR_B(d1), SCALAR_B(d2), SCALAR_B(k)));

#undef DEFINE_SDF_FUNCTION_EX
#undef DEFINE_SDF_FUNCTION
	
#undef ARG

#undef VECTOR_A
#undef VECTOR_B
	
#undef VECTOR2_A
#undef VECTOR2_B

#undef SCALAR_A
#undef SCALAR_B
}