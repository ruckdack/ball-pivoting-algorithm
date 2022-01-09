#pragma once

#include "../bpa/primitives.h"

#include <vector>
#include <string>

namespace IO {
    Vertices readVertices(std::string path);
    void writeMesh(std::string path, Vertices &vertices, Faces &faces);
}
