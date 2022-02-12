#include "bpa/primitives.h"
#include "bpa/bpa.h"
#include "helpers/helpers.h"
#include "io/io.h"

#include <iostream>
#include <string>

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
    float ballRadius = std::stof(argv[1]);
    std::string inputPath(argv[2]);
    std::string outputPath(argv[3]);
    if (!pathSyntaxValid(inputPath) || !pathSyntaxValid(outputPath)) {
        ERROUT << "Wrong path syntax." << std::endl;
    }
    try {
        Vertices vertices = IO::readVertices(inputPath);
        BPA bpa(vertices, ballRadius);
        size_t counter = 1;
        while (!bpa.isDone()) {
            bpa.step();
            // only for debugging
            #ifdef DEBUG
            if (false) {
                std::list<Triangle> faces = bpa.getFaces();
                std::string path = "../output/debug/debug_" + std::to_string(counter) + ".obj";
                IO::writeMesh(path, vertices, faces);
                counter++;
            }
            #endif
        }
        std::list<Triangle> faces = bpa.getFaces();
        IO::writeMesh(outputPath, vertices, faces);
        return 0;
    } catch (const std::runtime_error& error) {
        ERROUT << error.what() << std::endl;
        return -1;
    }
}

// TODO check which function inputs could be const
// TODO put reference AND-sign to type