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
        return usedVertices.end() == std::find(usedVertices.begin(), usedVertices.end(), vertexIndex);
    };

    while (true) {
        while (auto optionalEdge = front.getActiveEdge()) {
            auto edge = optionalEdge.value();
            if (auto optionalVertexIndex = ballPivot(query, edge); optionalVertexIndex 
                    && (notUsed(optionalVertexIndex.value()) || front.contains(optionalVertexIndex.value()))) {
                auto vertexIndex = optionalVertexIndex.value();
                faces.push_back({edge.i, vertexIndex, edge.j});
                usedVertices.push_back(edge.i);
                usedVertices.push_back(edge.j);
                usedVertices.push_back(vertexIndex);
                if (front.contains({vertexIndex, edge.i})) front.glue({edge.i, vertexIndex}, {vertexIndex, edge.i});
                if (front.contains({vertexIndex, edge.j})) front.glue({edge.j, vertexIndex}, {vertexIndex, edge.j});
            } else {
                front.markAsBoundary(edge);
            }  
        }

        if (auto optionalSeedTriangle = findSeedTriangle(vertices)) {
            auto seedTriangle = optionalSeedTriangle.value();
            faces.push_back(seedTriangle);
            VertexIndex a = seedTriangle.i;
            VertexIndex b = seedTriangle.j;
            VertexIndex c = seedTriangle.k;
            front.insertSeedTriangle({a, b}, {b, c}, {c, a});
        } else {
            return Helpers::convertFromListToVector<Triangle>(faces);
        }
    }
}

std::optional<Triangle> BPA::findSeedTriangle(Vertices &vertices) {
    return std::nullopt;
}

std::optional<VertexIndex> BPA::ballPivot(BPA::Query query, Edge edge) {
    return std::nullopt;
}