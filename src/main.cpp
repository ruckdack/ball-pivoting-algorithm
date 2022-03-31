#include "bpa/primitives.h"
#include "bpa/bpa.h"
#include "helpers/helpers.h"
#include "io/io.h"
#include "normals/normals.h"
#include "trace.h"

#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <cassert>



int main(int argc, char *argv[]) {
    #if defined(_OPENMP)
        INFOUT << "OpenMP enabled." << std::endl;
    #endif
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
        return -1;
    }
    try {
        INFOUT << "Reading pointcloud..." << std::endl;
        Points points = IO::readCloud(inputPath);
        INFOUT << "Read " << points.size << " vertices." << std::endl;
        INFOUT << "Running computations..." << std::endl;
        auto start = std::chrono::steady_clock::now();
        BPA bpa(points, ballRadius);
        BPA::Result result = bpa.run();
        std::vector<float> normalsDeviation = Normals::calculateNormalsDeviation(points, result.faces);
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed_seconds = end - start;
        INFOUT << "Reconstructed "
        << result.faces.size() << (result.faces.size() == 1 ? " face " : " faces")
        << " in " << std::round(elapsed_seconds.count()) 
        << (std::round(elapsed_seconds.count()) == 1 ? " second." : " seconds.") << std::endl;
        INFOUT << "Used " << result.numOfUsedVertices << (result.numOfUsedVertices == 1 ? " vertex (" : " vertices (")
               << roundToDigits(result.numOfUsedVertices / static_cast<float>(points.size) * 100, 2) << "% of total vertices amount)." << std::endl;
        if (result.boundaryExists) {
            INFOUT << "A boundary was found." << std::endl;
        }
        IO::writeMesh(outputPath, points, result.faces, normalsDeviation);
    } catch (const std::runtime_error& error) {
        ERROUT << error.what() << std::endl;
        return -1;
    }
}

// TODO check which function inputs could be const