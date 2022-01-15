#include "bpa.h"
#include "bpa_details.h"

#include "../helpers/helpers.h"

#include <optional>
#include <cmath>
#include <set>
#include <iostream>

Faces BPA::bpa(Vertices vertices, const float ballRadius) {
    std::list<Triangle> faces;
    Front front(vertices);
    Query query(vertices, 2*ballRadius);
    std::list<VertexIndex> usedVertices;
    Vertex currentBallPosition;

    auto notUsed = [&](VertexIndex vertexIndex) {
        return usedVertices.end() == std::find(usedVertices.begin(), usedVertices.end(), vertexIndex);
    };

    auto printFace = [&](Triangle triangle) {
        std::cout << "(" << vertices[triangle.i].x << "," << vertices[triangle.i].y << "," << vertices[triangle.i].z << ")"
                << ",(" << vertices[triangle.j].x << "," << vertices[triangle.j].y << "," << vertices[triangle.j].z << ")"
                << ",(" << vertices[triangle.k].x << "," << vertices[triangle.k].y << "," << vertices[triangle.k].z << ")" << std::endl;
    };

    if (auto optionalSeedTriangle = findSeedTriangle(vertices, query, ballRadius)) {
        auto [seedTriangle, newBallPosition] = optionalSeedTriangle.value();
        currentBallPosition = newBallPosition;
        faces.push_back(seedTriangle);
        std::cout << "inital triangle: ";
        printFace(faces.back());
        VertexIndex a = seedTriangle.i;
        VertexIndex b = seedTriangle.j;
        VertexIndex c = seedTriangle.k;
        front.insertSeedTriangle({a, b}, {b, c}, {c, a});
    } else {
        // TODO error message instead: no seed triangle found!
        return Helpers::convertFromListToVector<Triangle>(faces);
    }

    while (auto optionalEdge = front.getActiveEdge()) {
        auto edge = optionalEdge.value();
        auto optionalVertexIndex = ballPivot(vertices, query, edge, currentBallPosition, ballRadius);
        if (optionalVertexIndex) {
            auto [vertexIndex, newBallPosition] = optionalVertexIndex.value();
            currentBallPosition = newBallPosition;
            if (notUsed(vertexIndex) || front.contains(vertexIndex)) {
                faces.push_back({edge.i, vertexIndex, edge.j});
                std::cout << "  next triangle: ";
                printFace(faces.back());
                usedVertices.push_back(edge.i);
                usedVertices.push_back(edge.j);
                usedVertices.push_back(vertexIndex);
                if (front.contains({vertexIndex, edge.i})) front.glue({edge.i, vertexIndex}, {vertexIndex, edge.i});
                if (front.contains({vertexIndex, edge.j})) front.glue({edge.j, vertexIndex}, {vertexIndex, edge.j});
                continue;
            }
        }
        front.markAsBoundary(edge);
    }

    return Helpers::convertFromListToVector<Triangle>(faces);
}

