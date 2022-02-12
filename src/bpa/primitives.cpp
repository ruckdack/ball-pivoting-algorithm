#include "primitives.h"

#include "../helpers/helpers.h"

#include <cmath>
#include <iostream>

std::string toString(const Edge edge) {
    return std::to_string(edge.i) + "-" + std::to_string(edge.j);
}

bool operator==(const Edge edge1, const Edge edge2) {
    return edge1.i == edge2.i && edge1.j == edge2.j;
}

bool operator!=(const Edge edge1, const Edge edge2) {
    return !(edge1 == edge2);
}

bool operator==(const Triangle triangle1, const Triangle triangle2) {
    return triangle1.i == triangle2.i && triangle1.j == triangle2.j && triangle1.k == triangle2.k;
}

bool operator!=(const Triangle triangle1, const Triangle triangle2) {
    return !(triangle1 == triangle2);
}

Vector conn(const Vertex from, const Vertex to) {
    return {
        to.x - from.x, 
        to.y - from.y, 
        to.z - from.z, 
    };
}

float dist(const Vertex from, const Vertex to) {
    return len(conn(from, to));
}

bool equals(const Vertex vertex1, const Vertex vertex2) {
    return equals(vertex1.x, vertex2.x) 
    && equals(vertex1.y, vertex2.y) 
    && equals(vertex1.z, vertex2.z);
}

Vertex toVertex(const Vector vector) {
    return {
        vector.x,
        vector.y,
        vector.z
    };
}

Vector operator+(const Vector vector1, const Vector vector2) {
    return {
        vector1.x + vector2.x,
        vector1.y + vector2.y,
        vector1.z + vector2.z
    };
}

Vector operator-(const Vector vector1, const Vector vector2) {
    return vector1 + (-1.f * vector2);
}

float operator*(const Vector vector1, const Vector vector2) {
    return vector1.x * vector2.x + vector1.y * vector2.y + vector1.z * vector2.z;
}

Vector operator*(const float scale, const Vector vector) {
    return {
        scale * vector.x,
        scale * vector.y,
        scale * vector.z
    };
}

Vector cross(const Vector vector1, const Vector vector2) {
    return {
        vector1.y*vector2.z - vector1.z*vector2.y,
        vector1.z*vector2.x - vector1.x*vector2.z,
        vector1.x*vector2.y - vector1.y*vector2.x
    };
}

float len(const Vector vector) {
    return std::sqrt(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
}

Vector setMag(const Vector vector, const float newMag) {
    float l = len(vector);
    return (newMag / l) * vector;
}
