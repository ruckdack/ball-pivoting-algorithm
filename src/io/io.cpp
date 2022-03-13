#include "io.h"
#include "io_details.h"

#include "../helpers/helpers.h"
#include "../bpa/primitives.h"
#include "../trace.h"

#include <fstream>
#include <list>
#include <iostream>
#include <cassert>

Points IO::readCloud(std::string path) {
    // TODO reserve enough space
    Vertices vertices;
    Vectors normals;
    std::vector<PointIndex> pointIndicees;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("File not found.");
    }
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> splittedLine = split(line, ' ');
        if (splittedLine.size() < 1)
            throw std::runtime_error("Wrong file format.");
        if (splittedLine[0] == "v") {
            Vertex vertex = readVertex(splittedLine);
            vertices.emplace_back(vertex);
        } else if (splittedLine[0] == "vn") {
            Vector normal = readNormal(splittedLine);
            normals.emplace_back(normal);
        } else if (splittedLine[0] == "p") {
            PointIndex pointIndex = readPointIndex(splittedLine);
            pointIndicees.emplace_back(pointIndex);
        }
    }
    if (pointIndicees.size() == 0)
        throw std::runtime_error("No points found in file.");
    Vertices alignedVertices;
    Vectors alignedNormals;
    for (PointIndex pointIndex : pointIndicees) {
        // check indicees
        if (pointIndex.vertexIndex < 0 || pointIndex.vertexIndex >= vertices.size())
            throw std::runtime_error("Wrong indexing.");
        if (pointIndex.normalIndex < 0 || pointIndex.normalIndex >= normals.size())
            throw std::runtime_error("Wrong indexing.");
        alignedVertices.emplace_back(vertices[pointIndex.vertexIndex]);
        alignedNormals.emplace_back(normals[pointIndex.normalIndex]);
    }
    Points points;
    points.vertices = alignedVertices;
    points.normals = alignedNormals;
    points.size = pointIndicees.size();
    ASSERT(points.size == points.vertices.size());
    ASSERT(points.size == points.normals.size());
    return points;
}

Vertex IO::readVertex(std::vector<std::string> splittedLine) {
    if (splittedLine.size() != 4) 
        throw std::runtime_error("Wrong file format (in vertex definition).");
    ASSERT(splittedLine[0] == "v");
    double x = std::stof(splittedLine[1]);
    double y = std::stof(splittedLine[2]);
    double z = std::stof(splittedLine[3]);
    return {x, y, z};
}

Vector IO::readNormal(std::vector<std::string> splittedLine) {
    if (splittedLine.size() != 4) 
        throw std::runtime_error("Wrong file format (in normal definition).");
    ASSERT(splittedLine[0] == "vn");
    double x = std::stof(splittedLine[1]);
    double y = std::stof(splittedLine[2]);
    double z = std::stof(splittedLine[3]);
    Vector normal = {x, y, z};
    return normal;
}

PointIndex IO::readPointIndex(std::vector<std::string> splittedLine) {
    if (splittedLine.size() != 2)
        throw std::runtime_error("Wrong file format (in point definition).");
    ASSERT(splittedLine[0] == "p");
    std::vector<std::string> splittedPoint = split(splittedLine[1], '/');
    if (splittedPoint.size() != 3)
        throw std::runtime_error("Wrong file format (in point definition).");
    PointIndex pointIndex = {};
    uint32_t vertexIndexInt = std::stoi(splittedPoint[0]);
    uint32_t normalIndexInt = std::stoi(splittedPoint[2]);
    if (vertexIndexInt < 1 || normalIndexInt < 1)
        throw std::runtime_error("Wrong indexing.");
    pointIndex.vertexIndex = vertexIndexInt - 1;
    pointIndex.normalIndex = normalIndexInt - 1;
    return pointIndex;
}

void IO::writeMesh(std::string path, Points &points, std::vector<Triangle> &faces, std::vector<float> &normalsDeviations) {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Creating file failed.");
    }
    for (Vertex vertex : points.vertices) {
        file << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
    }
    for (float normalsDeviation : normalsDeviations) {
        file << "vt " << normalsDeviation << " 0.5" << std::endl;
    }
    for (Triangle face : faces) {
        file << "f";
        file << " " << face.i + 1 << "/" << face.i + 1 << "/" << face.i + 1;
        file << " " << face.j + 1 << "/" << face.j + 1 << "/" << face.j + 1;
        file << " " << face.k + 1 << "/" << face.k + 1 << "/" << face.k + 1;
        file << std::endl;
    }
}



