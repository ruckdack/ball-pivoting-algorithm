#include "helpers.h"

#include <regex>
#include <cstdarg>

#define EPS 1.0e-05f

std::vector<std::string> split(const std::string str, const char delimiter) {
    std::vector<size_t> locations = findChar(str, delimiter);
    if (locations.front() != 0)
        locations.insert(locations.begin(), -1);
    if (locations.back() != str.size() - 1)
        locations.push_back(str.size());
    std::vector<std::string> splitted;
    for (size_t i = 0; i < locations.size() - 1; i++) {
        size_t from = locations[i] + 1;
        size_t len = locations[i+1] - locations[i] - 1;
        if (!len) break;
        splitted.push_back(str.substr(from, len));
    }
    return splitted;
}

std::vector<size_t> findChar(const std::string str, const char c) {
    std::vector<size_t> locations;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == c)
            locations.push_back(i);
    }
    return locations;
}

bool pathSyntaxValid(std::string path) {
    std::regex re(R"(((\.{1,2}|([A-Za-z_0-9]|\-)+)\/)*([A-Za-z_0-9]|\-)+\.obj)");
    return std::regex_match(path, re);
}

bool equals(float f1, float f2) {
    if (std::abs(f1 - f2) <= EPS)
        return true;
    return std::abs(f1 - f2) <= EPS * std::max(std::abs(f1), std::abs(f2));
}