#include "bpa_details.h"

using namespace BPA;

Front::Front(Vertices &v) : vertices(v) {}

std::optional<Edge> Front::getActiveEdge() {
    return std::nullopt;
}

void Front::join(Edge edge, VertexIndex vertexIndex) {

}

void Front::glue(Edge edge1, Edge edge2) {

}

void Front::insertEdge(Edge edge) {

}

bool Front::contains(VertexIndex vertexIndex) {
    return false;
}

bool Front::contains(Edge edge) {
    return false;
}

void Front::markAsBoundary(Edge edge) {
    
}