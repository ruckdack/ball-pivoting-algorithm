#pragma once

#include "../bpa/bpa.h"
#include "../bpa/primitives.h"

#include <vector>
#include <list>

namespace Normals {
    std::vector<float> calculateNormalsDeviation(const Points &points, const std::vector<Triangle> &faces); // ranges from 0: no deviation to 1: opposite direction
    float calculateInternalAngle(const Vertices &vertices, const Triangle triangle, const VertexIndex vertexIndex);
}
