#include "helpers.h"

#include <regex>
#include <cstdarg>
#include <cmath>
#include <iostream>

#define EPS 1.0e-12

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
        if (!len) {
            splitted.push_back(std::string());
        } else {
            splitted.push_back(str.substr(from, len));
        }
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
    std::regex reg(R"(.*\.obj)");
    return std::regex_match(path, reg);
}

bool equals(double f1, double f2) {
    if (std::abs(f1 - f2) <= EPS)
        return true;
    return std::abs(f1 - f2) <= EPS * std::max(std::abs(f1), std::abs(f2));
}

bool assertionEquals(double f1, double f2) {
    if (std::abs(f1 - f2) <= 10*EPS)
        return true;
    return std::abs(f1 - f2) <= 10*EPS * std::max(std::abs(f1), std::abs(f2));
}

double roundToDigits(double value, size_t digits) {
    return std::round(value * std::pow(10.0, digits)) / std::pow(10.0, digits);
}