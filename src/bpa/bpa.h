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
        struct PivotResult {
            Edge edge;
            std::vector<VertexIndex> vertices;
            Vertex ballPosition;
        };
        struct PivotResultStep {
            Edge edge;
            VertexIndex vertex;
            Vertex ballPosition;
            std::vector<VertexIndex> additionalCorrespVertexIndicees;
        };
        const Vertices vertices;
        const Vectors normals;
        const double ballRadius;
        std::list<Triangle> faces;
        Query query;
        Front front;
        bool done;
        std::list<VertexIndex> usedVertices;
        void insertSeedTriangle(PivotResultStep pivotResultStep);
        bool insertPivotResultStep(PivotResultStep pivotResultStep);
        void step();
        bool used(VertexIndex vertexIndex);
        PivotResult findSeedTriangle();
        PivotResult ballPivot(const Edge edge, const Vertex ballPosition, const std::optional<VertexIndex> correspVertexIndex, const std::vector<VertexIndex> additonalCorrespVertexIndicees);
        std::vector<PivotResultStep> serializePivotResult(PivotResult pivotResult);
        double calcStartingScalarProduct(const Vertex edgeI, const Vertex edgeJ, const Vertex correspVertex, const Vertex ballPosition);
        std::vector<Vertex> intersectCircleSphere(const Circle circle, const Sphere sphere);
        std::optional<Circle> intersectSphereSphere(const Sphere sphere1, const Sphere sphere2);
        std::optional<Vertex> calcMinAlongXAxis(const Circle circle);
        void printFaceIndicees(Triangle triangle);
    public:
        struct Result {
            std::list<Triangle> triangles;
            bool boundaryExists;
            size_t numOfUsedVertices;
        };
        BPA(const Points &points, const double ballRadius);
        Result run();
};
