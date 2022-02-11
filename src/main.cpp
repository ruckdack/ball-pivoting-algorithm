#include "bpa/primitives.h"
#include "bpa/bpa.h"
#include "helpers/helpers.h"
#include "io/io.h"

#include <iostream>
#include <string>

#define DEBUG true

int main(int argc, char *argv[]) {
    size_t numOfArgs = 3;
    if (argc > numOfArgs + 1) {
        std::cout << "Too many arguments." << std::endl;
        return 0;
    } else if (argc < numOfArgs + 1) {
        std::cout << "Not enough arguments." << std::endl;
        return 0;
    }
    float ballRadius = std::stof(argv[1]);
    std::string inputPath(argv[2]);
    std::string outputPath(argv[3]);
    if (!Helpers::pathSyntaxValid(inputPath) || !Helpers::pathSyntaxValid(outputPath)) {
        std::cout << "Wrong path syntax." << std::endl;
    }
    // add try catch
    Vertices vertices = IO::readVertices(inputPath);
    BPA bpa(vertices, ballRadius);
    size_t counter = 1;
    while (!bpa.isDone()) {
        bpa.step();
        // only for debugging
        if (DEBUG) {
            std::list<Triangle> faces = bpa.getFaces();
            std::string path = "../output/debug/debug_" + std::to_string(counter) + ".obj";
            IO::writeMesh(path, vertices, faces);
            counter++;
        }
    }
    std::list<Triangle> faces = bpa.getFaces();
    IO::writeMesh(outputPath, vertices, faces);
    return 0;
}

// TODO check which function inputs could be const
// TODO put reference AND-sign to type