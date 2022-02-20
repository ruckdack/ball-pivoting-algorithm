#pragma once

#include <vector>
#include <string>
#include <list>

struct Vertex {
    double x;
    double y;
    double z;
};
typedef size_t VertexIndex;
typedef std::vector<Vertex> Vertices;

// indexes a point vector
struct Edge {
    VertexIndex i;
    VertexIndex j;
};

// indexes a point vector
struct Triangle {
    VertexIndex i;
    VertexIndex j;
    VertexIndex k;
};

struct Vector {
    double x;
    double y;
    double z;
};
typedef std::vector<Vector> Vectors;

struct PointIndex {
    VertexIndex vertexIndex;
    VertexIndex normalIndex;
};

struct Points {
    Vertices vertices;
    Vectors normals;
    size_t size;
};

struct Circle {
    Vertex center;
    double radius;
    Vector normal;
};

struct Sphere {
    Vertex center;
    double radius;
};

typedef std::list<Edge> Loop;

std::string toString(const Edge edge);

const bool operator==(const Edge edge1, const Edge edge2);
const bool operator!=(const Edge edge1, const Edge edge2);
const bool operator==(const Triangle triangle1, const Triangle triangle2);
const bool operator!=(const Triangle triangle1, const Triangle triangle2);

bool equals(const Vertex vertex1, const Vertex vertex2);
bool same(const Vertex vertex1, const Vertex vertex2);

Vector conn(const Vertex from, const Vertex to);
double dist(const Vertex from, const Vertex to);
Vertex toVertex(const Vector vector);
Vector operator+(const Vector vector1, const Vector vector2);
Vector operator-(const Vector vector1, const Vector vector2);
double operator*(const Vector vector1, const Vector vector2);
Vector operator*(const double scale, const Vector vector);
Vector cross(const Vector vector1, const Vector vector2);
double len(const Vector vector);
Vector setMag(const Vector vector, const double newMag);