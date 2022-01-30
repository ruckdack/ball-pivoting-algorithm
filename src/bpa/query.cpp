#include "bpa_details.h"

#include <set>
#include <cmath>
#include <iostream>

using namespace BPA;

static const std::vector<int64_t> OFFSETS = {-1, 0, 1};

BPA::Query::Query(Vertices &v, float size) : vertices(v), voxelSize(size) {
    for (VertexIndex i = 0; i < vertices.size(); i++) {
        Vertex v = vertices[i];
        int64_t gridX = std::floor(v.x / voxelSize);
        int64_t gridY = std::floor(v.y / voxelSize);
        int64_t gridZ = std::floor(v.z / voxelSize);
        auto key = std::make_tuple(gridX, gridY, gridZ);
        if (grid.find(key) == grid.end()) {
            grid[key] = std::vector<VertexIndex>(1, i);
        } else {
            grid[key].push_back(i);
        }
    }
    // for (auto it = grid.begin(); it != grid.end(); it++) {
    //     auto [k, v] = *it;
    //     auto [k1, k2, k3] = k;
    //     std::cout << "k: (" << k1 << "," << k2 << "," << k3 << "), v: " << v.size() << std::endl;
    // }
}

std::vector<VertexIndex> Query::getNeighbourhood(VertexIndex vertexIndex) {
    std::vector<VertexIndex> neighbours;
    for (int64_t offsetX : OFFSETS) {
        for (int64_t offsetY : OFFSETS) {
            for (int64_t offsetZ : OFFSETS) {
                Vertex v = vertices[vertexIndex];
                int64_t accessX = std::floor(v.x / voxelSize) + offsetX;
                int64_t accessY = std::floor(v.y / voxelSize) + offsetY;
                int64_t accessZ = std::floor(v.z / voxelSize) + offsetZ;
                auto access = std::make_tuple(accessX, accessY, accessZ);
                std::vector<VertexIndex> newNeighbours = grid[access];
                auto findIterator = std::find(newNeighbours.begin(), newNeighbours.end(), vertexIndex);
                if (findIterator != newNeighbours.end()) {
                    newNeighbours.erase(findIterator);
                }
                neighbours.insert(neighbours.end(), newNeighbours.begin(), newNeighbours.end());
            }
        }
    }
    return neighbours;
}

std::vector<VertexIndex> Query::getNeighbourhood(Edge edge) {
    std::vector<VertexIndex> neighbours;
    for (int64_t offsetX : OFFSETS) {
        for (int64_t offsetY : OFFSETS) {
            for (int64_t offsetZ : OFFSETS) {
                Vertex midpoint = toVertex(conn({0,0,0}, vertices[edge.i]) + 0.5*conn(vertices[edge.i], vertices[edge.j]));
                int64_t accessX = std::floor(midpoint.x / voxelSize) + offsetX;
                int64_t accessY = std::floor(midpoint.y / voxelSize) + offsetY;
                int64_t accessZ = std::floor(midpoint.z / voxelSize) + offsetZ;
                auto access = std::make_tuple(accessX, accessY, accessZ);
                std::vector<VertexIndex> newNeighbours = grid[access];
                auto findIterator = std::find(newNeighbours.begin(), newNeighbours.end(), edge.i);
                if (findIterator != newNeighbours.end()) {
                    newNeighbours.erase(findIterator);
                }
                findIterator = std::find(newNeighbours.begin(), newNeighbours.end(), edge.j);
                if (findIterator != newNeighbours.end()) {
                    newNeighbours.erase(findIterator);
                }
                neighbours.insert(neighbours.end(), newNeighbours.begin(), newNeighbours.end());
            }
        }
    }
    return neighbours;
}

