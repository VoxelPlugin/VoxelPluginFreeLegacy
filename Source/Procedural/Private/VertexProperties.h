#pragma once

struct VertexProperties
{
	bool IsNearXMin;
	bool IsNearXMax;

	bool IsNearYMin;
	bool IsNearYMax;

	bool IsNearZMin;
	bool IsNearZMax;

	bool IsNormalOnly;
};

struct VertexProperties2D
{
	bool IsNearXMin;
	bool IsNearXMax;

	bool IsNearYMin;
	bool IsNearYMax;

	bool IsNearZMin;
	bool IsNearZMax;

	bool NeedTranslation;
};