#include "bpa.h"
#include "bpa_details.h"

#include "../helpers/helpers.h"

#include <optional>

Faces BPA::bpa(Vertices vertices, float ballRadius) {
    std::list<Triangle> faces;
    Front front(vertices);
    Query query(vertices, 2*ballRadius);
    std::list<VertexIndex> usedVertices;

    while (true) {
        while (auto optionalEdge = front.getActiveEdge()) {
            auto edge = optionalEdge.value();
            if (auto optionalVertexIndex = ballPivot(query, edge)) {
                auto vertexIndex = optionalVertexIndex.value();
                if (false/*TODO check used vertices*/ || !front.contains(optionalVertexIndex.value())) break;
                usedVertices.push_back(edge.a);
                usedVertices.push_back(edge.b);
                usedVertices.push_back(vertexIndex);
            } else {
                front.markAsBoundary(edge);
            }  
        }

        if (auto optionalSeedTriangle = findSeedTriangle(vertices)) {
            auto seedTriangle = optionalSeedTriangle.value();
            faces.push_back(seedTriangle);
            VertexIndex a = seedTriangle.a;
            VertexIndex b = seedTriangle.b;
            VertexIndex c = seedTriangle.c;
            Edge edge1 {a, b};
            Edge edge2 {b, c};
            Edge edge3 {c, a};
            front.insertEdge(edge1);
            front.insertEdge(edge2);
            front.insertEdge(edge3);
        } else {
            return Helpers::convertFromListToVector(faces);
        }
    }
}

std::optional<Triangle> BPA::findSeedTriangle(Vertices &vertices) {
    return std::nullopt;
}

std::optional<VertexIndex> BPA::ballPivot(BPA::Query query, Edge edge) {
    return std::nullopt;
}