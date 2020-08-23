// Copyright 2020 Phyronnaz

#if defined(__INTELLISENSE__) || defined(__RSCPP_VERSION)
#include "CoreMinimal.h"
#include "VoxelRange.h"
#include "VoxelVector.h"

using flt = TVoxelRange<v_flt>;

namespace Impl
{
	flt Min(flt a, flt b);
	flt Max(flt a, flt b);
	flt Clamp(flt x, flt a, flt b);
	flt Abs(flt x);
	flt Sign(flt x);
	flt Sqrt(flt x);
	flt Lerp(flt a, flt b, flt x);
}
#error "Compiler defined as parser"
#endif

// Most functions here are from https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
// Huge thanks to Inigo Quilez for sharing these!

// NOTE: we FORCEINLINE math functions to allow for better optimizations

//////////////////////////////////////////////////////////////////////////////
// Wrappers to make copy pasting shader code easier

struct vec2
{
	flt x;
	flt y;

	vec2() = default;
	FORCEINLINE vec2(flt v) : x(v), y(v) {}
	FORCEINLINE vec2(flt x, flt y) : x(x), y(y) {}
	template<typename T, typename = typename TEnableIf<TOr<TIsSame<T, FVector2D>, TIsSame<T, FVoxelVector2D>, TIsSame<T, FIntPoint>>::Value>::Type>
	FORCEINLINE vec2(T Vector) : x(Vector.X), y(Vector.Y) {}
	
#define SWIZZLE(a, b) FORCEINLINE const vec2 a##b() const { return { a, b }; }
	SWIZZLE(x, y);
	SWIZZLE(y, x);
#undef SWIZZLE

#define OP(operator_op, op) \
FORCEINLINE vec2 operator_op(const vec2& v) { x op v.x; y op v.y; return *this; }
OP(operator+=, +=);
OP(operator-=, -=);
OP(operator*=, *=);
OP(operator/=, /=);
#undef OP
};

struct vec3
{
	flt x;
	flt y;
	flt z;

	vec3() = default;
	FORCEINLINE vec3(flt v) : x(v), y(v), z(v) {}
	FORCEINLINE vec3(flt x, flt y, flt z) : x(x), y(y), z(z) {}
	template<typename T, typename = typename TEnableIf<TOr<TIsSame<T, FVector>, TIsSame<T, FVoxelVector>, TIsSame<T, FIntVector>>::Value>::Type>
	FORCEINLINE vec3(T Vector) : x(Vector.X), y(Vector.Y), z(Vector.Z) {}

#define SWIZZLE(a, b) \
	FORCEINLINE const vec2 a##b() const { return { a, b }; } \
	FORCEINLINE void set_##a##b(vec2 v) { a = v.x, b = v.y; }
	SWIZZLE(x, y);
	SWIZZLE(x, z);
	SWIZZLE(y, x);
	SWIZZLE(y, z);
	SWIZZLE(z, x);
	SWIZZLE(z, y);
#undef SWIZZLE

#define SWIZZLE(a, b, c) \
	FORCEINLINE const vec3 a##b##c() const { return { a, b, c }; } \
	FORCEINLINE void set_##a##b##c(vec3 v) { a = v.x, b = v.y; c = v.z; }
	SWIZZLE(x, y, z);
	SWIZZLE(x, z, y);
	SWIZZLE(y, x, z);
	SWIZZLE(y, z, x);
	SWIZZLE(z, x, y);
	SWIZZLE(z, y, x);
#undef SWIZZLE

#define OP(operator_op,op) \
FORCEINLINE vec3 operator_op(const vec3& v) { x op v.x; y op v.y; z op v.z; return *this; }
OP(operator+=, +=);
OP(operator-=, -=);
OP(operator*=, *=);
OP(operator/=, /=);
#undef OP
};

FORCEINLINE flt min(flt a, flt b) { return Impl::Min(a, b); }
FORCEINLINE flt max(flt a, flt b) { return Impl::Max(a, b); }
FORCEINLINE flt abs(flt a) { return Impl::Abs(a); }
FORCEINLINE flt sign(flt a) { return Impl::Sign(a); }
FORCEINLINE flt sqrt(flt a) { return Impl::Sqrt(a); }
FORCEINLINE flt clamp(flt x, flt a, flt b) { return Impl::Clamp(x, a, b); }
FORCEINLINE flt mix(flt a, flt b, flt x) { return Impl::Lerp(a, b, x); }

