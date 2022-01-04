#include "helpers.h"

#include <regex>

std::vector<std::string> Helpers::split(const std::string str, const char delimiter) {
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

std::vector<size_t> Helpers::findChar(const std::string str, const char c) {
    std::vector<size_t> locations;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == c)
            locations.push_back(i);
    }
    return locations;
}

bool Helpers::pathSyntaxValid(std::string path) { // how about regex?  ((".." or "." or all_allowed_symbols+) concat "/"" )* concat all_allowed_symbols+ concat ".obj"
    std::regex re(R"(((\.{1,2}|([A-Za-z_0-9]|\-)+)\/)*([A-Za-z_0-9]|\-)+\.obj)");
    return std::regex_match(path, re);
}