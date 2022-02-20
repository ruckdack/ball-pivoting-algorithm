#pragma once

#include "../bpa/primitives.h"

#include <vector>
#include <string>

namespace IO {
    Points readCloud(std::string path);
    void writeMesh(std::string path, Points &points, std::list<Triangle> &faces);
}
