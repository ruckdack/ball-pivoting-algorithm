#include "primitives.h"

#include <list>
#include <optional>

namespace BPA {
    class Front {
        private:
            const Vertices &vertices;
            std::list<std::list<Edge>> front;
            std::list<Edge> boundary;
        public:
            Front(Vertices &verticies);
            std::optional<Edge> getActiveEdge();
            void join(Edge edge, VertexIndex vertexIndex);
            void glue(Edge edge1, Edge edge2);
            void insertEdge(Edge edge);
            bool contains(VertexIndex vertexIndex);
            bool contains(Edge edge);
            void markAsBoundary(Edge edge);
    };
    class Query {
        private:
            const Vertices &vertices;
            const float voxelSize;
        public:
            Query(Vertices &vertices, float voxelSize);
            std::vector<VertexIndex> getNeighbourhood(Vertex v);
    };
    std::optional<VertexIndex> ballPivot(Query query, Edge edge);
    std::optional<Triangle> findSeedTriangle(Vertices &vertices);
}