std::optional<std::tuple<Triangle, Vertex>> BPA::findSeedTriangle(const Vertices &vertices, BPA::Query query, const float ballRadius) {
    VertexIndex minVertex = 0;
    for (VertexIndex i = 0; i < vertices.size(); i++) {
        if (vertices[i].x < vertices[minVertex].x) {
            minVertex = i;
        }
    }
    // std::cout << "min vertex at: (" << vertices[minVertex].x << "," << vertices[minVertex].y << "," << vertices[minVertex].z << ")" << std::endl;
    std::vector<VertexIndex> neighbours = query.getNeighbourhood(minVertex);
    Vector awayFromFace = {-1, 0, 0};
    Vector fromMinToNeighbour;
    bool foundNeighbour = false;
    VertexIndex maxDotProductIndex = 0;
    float maxDotProduct = -1;
    for (VertexIndex neighbour : neighbours) {
        Vector fromMinToNeighbour = conn(vertices[minVertex], vertices[neighbour]);
        if (len(fromMinToNeighbour) > 2 * ballRadius) continue; // might not be needed, depends on query configuration
        foundNeighbour = true;
        float dotProduct = awayFromFace * fromMinToNeighbour;
        if (dotProduct > maxDotProduct) {
            maxDotProductIndex = neighbour;
            maxDotProduct = dotProduct;
        }
    }
    if (!foundNeighbour) {
        // std::cout << "no neighbour found for initial vertex" << std::endl;
        return std::nullopt;
    }
    fromMinToNeighbour = conn(vertices[minVertex], vertices[maxDotProductIndex]);
    // find ball position for initial rolling
    Vector pseudoNormal = cross(fromMinToNeighbour, awayFromFace); // this pseudo normal is not normalized (not needed)
    // std::cout << "pseudo normal z part: " << pseudoNormal.z << std::endl;
    float halfTriangleBaseLength = len(fromMinToNeighbour) * 0.5;
    // std::cout << "halfTriangleBaseLength: " << halfTriangleBaseLength << std::endl;
    Vector triangleHeight = setMag(cross(pseudoNormal, fromMinToNeighbour), std::sqrt(ballRadius*ballRadius - halfTriangleBaseLength*halfTriangleBaseLength));
    // std::cout << "triangle height: " << len(triangleHeight) << " and x part: " << triangleHeight.x << std::endl;
    Vertex ballPosition = toVertex(conn({0,0,0}, vertices[minVertex]) + (0.5 * fromMinToNeighbour) + triangleHeight);
    Edge firstEdge = {minVertex, maxDotProductIndex};
    // std::cout << "trying to roll over (" << vertices[minVertex].x << "," << vertices[minVertex].y << "," << vertices[minVertex].z << "(" << minVertex + 1 << ")) and ("
    // << vertices[maxDotProductIndex].x << "," << vertices[maxDotProductIndex].y << "," << vertices[maxDotProductIndex].z << "(" << maxDotProductIndex + 1 <<  ")) with ball at: ("
    // << ballPosition.x << "," << ballPosition.y << "," << ballPosition.z << ")" << std::endl << std::endl;
    // try rolling the ball
    auto optionalVertexIndex = ballPivot(vertices, query, firstEdge, ballPosition, ballRadius);
    if (!optionalVertexIndex) {
        std::cout << "inital ball rolling touched no vertex" << std::endl;
        return std::nullopt;
    }
    auto [vertexIndex, newBallPosition] = optionalVertexIndex.value();
    Triangle seedTriangle = {firstEdge.i, firstEdge.j, vertexIndex};
    return std::make_tuple(seedTriangle, newBallPosition);
}

std::optional<std::tuple<VertexIndex, Vertex>> BPA::ballPivot(const Vertices &vertices, BPA::Query query, const Edge edge, const Vertex ballPosition, const float ballRadius) {
    std::vector<VertexIndex> neighboursI = query.getNeighbourhood(edge.i);
    std::set<VertexIndex> neighboursISet(neighboursI.begin(), neighboursI.end());
    std::vector<VertexIndex> neighboursJ = query.getNeighbourhood(edge.j);
    std::set<VertexIndex> neighboursJSet(neighboursJ.begin(), neighboursJ.end());
    std::set<VertexIndex> neighboursSet;
    std::set_union(neighboursISet.begin(), neighboursISet.end(),
                neighboursJSet.begin(), neighboursJSet.end(),
                std::inserter(neighboursSet, neighboursSet.begin()));
    neighboursSet.erase(edge.i);
    neighboursSet.erase(edge.j);
    std::vector<VertexIndex> neighbours(neighboursSet.begin(), neighboursSet.end());

    Vertex midPoint = toVertex(conn({0,0,0}, vertices[edge.i]) + (0.5 * conn(vertices[edge.i], vertices[edge.j])));
    Vector midPointToBallPosition = conn(midPoint, ballPosition);
    Vector edgeVector = conn(vertices[edge.i], vertices[edge.j]); // circle plane normal, not normalized
    float edgeLength = len(edgeVector);
    float circleRadius = std::sqrt(ballRadius*ballRadius - 0.25*edgeLength*edgeLength);
    Vector edgeToBallPositionNormal = cross(edgeVector, conn(vertices[edge.i], ballPosition));
    bool foundNeighbour = false;
    VertexIndex maxDotProductIndex = 0;
    float maxDotProduct = -1;
    Vertex newBallPosition;
    for (VertexIndex neighbour : neighbours) {
        std::vector<Vertex> intersections = intersectCircleSphere(midPoint, circleRadius, edgeVector, vertices[neighbour], ballRadius);
        for (Vertex intersection : intersections) {
            Vector midPointToIntersection = conn(midPoint, intersection);
            if (edgeToBallPositionNormal * midPointToIntersection < 0) continue;
            float dotProduct = midPointToBallPosition * midPointToIntersection;
            if (dotProduct > maxDotProduct) {
                maxDotProduct = dotProduct;
                maxDotProductIndex = neighbour;
                foundNeighbour = true;
                newBallPosition = intersection;
            }
        }
    }
    if (!foundNeighbour) {
        //std::cout << "ball touched no vertex" << std::endl;
        return std::nullopt;
    }
    return std::make_tuple(maxDotProductIndex, newBallPosition);
}

