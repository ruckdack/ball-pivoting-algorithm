#include "bpa/primitives.h"
#include "bpa/bpa.h"
#include "helpers/helpers.h"
#include "io/io.h"
#include "trace.h"

#include <iostream>
#include <string>
#include <cmath>
#include <chrono>



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
        return -1;
    }
    try {
        Points points = IO::readCloud(inputPath);
        INFOUT << "Read " << points.size << " vertices." << std::endl;
        INFOUT << "Running computations..." << std::endl;
        auto start = std::chrono::steady_clock::now();
        BPA bpa(points, ballRadius);
        size_t counter = 1;
        while (!bpa.isDone()) {
            bpa.step();
            // only for debugging
            #ifdef DEBUG
            if (false) {
                std::list<Triangle> faces = bpa.getFaces();
                std::string path = "../output/debug/debug_" + std::to_string(counter) + ".obj";
                IO::writeMesh(path, points, faces);
            }
            counter++;
            DBOUT << "count: " << counter << std::endl;
            #endif
        }
        std::list<Triangle> faces = bpa.getFaces();
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed_seconds = end-start;
        INFOUT << "Reconstructed " << faces.size() << (faces.size() == 1 ? " triangle " : " triangles") << " in " << std::round(elapsed_seconds.count()) << " seconds." << std::endl;
        INFOUT << "Used " << bpa.numOfUsedVertices() << (bpa.numOfUsedVertices() == 1 ? " vertex (" : " vertices (")
               << roundToDigits(bpa.numOfUsedVertices() / static_cast<float>(points.size), 2) << "% of total vertices amount)." << std::endl;
        if (bpa.boundaryWasFound()) {
            INFOUT << "A boundary was found." << std::endl;
        }
        IO::writeMesh(outputPath, points, faces);
        return 0;
    } catch (const std::runtime_error& error) {
        ERROUT << error.what() << std::endl;
        return -1;
    }
}

// TODO check which function inputs could be const