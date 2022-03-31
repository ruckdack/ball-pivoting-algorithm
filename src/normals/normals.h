#pragma once

#include "../bpa/bpa.h"
#include "../bpa/primitives.h"

#include <vector>

namespace Normals {
    void provideNormalsDeviation(Vertices &vertices, const std::vector<Triangle> &faces); // ranges from 0: no deviation to 1: opposite direction
}
