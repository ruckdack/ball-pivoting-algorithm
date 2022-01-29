#include "bpa.h"
#include "bpa_details.h"

#include "../helpers/helpers.h"
#include "../io/io.h"

#include <cassert>
#include <optional>
#include <cmath>
#include <set>
#include <iostream>
#include <map>

#define EPS 0.0001f
#define asserteq(val1, val2, msg) assert(((void)msg, val1 == val2))
#define asserteqf(val1, val2, msg) assert(((void)msg, std::abs(val1 - val2) < EPS))

Faces BPA::bpa(Vertices vertices, const float ballRadius) {
    std::list<Triangle> faces;
    Front front(vertices);
    Query query(vertices, 2*ballRadius);
    std::list<VertexIndex> usedVertices;

    auto notUsed = [&](VertexIndex vertexIndex) {
        return usedVertices.end() == std::find(usedVertices.begin(), usedVertices.end(), vertexIndex);
    };

    auto printFace = [&](Triangle triangle) {
        std::cout << "(" << vertices[triangle.i].x << "," << vertices[triangle.i].y << "," << vertices[triangle.i].z << ")"
                << ",(" << vertices[triangle.j].x << "," << vertices[triangle.j].y << "," << vertices[triangle.j].z << ")"
                << ",(" << vertices[triangle.k].x << "," << vertices[triangle.k].y << "," << vertices[triangle.k].z << ")" << std::endl;
    };

    auto printIndexFace = [&](Triangle triangle) {
        std::cout << "(" << triangle.i << "," << triangle.j << "," << triangle.k << ")"<< std::endl;
    };

    if (auto optionalSeedTriangle = findSeedTriangle(vertices, query, ballRadius)) {
        auto [seedTriangle, newBallPosition] = optionalSeedTriangle.value();
        faces.push_back(seedTriangle); std::cout << "inital triangle: ";
        printIndexFace(faces.back());
        VertexIndex i = seedTriangle.i;
        VertexIndex j = seedTriangle.j;
        VertexIndex k = seedTriangle.k;
        usedVertices.push_back(i);
        usedVertices.push_back(j);
        usedVertices.push_back(k);
        front.insertSeedTriangle({i, j}, {j, k}, {k, i}, newBallPosition);
    } else {
        // TODO error message instead: no seed triangle found!
        return Helpers::convertFromListToVector<Triangle>(faces);
    }

    // only for debugging
    std::string path = "../output/test/test_" + std::to_string(faces.size()) + ".obj";
    std::vector<Triangle> f = Helpers::convertFromListToVector<Triangle>(faces);
    IO::writeMesh(path, vertices, f);

    while (auto activeEdge = front.getActiveEdge()) {
        auto [edge, ballPosition, correspondingVertex] = activeEdge.value();
        std::cout << "ball position: (" << ballPosition.x << "," << ballPosition.y << "," << ballPosition.z << ")" << std::endl;
        // for (Vertex vertex : vertices) {
        //     assert(len(conn(ballPosition, vertex)) >= ballRadius - EPS);
        // }
        std::cout << "pivoting on (" << edge.i << "," << edge.j << ")" << std::endl;
        auto optionalVertexIndex = ballPivot(vertices, query, edge, ballPosition, ballRadius, correspondingVertex);
        // ballPositions.erase(toString(edge));
        if (optionalVertexIndex) {
            auto [vertexIndex, newBallPosition] = optionalVertexIndex.value();
            if (notUsed(vertexIndex) || front.contains(vertexIndex)) {
                faces.push_back({edge.i, vertexIndex, edge.j});
                std::cout << "new triangle: "; printIndexFace(faces.back());
                usedVertices.push_back(vertexIndex);
                front.join(edge, vertexIndex, newBallPosition);
                assert(!front.contains(edge));
                if (front.contains({vertexIndex, edge.i})) {
                    front.glue({edge.i, vertexIndex}, {vertexIndex, edge.i});
                }
                if (front.contains({edge.j, vertexIndex})) {
                    front.glue({vertexIndex, edge.j}, {edge.j, vertexIndex});
                }
                // only for debugging
                std::string path = "../output/test/test_" + std::to_string(faces.size()) + ".obj";
                std::vector<Triangle> f = Helpers::convertFromListToVector<Triangle>(faces);
                IO::writeMesh(path, vertices, f);
                continue;
            }
        }
        std::cout << ">>>>>    marking as boundary    <<<<<" << std::endl;
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
    // move this code into query
    std::vector<VertexIndex> all = query.getNeighbourhood(minVertex);
    std::set<VertexIndex> neighboursSet(all.begin(), all.end());
    neighboursSet.erase(minVertex);
    std::vector<VertexIndex> neighbours(neighboursSet.begin(), neighboursSet.end());

    Vector awayFromFace = {-1, 0, 0};
    bool foundNeighbour = false;
    VertexIndex maxDotProductIndex = 0;
    float maxDotProduct = -1;
    for (VertexIndex neighbour : neighbours) {
        Vector fromMinToNeighbour = conn(vertices[minVertex], vertices[neighbour]);
        if (len(fromMinToNeighbour) >= 2 * ballRadius) continue;
        foundNeighbour = true;
        float dotProduct = awayFromFace * setMag(fromMinToNeighbour, 1.f);
        if (dotProduct > maxDotProduct) {
            maxDotProductIndex = neighbour;
            maxDotProduct = dotProduct;
        }
    }
    if (!foundNeighbour) {
        std::cout << "no neighbour found for initial vertex" << std::endl;
        return std::nullopt;
    }
    Vector fromMinToNeighbour = conn(vertices[minVertex], vertices[maxDotProductIndex]);
    // find ball position for initial rolling
    Vector pseudoNormal = cross(fromMinToNeighbour, awayFromFace); // this pseudo normal is not normalized (not needed)
    float halfTriangleBaseLength = len(fromMinToNeighbour) * 0.5;
    Vector triangleHeight = setMag(cross(pseudoNormal, fromMinToNeighbour), std::sqrt(ballRadius*ballRadius - halfTriangleBaseLength*halfTriangleBaseLength));
    Vertex ballPosition = toVertex(conn({0,0,0}, vertices[minVertex]) + (0.5 * fromMinToNeighbour) + triangleHeight);
    asserteqf(len(conn(vertices[minVertex], ballPosition)), ballRadius, "inital ball is placed incorrectly");
    asserteqf(len(conn(vertices[maxDotProductIndex], ballPosition)), ballRadius, "inital ball is placed incorrectly");
    Edge firstEdge = {minVertex, maxDotProductIndex};
    // try rolling the ball in one direction
    Triangle seedTriangle;
    auto optionalVertexIndex = ballPivot(vertices, query, firstEdge, ballPosition, ballRadius, std::nullopt);
    if (!optionalVertexIndex) {
        // try rolling into other direction 
        optionalVertexIndex = ballPivot(vertices, query, {firstEdge.j, firstEdge.i}, ballPosition, ballRadius, std::nullopt);
        auto [vertexIndex, newBallPosition] = optionalVertexIndex.value();
        seedTriangle = {firstEdge.i, firstEdge.j, vertexIndex};
        return std::make_tuple(seedTriangle, newBallPosition);
        if (!optionalVertexIndex) {
            std::cout << "inital ball rolling touched no vertex" << std::endl;
            return std::nullopt;
        }
    }
    auto [vertexIndex, newBallPosition] = optionalVertexIndex.value();
    seedTriangle = {firstEdge.j, firstEdge.i, vertexIndex};
    return std::make_tuple(seedTriangle, newBallPosition);
}

std::optional<std::tuple<VertexIndex, Vertex>> BPA::ballPivot(const Vertices &vertices, BPA::Query query, const Edge edge, const Vertex ballPosition, const float ballRadius, const std::optional<VertexIndex> correspondingVertex) {
    // TODO move all of this into query
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

    Vertex e_i = vertices[edge.i];
    Vertex e_j = vertices[edge.j];
    Vertex b = ballPosition;
    float r_b = ballRadius;

    Vector e_i_to_e_j = conn(e_i, e_j); // circle plane normal, not normalized
    Vertex m = toVertex(conn({0,0,0}, e_i) + (0.5 * e_i_to_e_j));
    Vector m_to_b_normalized = setMag(conn(m, b), 1.f);
    float l_e = len(e_i_to_e_j);
    float r_c = std::sqrt(r_b*r_b - 0.25*l_e*l_e);
    asserteqf(len(conn(e_i, b)), r_b, "ball is placed incorrectly");
    asserteqf(len(conn(e_j, b)), r_b, "ball is placed incorrectly");
    Vector n = setMag(cross(e_i_to_e_j, m_to_b_normalized), 1.f);
    asserteqf(n*m_to_b_normalized, 0.f, "normal is not perpendicular to plane(e_i, e_j, b)");
    bool foundNeighbour = false;
    VertexIndex maxDotProductIndex = 0;
    float maxDotProduct = -1.f;
    Vertex newBallPosition;
    for (VertexIndex neighbour : neighbours) {
        if (correspondingVertex.has_value() && correspondingVertex.value() == neighbour) {
            continue;
        }
        std::vector<Vertex> intersections = intersectCircleSphere(m, r_c, setMag(e_i_to_e_j, 1.f), vertices[neighbour], r_b);
        for (Vertex i : intersections) {
            asserteqf(len(conn(e_i,i)), r_b, "intersection is not ball radius away from edgepoint i");
            asserteqf(len(conn(e_j,i)), r_b, "intersection is not ball radius away from edgepoint j");
            asserteqf(len(conn(vertices[neighbour],i)), r_b, "intersection is not ball radius away from neighbour");
            Vector m_to_i_normalized = setMag(conn(m, i), 1.f);
            float n_dot_m_to_i_normalized = n * m_to_i_normalized;
            if (n_dot_m_to_i_normalized < -EPS) continue;
            float p = m_to_b_normalized * m_to_i_normalized;
            if (p > maxDotProduct) {
                foundNeighbour = true;
                maxDotProduct = p;
                maxDotProductIndex = neighbour;
                newBallPosition = i;
            }
        }
    }
    if (!foundNeighbour) {
        std::cout << "ball touched no vertex, checked " << neighbours.size() << " neighbours" << std::endl;
        return std::nullopt;
    }
    return std::make_tuple(maxDotProductIndex, newBallPosition);
}

std::vector<Vertex> BPA::intersectCircleSphere(const Vertex circleCenter, const float circleRadius, const Vector circleNormal, const Vertex sphereCenter, const float sphereRadius) {
    std::vector<Vertex> intersections;
    if (len(conn(circleCenter, sphereCenter)) > circleRadius + sphereRadius) {
        return intersections;
    }
    // first intersect circle plane with sphere -> we get a second circle in the same plane
    // intersect line with direction of circleNormal through sphereCenter with circle plane
    float scale = conn(sphereCenter, circleCenter) * circleNormal;
    if (std::abs(scale) > sphereRadius) return intersections;
    Vertex intersectionCircleCenter = toVertex(conn({0,0,0}, sphereCenter) + setMag(circleNormal, scale));
    float intersectionCircleRadius = std::sqrt(sphereRadius*sphereRadius - scale*scale);
    
    
    // second we intersect both circles
    // we use notation from http://paulbourke.net/geometry/circlesphere/

    Vertex P_0 = circleCenter;
    float r_0 = circleRadius;
    Vertex P_1 = intersectionCircleCenter;
    float r_1 = intersectionCircleRadius;

    Vector P_0_to_P_1 = conn(P_0, P_1);
    float d = len(P_0_to_P_1);
    if (d < std::abs(r_0 - r_1)) return intersections;
    if (d > r_0 + r_1) return intersections;
    float a = (r_0*r_0 - r_1*r_1 + d*d) / (2*d);
    Vertex P_2 = toVertex(conn({0,0,0}, P_0) + setMag(P_0_to_P_1, a));
    if (r_0 - std::abs(a) < EPS) {
        intersections.push_back(toVertex(conn({0,0,0}, P_2)));
        return intersections;
    }
    float h = std::sqrt(r_0*r_0 - a*a);
    Vector dir_h = cross(circleNormal, P_0_to_P_1);
    Vertex i1 = toVertex(conn({0,0,0}, P_2) + setMag(dir_h, h));
    Vertex i2 = toVertex(conn({0,0,0}, P_2) + setMag(dir_h, -h));
    asserteqf(len(conn(i1, circleCenter)), circleRadius, "intersection is not circle radius away from circle center");
    asserteqf(len(conn(i1, sphereCenter)), sphereRadius, "intersection is not sphere radius away from sphere center");
    intersections.push_back(i1);
    intersections.push_back(i2);
    return intersections;
}