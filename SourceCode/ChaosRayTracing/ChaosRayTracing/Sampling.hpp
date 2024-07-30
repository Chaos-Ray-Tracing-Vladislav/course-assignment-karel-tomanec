#pragma once

#include "Math3D.hpp"

inline float PowerHeuristic(float fPdf, float gPdf)
{
    float f = fPdf;
    float g = gPdf;
    return (f * f) / (f * f + g * g);
}