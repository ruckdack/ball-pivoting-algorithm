#include "bpa_details.h"

using namespace BPA;

BPA::Query::Query(Vertices &v, float size) : vertices(v), voxelSize(size) {
    // more stuff later
}

std::vector<VertexIndex> Query::getNeighbourhood(Vertex v) {
    // dummy implementation
    std::vector<VertexIndex> indicees(vertices.size());
    for (VertexIndex i = 0; i < vertices.size(); i++) {
        indicees[i] = i;
    }
    return indicees;
}

