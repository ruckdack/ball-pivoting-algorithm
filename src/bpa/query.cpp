#include "bpa_details.h"

using namespace BPA;

BPA::Query::Query(Vertices &v, float size) : vertices(v), voxelSize(size) {
    // more stuff later
}

std::vector<VertexIndex> Query::getNeighbourhood(VertexIndex vertexIndex) {
    // dummy implementation
    std::vector<VertexIndex> indicees(vertices.size() - 1);
    for (VertexIndex i = 0; i < vertices.size(); i++) {
        if (i != vertexIndex) indicees.push_back(i);
    }
    return indicees;
}

