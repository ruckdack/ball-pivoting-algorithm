#include "bpa.h"
#include "bpa_details.h"

#include "../helpers/helpers.h"

#include <optional>

Faces BPA::bpa(Vertices vertices, float ballRadius) {
    std::list<Triangle> faces;
    Front front(vertices);
    Query query(vertices, 2*ballRadius);
    std::list<VertexIndex> usedVertices;

    auto notUsed = [&](VertexIndex vertexIndex) {
        return usedVertices.end() != std::find(usedVertices.begin(), usedVertices.end(), vertexIndex);
    };

    while (true) {
        while (auto optionalEdge = front.getActiveEdge()) {
            auto edge = optionalEdge.value();
            if (auto optionalVertexIndex = ballPivot(query, edge); optionalVertexIndex 
                    && (notUsed(optionalVertexIndex.value()) || front.contains(optionalVertexIndex.value()))) {
                auto vertexIndex = optionalVertexIndex.value();
                faces.push_back({edge.a, vertexIndex, edge.b});
                usedVertices.push_back(edge.a);
                usedVertices.push_back(edge.b);
                usedVertices.push_back(vertexIndex);
                if (front.contains({vertexIndex, edge.a})) front.glue({edge.a, vertexIndex}, {vertexIndex, edge.a});
                if (front.contains({vertexIndex, edge.b})) front.glue({edge.b, vertexIndex}, {vertexIndex, edge.b});
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
            front.insertEdge({a, b});
            front.insertEdge({b, c});
            front.insertEdge({c, a});
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