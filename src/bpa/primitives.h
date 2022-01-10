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
    VertexIndex i;
    VertexIndex j;
};

// indexes a vertex vector
struct Triangle {
    VertexIndex i;
    VertexIndex j;
    VertexIndex k;
};

typedef std::vector<Vertex> Vertices;
typedef std::vector<Edge> Edges;
typedef std::vector<Triangle> Faces;

bool operator==(const Edge edge1, const Edge edge2);
bool operator!=(const Edge edge1, const Edge edge2);
bool operator==(const Triangle triangle1, const Triangle triangle2);
bool operator!=(const Triangle triangle1, const Triangle triangle2);
