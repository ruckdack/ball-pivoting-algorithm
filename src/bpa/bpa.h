#include "primitives.h"

#include "query.h"
#include "front.h"

#include <list>
#include <optional>
#include <tuple>

class BPA {
    private:
        const Vertices vertices;
        const double ballRadius;
        std::list<Triangle> faces;
        Query query;
        Front front;
        bool done;
        std::list<VertexIndex> usedVertices;
        bool used(VertexIndex vertexIndex);
        void printIndexFace(Triangle triangle);
        bool multiInsert(Edge edge, std::vector<VertexIndex> vertexIndicees, Vertex ballPosition);
        std::optional<std::tuple<Edge, std::vector<VertexIndex>, Vertex>> findSeedTriangles();   
        std::optional<std::tuple<std::vector<VertexIndex>, Vertex>> ballPivot(const Edge edge, const Vertex ballPosition, const std::optional<VertexIndex> correspondingVertex);
        double calcStartingScalarProduct(const Vertex edgeI, const Vertex edgeJ, const Vertex correspondingVertex, const Vertex ballPosition);
        std::vector<std::tuple<Edge, VertexIndex>> triangulatePlanar(const Edge edge, const std::vector<VertexIndex> &vertexIndicees);
        std::vector<Vertex> intersectCircleSphere(const Vertex circleCenter, const double circleRadius, const Vector circleNormal, const Vertex sphereCenter, const double sphereRadius);
    public:
        BPA(const Vertices &vertices, const double ballRadius);
        bool isDone();
        void step();
        bool boundaryWasFound();
        size_t numOfUsedVertices();
        std::list<Triangle> getFaces();
};