FORCEINLINE flt length(vec2 v) { return sqrt(v.x * v.x + v.y * v.y); }
FORCEINLINE flt length(vec3 v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

FORCEINLINE flt dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }
FORCEINLINE flt dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

FORCEINLINE vec3 cross(vec3 a, vec3 b)
{
	return
	{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

FORCEINLINE vec2 clamp(vec2 x, vec2 a, vec2 b) { return { Impl::Clamp(x.x, a.x, b.x), Impl::Clamp(x.y, a.y, b.y) }; }
FORCEINLINE vec3 clamp(vec3 x, vec3 a, vec3 b) { return { Impl::Clamp(x.x, a.x, b.x), Impl::Clamp(x.y, a.y, b.y), Impl::Clamp(x.z, a.z, b.z) }; }

template<typename T>
FORCEINLINE flt dot2(T v) { return dot(v, v); }

#define OP(operator_op, op) \
FORCEINLINE vec2 operator_op(const vec2& a, const vec2& b) { return { a.x op b.x, a.y op b.y }; } \
FORCEINLINE vec3 operator_op(const vec3& a, const vec3& b) { return { a.x op b.x, a.y op b.y, a.z op b.z }; }
OP(operator+, +);
OP(operator-, -);
OP(operator*, *);
OP(operator/, /);
#undef OP

#define OP(name, op) \
FORCEINLINE vec2 name(const vec2& a, const vec2& b) { return { op(a.x, b.x), op(a.y, b.y) }; } \
FORCEINLINE vec3 name(const vec3& a, const vec3& b) { return { op(a.x, b.x), op(a.y, b.y), op(a.z, b.z) }; }
OP(min, Impl::Min);
OP(max, Impl::Max);
#undef OP

#define OP(name, op) \
FORCEINLINE vec2 name(const vec2& a) { return { op(a.x), op(a.y) }; } \
FORCEINLINE vec3 name(const vec3& a) { return { op(a.x), op(a.y), op(a.z) }; }
OP(abs, Impl::Abs);
#undef OP

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// Sphere - exact
FORCEINLINE flt sdSphere( vec3 p, flt s )
{
	return length(p) - s;
}

// Box - exact
FORCEINLINE flt sdBox(vec3 p, vec3 b)
{
	vec3 q = abs(p) - b;
	return length(max(q, flt(0.0))) + min(max(q.x, max(q.y, q.z)), flt(0.0));
}

// Round Box - exact
FORCEINLINE flt sdRoundBox(vec3 p, vec3 b, flt r)
{
	vec3 q = abs(p) - b;
	return length(max(q, flt(0.0))) + min(max(q.x, max(q.y, q.z)), flt(0.0)) - r;
}

// Torus - exact
FORCEINLINE flt sdTorus(vec3 p, vec2 t)
{
	vec2 q = vec2(length(p.xz()) - t.x, p.y);
	return length(q) - t.y;
}

// Capped Torus - exact
FORCEINLINE flt sdCappedTorus(vec3 p, vec2 sc, flt ra, flt rb)
{
	p.x = abs(p.x);
	flt k = (sc.y * p.x > sc.x* p.y) ? dot(p.xy(), sc) : length(p.xy());
	return sqrt(dot(p, p) + ra * ra - 2.0 * ra * k) - rb;
}

// Link - exact
FORCEINLINE flt sdLink(vec3 p, flt le, flt r1, flt r2)
{
	vec3 q = vec3(p.x, max(abs(p.y) - le, 0.0), p.z);
	return length(vec2(length(q.xy()) - r1, q.z)) - r2;
}

// Infinite Cylinder - exact
FORCEINLINE flt sdCylinder(vec3 p, vec3 c)
{
	return length(p.xz() - c.xy()) - c.z;
}

// Cone - exact
FORCEINLINE flt sdCone(vec3 p, vec2 c, flt h)
{
	// c is the sin/cos of the angle, h is height
	// Alternatively pass q instead of (c,h),
	// which is the point at the base 2D
	vec2 q = h * vec2(c.x / c.y, -1.0);

	vec2 w = vec2(length(p.xz()), p.y);
	vec2 a = w - q * clamp(dot(w, q) / dot(q, q), 0.0, 1.0);
	vec2 b = w - q * vec2(clamp(w.x / q.x, 0.0, 1.0), 1.0);
	flt k = sign(q.y);
	flt d = min(dot(a, a), dot(b, b));
	flt s = max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));
	return sqrt(d) * sign(s);
}

