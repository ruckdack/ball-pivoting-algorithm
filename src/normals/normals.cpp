#include "normals.h"

#include "../bpa/primitives.h"
#include "../helpers/helpers.h"
#include "../trace.h"

#include <vector>
#include <set>
#include <cassert>
#include <cmath>
#include <iostream>

Normals::VertexNormals Normals::calculateVertexNormals(const Points &points, const std::vector<Triangle> &faces, const Vectors &faceNormals) {
    ASSERT(faces.size() == faceNormals.size());
    Vertices vertices = points.vertices;
    Vectors pointcloudNormals = points.normals;
    Vectors interpolatedNormals;
    std::vector<float> normalDeviations;
    interpolatedNormals.reserve(points.size);
    normalDeviations.reserve(points.size);

    std::vector<std::vector<size_t>> faceIndiceesForVertex;
    faceIndiceesForVertex.resize(points.size, std::vector<size_t>());
    // for (size_t i = 0; i < points.size; i++) {
    //     std::vector<size_t> empty;
    //     faceIndiceesForVertex.push_back(empty);
    // }
    for (size_t faceIndex = 0; faceIndex < faces.size(); faceIndex++) {
        Triangle face = faces[faceIndex];
        faceIndiceesForVertex[face.i].push_back(faceIndex);
        faceIndiceesForVertex[face.j].push_back(faceIndex);
        faceIndiceesForVertex[face.k].push_back(faceIndex);
        faceIndex++;
    }
    for (size_t vertexIndex = 0; vertexIndex < vertices.size(); vertexIndex++) {
        std::vector<size_t> faceIndicees = faceIndiceesForVertex[vertexIndex];
        Vector interpolatedNormal = {0.0, 0.0, 0.0};
        if (faceIndicees.size() == 0) {
            interpolatedNormals.push_back(interpolatedNormal);
            normalDeviations.push_back(1.0);
            continue;
        }
        for (size_t faceIndex : faceIndicees) {
            float internalAngle = calculateInternalAngle(vertices, faces[faceIndex], vertexIndex);
            interpolatedNormal = interpolatedNormal + internalAngle * faceNormals[faceIndex];
        }
        interpolatedNormal = setMag(interpolatedNormal, 1.0);
        interpolatedNormals.push_back(interpolatedNormal);
        Vector pointcloudNormal = setMag(pointcloudNormals[vertexIndex], 1.0);
        float normalDeviation = std::acos(std::clamp(pointcloudNormal*interpolatedNormal, -1.0, 1.0)) * M_1_PI;
        normalDeviations.push_back(normalDeviation);
    }
    return {pointcloudNormals, normalDeviations};
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