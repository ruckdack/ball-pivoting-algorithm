#pragma once

#include "primitives.h"

#include <list>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

class Front {
    private:
        std::list<Loop> front;
        std::unordered_set<Edge> boundary;
        std::unordered_map<Edge, Vertex> ballPositions;
        std::unordered_map<Edge, VertexIndex> correspVertexIndexMap;
        std::unordered_map<Edge, std::vector<VertexIndex>> additionalCorrespVertexIndiceesMap;
        bool areConsecutive(Loop &loop, Loop::iterator edge1Iterator, Loop::iterator edge2Iterator);
        bool loopIntegrity(Loop &loop);
        bool integrity();
    public:
        struct ActiveEdge {
            Edge edge;
            Vertex ballPosition;
            VertexIndex correspVertexIndex;
            std::vector<VertexIndex> additionalCorrespVertexIndicees;
        };
        Front() = default;
        std::optional<ActiveEdge> getActiveEdge();
        void join(const Edge edge, const VertexIndex vertexIndex, const Vertex ballPosition, const std::vector<VertexIndex> additionalCorrespVertexIndicees);
        void glue(Edge edge1, Edge edge2);
        void insertSeedTriangle(Edge edge1, Edge edge2, Edge edge3, Vertex ballPosition);
        bool contains(VertexIndex vertexIndex);
        bool contains(Edge edge);
        void markAsBoundary(Edge edge);
        bool nonemptyBoundary();
};