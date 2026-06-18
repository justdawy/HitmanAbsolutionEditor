#pragma once
#include "Color.h"
class LinearColor
{
public:
	LinearColor() = default;
	LinearColor(const float r, const float g, const float b, const float a = 1.f);
	LinearColor(const Color& Color);
	const float* Data() const;
	const bool operator==(const LinearColor& rhs) const;
	const bool operator!=(const LinearColor& rhs) const;
	Color QuantizeRound() const;
	Color ToFColorSRGB() const;
	Color ToColor(const bool bSRGB) const;
	static float Clamp01NansTo0(const float InValue);
	float r;
	float g;
	float b;
	float a;
	static float sRGBToLinearTable[256];
};