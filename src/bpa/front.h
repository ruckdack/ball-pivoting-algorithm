#pragma once

#include "primitives.h"

#include <list>
#include <optional>
#include <tuple>
#include <map>

class Front {
    private:
        std::list<Loop> front;
        std::list<Edge> boundary;
        std::map<std::string, Vertex> ballPositions; // TODO dont use string keys
        std::map<std::string, VertexIndex> correspVertexIndexMap;
        std::map<std::string, std::vector<VertexIndex>> additionalCorrespVertexIndiceesMap;
        // void insertCorrespondingVertex(Edge edge, VertexIndex vertexIndex);
        bool areConsecutive(Loop &loop, Loop::iterator edge1Iterator, Loop::iterator edge2Iterator);
        bool boundaryContains(Edge edge);
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