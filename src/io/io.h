#pragma once

#include "../bpa/primitives.h"

#include <vector>
#include <string>
#include <optional>

namespace IO {
    Points readCloud(std::string path);
    void writeMesh(std::string path, Points &points, std::vector<Triangle> &faces, std::vector<float> &normalsDeviations);
}
