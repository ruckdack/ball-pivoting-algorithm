#include "primitives.h"

#include <list>
#include <optional>

namespace BPA {
    typedef std::list<Edge> Loop;
    class Front {
        private:
            const Vertices &vertices;
            std::list<Loop> front;
            std::list<Edge> boundary;
            bool areConsecutive(Loop &loop, Loop::iterator edge1Iterator, Loop::iterator edge2Iterator);
        public:
            Front(Vertices &verticies);
            std::optional<Edge> getActiveEdge();
            void join(Edge edge, VertexIndex vertexIndex);
            void glue(Edge edge1, Edge edge2);
            void insertSeedTriangle(Edge edge1, Edge edge2, Edge edge3);
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