// Cone - bound(not exact!)
FORCEINLINE flt sdConeFast(vec3 p, vec2 c, flt h)
{
	flt q = length(p.xz());
	return max(dot(c.xy(), vec2(q, p.y)), -h - p.y);
}

// Infinite Cone - exact
FORCEINLINE flt sdCone(vec3 p, vec2 c)
{
	// c is the sin/cos of the angle
	vec2 q = vec2(length(p.xz()), -p.y);
	flt d = length(q - c * max(dot(q, c), 0.0));
	return d * ((q.x * c.y - q.y * c.x < 0.0) ? -1.0 : 1.0);
}

// Plane - exact
FORCEINLINE flt sdPlane(vec3 p, vec3 n)
{
	// n must be normalized
	return dot(p, n.xyz());
}

// Hexagonal Prism - exact
FORCEINLINE flt sdHexPrism(vec3 p, vec2 h)
{
	const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
	p = abs(p);
	p.set_xy(p.xy() - 2.0 * min(dot(k.xy(), p.xy()), 0.0) * k.xy());
	vec2 d = vec2(
		length(p.xy() - vec2(clamp(p.x, -k.z * h.x, k.z * h.x), h.x)) * sign(p.y - h.x),
		p.z - h.y);
	return min(max(d.x, d.y), 0.0) + length(max(d, flt(0.0)));
}

// Triangular Prism - bound
FORCEINLINE flt sdTriPrism(vec3 p, vec2 h)
{
	vec3 q = abs(p);
	return max(q.z - h.y, max(q.x * 0.866025 + p.y * 0.5, -p.y) - h.x * 0.5);
}

// Capsule / Line - exact
FORCEINLINE flt sdCapsule(vec3 p, vec3 a, vec3 b, flt r)
{
	vec3 pa = p - a, ba = b - a;
	flt h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
	return length(pa - ba * h) - r;
}

// Capsule / Line - exact
FORCEINLINE flt sdVerticalCapsule(vec3 p, flt h, flt r)
{
	p.y -= clamp(p.y, 0.0, h);
	return length(p) - r;
}

// Capped Cylinder - exact
FORCEINLINE flt sdCappedCylinder(vec3 p, flt h, flt r)
{
	vec2 d = abs(vec2(length(p.xz()), p.y)) - vec2(h, r);
	return min(max(d.x, d.y), flt(0.0)) + length(max(d, flt(0.0)));
}

// Capped Cylinder - exact
FORCEINLINE flt sdCappedCylinder(vec3 p, vec3 a, vec3 b, flt r)
{
	vec3  ba = b - a;
	vec3  pa = p - a;
	flt baba = dot(ba, ba);
	flt paba = dot(pa, ba);
	flt x = length(pa * baba - ba * paba) - r * baba;
	flt y = abs(paba - baba * 0.5) - baba * 0.5;
	flt x2 = x * x;
	flt y2 = y * y * baba;
	flt d = (max(x, y) < 0.0) ? -min(x2, y2) : (((x > 0.0) ? x2 : 0.0) + ((y > 0.0) ? y2 : 0.0));
	return sign(d) * sqrt(abs(d)) / baba;
}

// Rounded Cylinder - exact
FORCEINLINE flt sdRoundedCylinder(vec3 p, flt ra, flt rb, flt h)
{
	vec2 d = vec2(length(p.xz()) - 2.0 * ra + rb, abs(p.y) - h);
	return min(max(d.x, d.y), flt(0.0)) + length(max(d, flt(0.0))) - rb;
}

