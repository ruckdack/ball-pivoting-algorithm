#pragma once

#include <vector>
#include <string>

namespace Helpers {
    std::vector<std::string> split(const std::string str, const char delimiter);
    std::vector<size_t> findChar(const std::string str, const char c);
    bool pathSyntaxValid(std::string path);
}  