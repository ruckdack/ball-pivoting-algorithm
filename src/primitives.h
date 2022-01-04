#pragma once

#include <vector>

struct Vertex {
    float x;
    float y;
    float z;
};

// indexes a vertex vector
struct Edge {
    size_t a;
    size_t b;
};

// indexes a vertex vector
struct Triangle {
    size_t a;
    size_t b;
    size_t c;
};

typedef std::vector<Vertex> Vertices;
typedef std::vector<Edge> Edges;
typedef std::vector<Triangle> Faces;