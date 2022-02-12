template <typename T> std::vector<T> convertFromListToVector(std::list<T> &list) {
    std::vector<T> vector;
    vector.reserve(list.size());
    for (auto l : list) {
        vector.push_back(l);
    }
    return vector;
}

template <typename T> bool contains(std::list<T> list, T element) {
    return list.end() != std::find(list.begin(), list.end(), element);
}