std::vector<Vertex> BPA::intersectCircleSphere(const Vertex circleCenter, const float circleRadius, const Vector circleNormal, const Vertex sphereCenter, const float sphereRadius) {
    std::vector<Vertex> intersections;
    if (len(conn(circleCenter, sphereCenter)) > circleRadius + sphereRadius) {
        // std::cout << "circle - sphere too far away" << std::endl;
        return intersections;
    }
    // first intersect circle plane with sphere -> we get a second circle in the same plane
    // intersect line with direction of circleNormal through sphereCenter with circle plane
    float scale = conn(sphereCenter, circleCenter) * circleNormal;
    if (scale > sphereRadius) return intersections;
    Vertex intersectionCircleCenter = toVertex(conn({0,0,0}, sphereCenter) + setMag(circleNormal, scale));
    float intersectionCircleRadius = std::sqrt(sphereRadius*sphereRadius - scale*scale);
    // std::cout << "circle radius: " << circleRadius << std::endl;
    // std::cout << "intersection circle radius: " << intersectionCircleRadius << std::endl;
    // second we intersect both circles
    Vector centerToCenter = conn(circleCenter, intersectionCircleCenter);
    float centersDistance = len(centerToCenter);
    // std::cout << "centers distance: " << centersDistance << std::endl;
    if (centersDistance > circleRadius + intersectionCircleRadius) return intersections;
    float temp = centersDistance*centersDistance - intersectionCircleRadius*intersectionCircleRadius + circleRadius*circleRadius;
    float distanceCircleCenterToIntersectionCircleCenter = temp / (2*centersDistance);
    // std::cout << "distance circle center to intersection circle center: " << distanceCircleCenterToIntersectionCircleCenter << std::endl;
    Vector circleCenterToIntersectionLine = setMag(centerToCenter, distanceCircleCenterToIntersectionCircleCenter);
    Vertex intersectionLineCenter = toVertex(conn({0,0,0}, circleCenter) + circleCenterToIntersectionLine);
    float intersectionsDistance = 0.5 / centersDistance * std::sqrt(4*centersDistance*centersDistance*intersectionCircleRadius*intersectionCircleRadius - temp*temp);
    // std::cout << "intersections distance: " << intersectionsDistance << std::endl;
    Vertex intersection1 = toVertex(conn({0,0,0}, intersectionLineCenter) + setMag(circleNormal, intersectionsDistance));
    Vertex intersection2 = toVertex(conn({0,0,0}, intersectionLineCenter) + setMag(circleNormal, -intersectionsDistance));
    intersections.push_back(intersection1);
    intersections.push_back(intersection2);
    // std::cout << "found two intersections: (" << intersection1.x << "," << intersection1.y << "," << intersection1.z << "), (" << intersection2.x << "," << intersection2.y << "," << intersection2.z << ")" << std::endl << std::endl;
    return intersections;
}