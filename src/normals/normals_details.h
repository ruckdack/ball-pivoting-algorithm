#pragma once

#include "../bpa/primitives.h"

#include <vector>

namespace Normals {
    float calculateInternalAngle(const Vertices &vertices, const VertexIndex vertexIndex, const Triangle triangle);
}