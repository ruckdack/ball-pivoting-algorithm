#include "primitives.h"

#include <list>
#include <optional>
#include <tuple>
#include <map>

namespace BPA {
    typedef std::list<Edge> Loop;
    class Front {
        private:
            const Vertices &vertices;
            std::list<Loop> front;
            std::list<Edge> boundary;
            std::map<std::string, Vertex> ballPositions;
            std::map<std::string, VertexIndex> correspondingTriangle;
            bool areConsecutive(Loop &loop, Loop::iterator edge1Iterator, Loop::iterator edge2Iterator);
            bool boundaryContains(Edge edge);
            bool loopIntegrity(Loop &loop);
            bool integrity();
        public:
            Front(Vertices &verticies);
            // returns the active edge, the ball position and the corresponding vertex to form a triangle (to prevent self-rolling)
            std::optional<std::tuple<Edge, Vertex, VertexIndex>> getActiveEdge();
            void join(Edge edge, VertexIndex vertexIndex, Vertex ballPosition);
            void glue(Edge edge1, Edge edge2);
            void insertSeedTriangle(Edge edge1, Edge edge2, Edge edge3, Vertex ballPosition);
            bool contains(VertexIndex vertexIndex);
            bool contains(Edge edge);
            void markAsBoundary(Edge edge);
            bool nonemptyBoundary();
    };
    class Query {
        private:
            const Vertices &vertices;
            const float voxelSize;
            std::map<std::tuple<int64_t, int64_t, int64_t>, std::vector<VertexIndex>> grid;
        public:
            Query(Vertices &vertices, float voxelSize);
            std::vector<VertexIndex> getNeighbourhood(VertexIndex vertexIndex);
            std::vector<VertexIndex> getNeighbourhood(Edge edge);
    };
    std::optional<std::tuple<Triangle, Vertex>> findSeedTriangle(const Vertices &vertices, Query query, const float ballRadius);    
    std::optional<std::tuple<VertexIndex, Vertex>> ballPivot(const Vertices &vertices, Query query, const Edge edge, const Vertex ballPosition, const float ballRadius, const std::optional<VertexIndex> correspondingVertex);
    float calcStartingScalarProduct(Vertex edgeI, Vertex edgeJ, Vertex correspondingVertex, Vertex ballPosition, float maxRollingAngle);
    std::vector<Vertex> intersectCircleSphere(const Vertex circleCenter, const float circleRadius, const Vector circleNormal, const Vertex sphereCenter, const float sphereRadius);
}
