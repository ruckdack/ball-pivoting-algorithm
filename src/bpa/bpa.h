#pragma once

#include "primitives.h"
#include "query.h"
#include "front.h"
#include "../trace.h"

#include <optional>
#include <tuple>
#include <unordered_set>
#include <unordered_map>

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
        };
        const Vertices vertices;
        const double ballRadius;
        const bool reuseVertices;
        std::unordered_set<Triangle> faces;
        std::unordered_map<Triangle, Vector> faceNormals;
        Query query;
        Front front;
        bool started;
        bool done;
        bool multiRollingOccured;
        std::unordered_set<VertexIndex> usedVertices;
        void insertSeedTriangle(PivotResultStep pivotResultStep);
        void insertPivotResultStep(PivotResultStep pivotResultStep);
        void step();
        bool used(VertexIndex vertexIndex);
        PivotResult findSeedTriangle();
        PivotResult ballPivot(const Edge edge, const Vertex ballPosition, const std::optional<VertexIndex> correspVertexIndex);
        std::vector<PivotResultStep> serializePivotResult(PivotResult pivotResult);
        double calcStartingScalarProduct(const Vertex edgeI, const Vertex edgeJ, const Vertex correspVertex, const Vertex ballPosition, const double circleRadius);
        std::vector<Vertex> intersectCircleSphere(const Circle circle, const Sphere sphere);
        std::optional<Circle> intersectSphereSphere(const Sphere sphere1, const Sphere sphere2);
        std::optional<Vertex> calcMinAlongXAxis(const Circle circle);
        void printFaceIndicees(Triangle triangle);
    public:
        struct Result {
            std::vector<Triangle> faces;
            size_t numOfUsedVertices;
            bool multiRollingOccured;
        };
        BPA(const Vertices &vertices, const double ballRadius, const bool reuseVertices);
        Result run();
};
