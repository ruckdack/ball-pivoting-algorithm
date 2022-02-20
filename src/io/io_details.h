#pragma once

#include "../bpa/primitives.h"

#include <vector>
#include <string>

namespace IO {
    Vertex readVertex(std::vector<std::string> splittedLine);
    Vector readNormal(std::vector<std::string> splittedLine);
    PointIndex readPointIndex(std::vector<std::string> splittedLine);
}