#pragma once

#include "../bpa/bpa.h"
#include "../bpa/primitives.h"

#include <vector>
#include <list>

namespace Normals {
    struct VertexNormals {
        Vectors normals;
        std::vector<float> normalDeviations;
    };
    VertexNormals calculateVertexNormals(const Points &points, const std::vector<Triangle> &faces, const Vectors &faceNormals); // ranges from 0: no deviation to 1: opposite direction
    float calculateInternalAngle(const Vertices &vertices, const Triangle triangle, const VertexIndex vertexIndex);
}
