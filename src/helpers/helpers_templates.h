template <typename T> std::vector<T> convertToVector(std::unordered_set<T> &c) {
    std::vector<T> vector;
    vector.reserve(c.size());
    for (auto l : c) {
        vector.push_back(l);
    }
    return vector;
}