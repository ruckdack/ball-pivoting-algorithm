#include "normals.h"
#include "normals_details.h"

#include "../bpa/primitives.h"
#include "../helpers/helpers.h"
#include "../trace.h"

#include <vector>
#include <set>
#include <cassert>
#include <cmath>
#include <iostream>

void Normals::provideNormalsDeviation(Vertices &vertices, const std::vector<Triangle> &faces) {
    std::vector<std::vector<size_t>> faceIndiceesForVertex;
    faceIndiceesForVertex.resize(vertices.size(), std::vector<size_t>());

    // generate inverted data structure to get from vertex to triangle
    for (size_t faceIndex = 0; faceIndex < faces.size(); faceIndex++) {
        Triangle face = faces[faceIndex];
        faceIndiceesForVertex[face.i].push_back(faceIndex);
        faceIndiceesForVertex[face.j].push_back(faceIndex);
        faceIndiceesForVertex[face.k].push_back(faceIndex);
        faceIndex++;
    }
    // calculate normals
    for (size_t vertexIndex = 0; vertexIndex < vertices.size(); vertexIndex++) {
        std::vector<size_t> faceIndicees = faceIndiceesForVertex[vertexIndex];
        Vector interpolatedNormal = {0.0, 0.0, 0.0};
        for (size_t faceIndex : faceIndicees) {
            float internalAngle = calculateInternalAngle(vertices, vertexIndex, faces[faceIndex]);
            interpolatedNormal = interpolatedNormal + internalAngle * faces[faceIndex].normal;
        }
        interpolatedNormal = setMag(interpolatedNormal, 1.0);
        float normalDeviation = std::acos(std::clamp(vertices[vertexIndex].inputNormal*interpolatedNormal, -1.0, 1.0)) * M_1_PI;
        vertices[vertexIndex].normalDeviation = normalDeviation;
    }
}

float Normals::calculateInternalAngle(const Vertices &vertices, const VertexIndex vertexIndex, const Triangle face) {
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