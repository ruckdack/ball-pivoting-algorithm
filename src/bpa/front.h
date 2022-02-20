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
        std::map<std::string, Vertex> ballPositions;
        std::map<std::string, VertexIndex> correspondingTriangle;
        bool areConsecutive(Loop &loop, Loop::iterator edge1Iterator, Loop::iterator edge2Iterator);
        bool boundaryContains(Edge edge);
        bool loopIntegrity(Loop &loop);
        bool integrity();
    public:
        struct ActiveEdge {
            Edge edge;
            Vertex ballPosition;
            VertexIndex correspondingVertex;
        };
        Front() = default;
        std::optional<ActiveEdge> getActiveEdge();
        void join(Edge edge, VertexIndex vertexIndex, Vertex ballPosition);
        void glue(Edge edge1, Edge edge2);
        void insertSeedTriangle(Edge edge1, Edge edge2, Edge edge3, Vertex ballPosition);
        bool contains(VertexIndex vertexIndex);
        bool contains(Edge edge);
        void markAsBoundary(Edge edge);
        bool nonemptyBoundary();
};