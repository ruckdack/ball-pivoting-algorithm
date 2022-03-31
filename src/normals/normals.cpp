#include "normals.h"

#include "../bpa/primitives.h"
#include "../helpers/helpers.h"
#include "../trace.h"

#include <vector>
#include <set>
#include <cassert>
#include <cmath>
#include <iostream>

std::vector<float> Normals::calculateNormalsDeviation(const Points &points, const std::vector<Triangle> &faces) {
    Vertices vertices = points.vertices;
    Vectors pointcloudNormals = points.normals;
    Vectors faceNormals;
    std::vector<float> normalDeviations;
    faceNormals.reserve(faces.size());
    normalDeviations.reserve(points.size);

    std::vector<std::vector<size_t>> faceIndiceesForVertex;
    faceIndiceesForVertex.resize(points.size, std::vector<size_t>());

    // calculate face normals and invert to get from vertex to triangle
    for (size_t faceIndex = 0; faceIndex < faces.size(); faceIndex++) {
        Triangle face = faces[faceIndex];
        Vector ij = conn(vertices[face.i], vertices[face.j]);
        Vector ik = conn(vertices[face.i], vertices[face.k]);
        Vector faceNormal = setMag(cross(ij, ik), 1.0);
        faceNormals.push_back(faceNormal);
        faceIndiceesForVertex[face.i].push_back(faceIndex);
        faceIndiceesForVertex[face.j].push_back(faceIndex);
        faceIndiceesForVertex[face.k].push_back(faceIndex);
        faceIndex++;
    }
    // calculate normals
    for (size_t vertexIndex = 0; vertexIndex < vertices.size(); vertexIndex++) {
        std::vector<size_t> faceIndicees = faceIndiceesForVertex[vertexIndex];
        Vector interpolatedNormal = {0.0, 0.0, 0.0};
        if (faceIndicees.size() == 0) {
            normalDeviations.push_back(1.0);
            continue;
        }
        for (size_t faceIndex : faceIndicees) {
            float internalAngle = calculateInternalAngle(vertices, faces[faceIndex], vertexIndex);
            interpolatedNormal = interpolatedNormal + internalAngle * faceNormals[faceIndex];
        }
        interpolatedNormal = setMag(interpolatedNormal, 1.0);
        Vector pointcloudNormal = setMag(pointcloudNormals[vertexIndex], 1.0);
        float normalDeviation = std::acos(std::clamp(pointcloudNormal*interpolatedNormal, -1.0, 1.0)) * M_1_PI;
        normalDeviations.push_back(normalDeviation);
    }
    return normalDeviations;
}

float Normals::calculateInternalAngle(const Vertices &vertices, const Triangle face, const VertexIndex vertexIndex) {
    std::set<VertexIndex> indexedVertices = {face.i, face.j, face.k};
    indexedVertices.erase(vertexIndex);
    ASSERT(indexedVertices.size() == 2);
    VertexIndex vertex1 = *indexedVertices.begin();
    VertexIndex vertex2 = *indexedVertices.end();
    Vector ray1 = setMag(conn(vertices[vertexIndex], vertices[vertex1]), 1.0);
    Vector ray2 = setMag(conn(vertices[vertexIndex], vertices[vertex2]), 1.0);
    float angle = std::acos(std::clamp(ray1*ray2, -1.0, 1.0));
    return angle;
}