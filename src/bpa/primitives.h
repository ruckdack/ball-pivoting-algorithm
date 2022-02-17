#pragma once

#include <vector>
#include <string>
#include <list>

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

struct Vector {
    float x;
    float y;
    float z;
};

typedef std::vector<Vertex> Vertices;
typedef std::list<Edge> Loop;

std::string toString(const Edge edge);

bool operator==(const Edge edge1, const Edge edge2);
bool operator!=(const Edge edge1, const Edge edge2);
bool operator==(const Triangle triangle1, const Triangle triangle2);
bool operator!=(const Triangle triangle1, const Triangle triangle2);

Vector conn(const Vertex from, const Vertex to);
float dist(const Vertex from, const Vertex to);
bool equals(const Vertex vertex1, const Vertex vertex2);
bool same(const Vertex vertex1, const Vertex vertex2);

Vertex toVertex(const Vector vector);
Vector operator+(const Vector vector1, const Vector vector2);
Vector operator-(const Vector vector1, const Vector vector2);
float operator*(const Vector vector1, const Vector vector2);
Vector operator*(const float scale, const Vector vector);
Vector cross(const Vector vector1, const Vector vector2);
float len(const Vector vector);
Vector setMag(const Vector vector, const float newMag);