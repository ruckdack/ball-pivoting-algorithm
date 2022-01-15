#include "bpa/primitives.h"
#include "bpa/bpa.h"
#include "helpers/helpers.h"
#include "io/io.h"

#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc > 4) {
        std::cout << "Too many arguments." << std::endl;
        return 0;
    } else if (argc < 4) {
        std::cout << "Not enough arguments." << std::endl;
        return 0;
    }
    float ballRadius = std::stof(argv[1]);
    std::string inputPath(argv[2]);
    std::string outputPath(argv[3]);
    if (!Helpers::pathSyntaxValid(inputPath) || !Helpers::pathSyntaxValid(outputPath)) {
        throw std::runtime_error("Wrong path syntax.");
    }
    Vertices vertices = IO::readVertices(inputPath);
    Faces faces = BPA::bpa(vertices, ballRadius);
    if (faces.size() <= 0) {
        std::cout << "no faces" << std::endl;
    }
    IO::writeMesh(outputPath, vertices, faces);
    return 0;
}

// TODO check which function inputs could be const
// TODO put reference AND-sign to type