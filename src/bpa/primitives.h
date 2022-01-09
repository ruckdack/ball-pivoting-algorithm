#pragma once

#include <vector>

struct Vertex {
    float x;
    float y;
    float z;
};

typedef size_t VertexIndex;

// indexes a vertex vector
struct Edge {
    VertexIndex a;
    VertexIndex b;
};

// indexes a vertex vector
struct Triangle {
    VertexIndex a;
    VertexIndex b;
    VertexIndex c;
};

typedef std::vector<Vertex> Vertices;
typedef std::vector<Edge> Edges;
typedef std::vector<Triangle> Faces;