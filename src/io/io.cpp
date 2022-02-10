#include "io.h"
#include "io_details.h"

#include "../helpers/helpers.h"

#include <fstream>
#include <list>
#include <iostream>

Vertices IO::readVertices(std::string path) {
    // TODO reserve enough space
    Vertices vertices;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("File not found.");
    }
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> splittedLine = Helpers::split(line, ' ');
        if (splittedLine.front() == "v") {
            Vertex vertex = readVertex(splittedLine);
            vertices.emplace_back(vertex);
        }
    }
    return vertices;
}

void IO::writeMesh(std::string path, Vertices &vertices, std::list<Triangle> &faces) {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Creating file failed.");
    }
    for (Vertex vertex : vertices) {
        file << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
    }
    for (Triangle face : faces) {
        file << "f";
        file << " " << face.i + 1 << " " << face.j + 1 << " " << face.k + 1;
        file << std::endl;
    }
}

Vertex IO::readVertex(std::vector<std::string> splittedLine) {
    if (splittedLine.size() != 4) 
        throw std::runtime_error("Wrong file format.");
    float x = std::stof(splittedLine[1]);
    float y = std::stof(splittedLine[2]);
    float z = std::stof(splittedLine[3]);
    return {x, y, z};
}


