#pragma once

#include "../bpa/primitives.h"

#include <vector>
#include <list>
#include <string>


namespace Helpers {
    std::vector<std::string> split(const std::string str, const char delimiter);
    std::vector<size_t> findChar(const std::string str, const char c);
    bool pathSyntaxValid(std::string path);
    std::vector<Triangle> convertFromListToVector(std::list<Triangle> &list);
}  