// Capped Cone - exact
FORCEINLINE flt sdCappedCone(vec3 p, flt h, flt r1, flt r2)
{
	vec2 q = vec2(length(p.xz()), p.y);
	vec2 k1 = vec2(r2, h);
	vec2 k2 = vec2(r2 - r1, 2.0 * h);
	vec2 ca = vec2(q.x - min(q.x, (q.y < 0.0) ? r1 : r2), abs(q.y) - h);
	vec2 cb = q - k1 + k2 * clamp(dot(k1 - q, k2) / dot2(k2), 0.0, 1.0);
	flt s = (cb.x < 0.0 && ca.y < 0.0) ? -1.0 : 1.0;
	return s * sqrt(min(dot2(ca), dot2(cb)));
}

// Capped Cone - exact
FORCEINLINE flt sdCappedCone(vec3 p, vec3 a, vec3 b, flt ra, flt rb)
{
	flt rba = rb - ra;
	flt baba = dot(b - a, b - a);
	flt papa = dot(p - a, p - a);
	flt paba = dot(p - a, b - a) / baba;
	flt x = sqrt(papa - paba * paba * baba);
	flt cax = max(0.0, x - ((paba < 0.5) ? ra : rb));
	flt cay = abs(paba - 0.5) - 0.5;
	flt k = rba * rba + baba;
	flt f = clamp((rba * (x - ra) + paba * baba) / k, 0.0, 1.0);
	flt cbx = x - ra - f * rba;
	flt cby = paba - f;
	flt s = (cbx < 0.0 && cay < 0.0) ? -1.0 : 1.0;
	return s * sqrt(min(cax * cax + cay * cay * baba,
		cbx * cbx + cby * cby * baba));
}

// Solid Angle - exact
FORCEINLINE flt sdSolidAngle(vec3 p, vec2 c, flt ra)
{
	// c is the sin/cos of the angle
	vec2 q = vec2(length(p.xz()), p.y);
	flt l = length(q) - ra;
	flt m = length(q - c * clamp(dot(q, c), 0.0, ra));
	return max(l, m * sign(c.y * q.x - c.x * q.y));
}

// Round cone - exact
FORCEINLINE flt sdRoundCone(vec3 p, flt r1, flt r2, flt h)
{
	vec2 q = vec2(length(p.xz()), p.y);

	flt b = (r1 - r2) / h;
	flt a = sqrt(1.0 - b * b);
	flt k = dot(q, vec2(-b, a));

	if (k < 0.0) return length(q) - r1;
	if (k > a* h) return length(q - vec2(0.0, h)) - r2;

	return dot(q, vec2(a, b)) - r1;
}

// Round Cone - exact
FORCEINLINE flt sdRoundCone(vec3 p, vec3 a, vec3 b, flt r1, flt r2)
{
	// sampling independent computations (only depend on shape)
	vec3  ba = b - a;
	flt l2 = dot(ba, ba);
	flt rr = r1 - r2;
	flt a2 = l2 - rr * rr;
	flt il2 = 1.0 / l2;

	// sampling dependant computations
	vec3 pa = p - a;
	flt y = dot(pa, ba);
	flt z = y - l2;
	flt x2 = dot2(pa * l2 - ba * y);
	flt y2 = y * y * l2;
	flt z2 = z * z * l2;

	// single square root!
	flt k = sign(rr) * rr * rr * x2;
	if (sign(z) * a2 * z2 > k) return  sqrt(x2 + z2) * il2 - r2;
	if (sign(y) * a2 * y2 < k) return  sqrt(x2 + y2) * il2 - r1;
	return (sqrt(x2 * a2 * il2) + y * rr) * il2 - r1;
}

// Ellipsoid - bound(not exact!)
FORCEINLINE flt sdEllipsoid(vec3 p, vec3 r)
{
	flt k0 = length(p / r);
	flt k1 = length(p / (r * r));
	return k0 * (k0 - 1.0) / k1;
}

// Octahedron - exact
FORCEINLINE flt sdOctahedron(vec3 p, flt s)
{
	p = abs(p);
	flt m = p.x + p.y + p.z - s;
	vec3 q;
	if (3.0 * p.x < m) q = p.xyz();
	else if (3.0 * p.y < m) q = p.yzx();
	else if (3.0 * p.z < m) q = p.zxy();
	else return m * 0.57735027;

	flt k = clamp(0.5 * (q.z - q.y + s), 0.0, s);
	return length(vec3(q.x, q.y - s + k, q.z - k));
}

