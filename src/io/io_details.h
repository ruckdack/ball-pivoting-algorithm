#pragma once

#include "../bpa/primitives.h"

#include <vector>
#include <string>

namespace IO {
    struct PointIndex {
        VertexIndex vertexIndex;
        VertexIndex normalIndex;
    };
    Vertex readVertex(const std::vector<std::string> splittedLine);
    Vector readNormal(const std::vector<std::string> splittedLine);
    PointIndex readPointIndex(const std::vector<std::string> splittedLine);
}