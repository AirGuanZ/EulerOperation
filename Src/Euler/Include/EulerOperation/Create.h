#pragma once

#include <EulerOperation/Solid.h>

EULER_OPR_BEGIN

struct Polygon2D
{
    using PLoop = std::vector<Vec2>;

    PLoop outerLoop;
    std::vector<PLoop> innerLoops;
};

Solid *CreateFromPolygon(const Polygon2D &polygon, float height);

EULER_OPR_END
