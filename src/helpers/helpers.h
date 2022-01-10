#pragma once

#include <vector>
#include <list>
#include <string>


namespace Helpers {
    std::vector<std::string> split(const std::string str, const char delimiter);
    std::vector<size_t> findChar(const std::string str, const char c);
    bool pathSyntaxValid(std::string path);
    template <typename T> std::vector<T> convertFromListToVector(std::list<T> &list);
    template <typename T> bool contains(std::list<T> list, T element);
}
#include "helpers_templates.h"