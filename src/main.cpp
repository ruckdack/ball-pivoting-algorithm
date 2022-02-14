#include "bpa/primitives.h"
#include "bpa/bpa.h"
#include "helpers/helpers.h"
#include "io/io.h"

#include <iostream>
#include <string>
#include <cmath>

#include "trace.h"


int main(int argc, char *argv[]) {
    size_t numOfArgs = 3;
    if (argc > numOfArgs + 1) {
        ERROUT << "Too many arguments." << std::endl;
        return 0;
    } else if (argc < numOfArgs + 1) {
        ERROUT << "Not enough arguments." << std::endl;
        return 0;
    }
    double ballRadius = std::stof(argv[1]);
    std::string inputPath(argv[2]);
    std::string outputPath(argv[3]);
    if (!pathSyntaxValid(inputPath) || !pathSyntaxValid(outputPath)) {
        ERROUT << "Wrong path syntax." << std::endl;
        return -1;
    }
    try {
        Vertices vertices = IO::readVertices(inputPath);
        INFOUT << "Read " << vertices.size() << " vertices." << std::endl;
        INFOUT << "Running computations..." << std::endl;
        BPA bpa(vertices, ballRadius);
        size_t counter = 1;
        while (!bpa.isDone()) {
            bpa.step();
            // only for debugging
            #ifdef DEBUG
            if (true) {
                std::list<Triangle> faces = bpa.getFaces();
                std::string path = "../output/debug/debug_" + std::to_string(counter) + ".obj";
                IO::writeMesh(path, vertices, faces);
                counter++;
            }
            #endif
        }
        std::list<Triangle> faces = bpa.getFaces();
        INFOUT << "Reconstructed " << faces.size() << (faces.size() == 1 ? " triangle. " : " triangles.") << std::endl;
        INFOUT << "Used " << bpa.numOfUsedVertices() << (bpa.numOfUsedVertices() == 1 ? " vertex (" : " vertices (") << std::round(bpa.numOfUsedVertices() / static_cast<double>(vertices.size()) * 100.0) / 100.0 << "% of total vertices amount)." << std::endl;
        if (bpa.boundaryWasFound()) {
            INFOUT << "A boundary was found." << std::endl;
        }
        IO::writeMesh(outputPath, vertices, faces);
        return 0;
    } catch (const std::runtime_error& error) {
        ERROUT << error.what() << std::endl;
        return -1;
    }
}

// TODO check which function inputs could be const