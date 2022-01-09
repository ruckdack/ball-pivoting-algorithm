#include "bpa/primitives.h"
#include "bpa/bpa.h"
#include "helpers/helpers.h"
#include "io/io.h"

#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc > 3) {
        std::cout << "Too many arguments." << std::endl;
        return 0;
    } else if (argc < 3) {
        std::cout << "Not enough arguments." << std::endl;
        return 0;
    }
    std::string inputPath(argv[1]);
    std::string outputPath(argv[2]);
    if (!Helpers::pathSyntaxValid(inputPath) || !Helpers::pathSyntaxValid(outputPath)) {
        throw std::runtime_error("Wrong path syntax.");
    }
    Vertices vertices = IO::readVertices(inputPath);
    Faces faces = BPA::bpa(vertices, 10.0);
    IO::writeMesh(outputPath, vertices, faces);
    return 0;
}