// Octahedron - bound(not exact)
FORCEINLINE flt sdOctahedronFast(vec3 p, flt s)
{
	p = abs(p);
	return (p.x + p.y + p.z - s) * 0.57735027;
}

// Pyramid - exact
FORCEINLINE flt sdPyramid(vec3 p, flt h)
{
	flt m2 = h * h + 0.25;

	p.set_xz(abs(p.xz()));
	p.set_xz((p.z > p.x) ? p.zx() : p.xz());
	p.set_xz(p.xz() - flt(0.5));

	vec3 q = vec3(p.z, h * p.y - 0.5 * p.x, h * p.x + 0.5 * p.y);

	flt s = max(-q.x, 0.0);
	flt t = clamp((q.y - 0.5 * p.z) / (m2 + 0.25), 0.0, 1.0);

	flt a = m2 * (q.x + s) * (q.x + s) + q.y * q.y;
	flt b = m2 * (q.x + 0.5 * t) * (q.x + 0.5 * t) + (q.y - m2 * t) * (q.y - m2 * t);

	flt d2 = min(q.y, -q.x * m2 - q.y * 0.5) > 0.0 ? 0.0 : min(a, b);

	return sqrt((d2 + q.z * q.z) / m2) * sign(max(q.z, -p.y));
}

// Triangle - exact
FORCEINLINE flt udTriangle(vec3 p, vec3 a, vec3 b, vec3 c)
{		
	vec3 ba = b - a; vec3 pa = p - a;
	vec3 cb = c - b; vec3 pb = p - b;
	vec3 ac = a - c; vec3 pc = p - c;
	vec3 nor = cross(ba, ac);

	return sqrt(
		(sign(dot(cross(ba, nor), pa)) +
			sign(dot(cross(cb, nor), pb)) +
			sign(dot(cross(ac, nor), pc)) < 2.0)
		?
		min(min(
			dot2(ba * clamp(dot(ba, pa) / dot2(ba), 0.0, 1.0) - pa),
			dot2(cb * clamp(dot(cb, pb) / dot2(cb), 0.0, 1.0) - pb)),
			dot2(ac * clamp(dot(ac, pc) / dot2(ac), 0.0, 1.0) - pc))
		:
		dot(nor, pa) * dot(nor, pa) / dot2(nor));
}

// Quad - exact
FORCEINLINE flt udQuad(vec3 p, vec3 a, vec3 b, vec3 c, vec3 d)
{
	vec3 ba = b - a; vec3 pa = p - a;
	vec3 cb = c - b; vec3 pb = p - b;
	vec3 dc = d - c; vec3 pc = p - c;
	vec3 ad = a - d; vec3 pd = p - d;
	vec3 nor = cross(ba, ad);

	return sqrt(
		(sign(dot(cross(ba, nor), pa)) +
			sign(dot(cross(cb, nor), pb)) +
			sign(dot(cross(dc, nor), pc)) +
			sign(dot(cross(ad, nor), pd)) < 3.0)
		?
		min(min(min(
			dot2(ba * clamp(dot(ba, pa) / dot2(ba), 0.0, 1.0) - pa),
			dot2(cb * clamp(dot(cb, pb) / dot2(cb), 0.0, 1.0) - pb)),
			dot2(dc * clamp(dot(dc, pc) / dot2(dc), 0.0, 1.0) - pc)),
			dot2(ad * clamp(dot(ad, pd) / dot2(ad), 0.0, 1.0) - pd))
		:
		dot(nor, pa) * dot(nor, pa) / dot2(nor));
}

//////////////////////////////////////////////////////////////////////////////

FORCEINLINE flt opSmoothUnion(flt d1, flt d2, flt k)
{
	flt h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
	return mix(d2, d1, h) - k * h * (1.0 - h);
}

FORCEINLINE flt opSmoothSubtraction(flt d1, flt d2, flt k) {
	flt h = clamp(0.5 - 0.5 * (d2 + d1) / k, 0.0, 1.0);
	return mix(d2, -d1, h) + k * h * (1.0 - h);
}

FORCEINLINE flt opSmoothIntersection(flt d1, flt d2, flt k)
{
	flt h = clamp(0.5 - 0.5 * (d2 - d1) / k, 0.0, 1.0);
	return mix(d2, d1, h) + k * h * (1.0 - h);
}