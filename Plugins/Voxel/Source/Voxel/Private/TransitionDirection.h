#pragma once

enum TransitionDirection { XMin, XMax, YMin, YMax, ZMin, ZMax };

FORCEINLINE TransitionDirection InvertTransitionDirection(TransitionDirection Direction)
{
	if (Direction % 2 == 0)
	{
		return (TransitionDirection)(Direction + 1);
	}
	else
	{
		return (TransitionDirection)(Direction - 1);
	}
}