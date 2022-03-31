#pragma once

#include "../bpa/primitives.h"

#include <vector>
#include <string>
#include <optional>

namespace IO {
    Vertices readCloud(const std::string path);
    void writeMesh(const std::string path, const Vertices &vertices, const std::vector<Triangle> &faces);
}
