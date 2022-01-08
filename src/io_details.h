#pragma once

#include "primitives.h"

#include <vector>
#include <string>

namespace IO {
    Vertex readVertex(std::vector<std::string> splittedLine);
}