#pragma once

#include <vector>
#include <list>
#include <string>
#include <unordered_set>

std::vector<std::string> split(const std::string str, const char delimiter);
std::vector<size_t> findChar(const std::string str, const char c);
bool pathSyntaxValid(std::string path);
template <typename T> std::vector<T> convertToVector(std::unordered_set<T> &c);
bool equals(double f1, double f2);
bool assertionEquals(double f1, double f2);
double roundToDigits(double value, size_t digits);
#include "helpers_templates.h"