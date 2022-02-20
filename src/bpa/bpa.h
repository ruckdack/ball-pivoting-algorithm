#pragma once

#include "primitives.h"
#include "query.h"
#include "front.h"
#include "../trace.h"

#include <list>
#include <optional>
#include <tuple>

class BPA {
    private:
        struct SeedTriangleResult {
            Triangle triangle;
            Vertex ballPosition;
        };
        struct PivotResult {
            Edge pivotEdge;
            VertexIndex foundVertex;
            Vertex ballPosition;
        };
        const Vertices vertices;
        const Vectors normals;
        const double ballRadius;
        std::list<Triangle> faces;
        Query query;
        Front front;
        bool done;
        std::list<VertexIndex> usedVertices;
        void insertSeedTriangle(SeedTriangleResult seedTriangleResult);
        void insertPivotResult(PivotResult pivotResult);
        void step();
        bool used(VertexIndex vertexIndex);
        std::optional<SeedTriangleResult> findSeedTriangle();
        std::optional<PivotResult> ballPivot(const Edge edge, const Vertex ballPosition, const std::optional<VertexIndex> correspondingVertexIndex);
        double calcStartingScalarProduct(const Vertex edgeI, const Vertex edgeJ, const Vertex correspondingVertex, const Vertex ballPosition);
        std::vector<Vertex> intersectCircleSphere(const Circle circle, const Sphere sphere);
        std::optional<Circle> intersectSphereSphere(const Sphere sphere1, const Sphere sphere2);
        std::optional<Vertex> calcMinAlongXAxis(const Circle circle);
        #ifdef DEBUG
        void printFaceIndicees(Triangle triangle);
        #endif
    public:
        struct Result {
            std::list<Triangle> triangles;
            bool boundaryExists;
            size_t numOfUsedVertices;
        };
        BPA(const Points &points, const double ballRadius);
        Result run();
};
