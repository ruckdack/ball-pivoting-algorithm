#include "bpa.h"

#include "../helpers/helpers.h"
#include "../io/io.h"

#include <cassert>
#include <optional>
#include <cmath>
#include <set>
#include <iostream>
#include <map>

#define _USE_MATH_DEFINES
#define assertm(eq, msg) assert(((void)msg, eq))

BPA::BPA(const Vertices &v, const float ballRad)
: vertices(v)
, ballRadius(ballRad)
, query(vertices, 2*ballRadius)
, front(vertices)
, done(false) {}

bool BPA::used(VertexIndex vertexIndex) {
    return usedVertices.end() != std::find(usedVertices.begin(), usedVertices.end(), vertexIndex);
}

void BPA::printIndexFace(Triangle triangle) {
    std::cout << "(" << triangle.i << "," << triangle.j << "," << triangle.k << ")"<< std::endl;
}

bool BPA::isDone() {
    return done;
}

void BPA::step() {
    if (faces.size() == 0) {
        if (auto optionalSeedTriangles = findSeedTriangles()) {
            auto [firstEdge, vertexIndicees, newBallPosition] = optionalSeedTriangles.value();
            assert(vertexIndicees.size() > 0);
            auto [_, firstIndex] = triangulatePlanar(firstEdge, vertexIndicees)[0]; // we assert planar vertices
            Triangle seedTriangle = {firstEdge.j, firstEdge.i, firstIndex};
            faces.push_back(seedTriangle);
            std::cout << "inital triangle: "; printIndexFace(faces.back());
            VertexIndex i = seedTriangle.i;
            VertexIndex j = seedTriangle.j;
            VertexIndex k = seedTriangle.k;
            usedVertices.push_back(i);
            usedVertices.push_back(j);
            usedVertices.push_back(k);
            front.insertSeedTriangle({i, j}, {j, k}, {k, i}, newBallPosition);
            if (vertexIndicees.size() > 1) {
                std::vector<VertexIndex> remainingVertexIndicees(vertexIndicees.begin(), vertexIndicees.end());
                auto firstIndexLoc = std::find(remainingVertexIndicees.begin(), remainingVertexIndicees.end(), firstIndex);
                remainingVertexIndicees.erase(firstIndexLoc);
                multiInsert({firstIndex, firstEdge.j}, remainingVertexIndicees, newBallPosition);
            }
        } else {
            // TODO error message instead: no seed triangle found!
            done = true;
            std::cout << "found no seed triangle" << std::endl;
        }
    } else {
        if (auto activeEdge = front.getActiveEdge()) {
            auto [edge, ballPosition, correspondingVertex] = activeEdge.value();
            auto optionalVertexIndicees = ballPivot(edge, ballPosition, correspondingVertex);
            if (optionalVertexIndicees) {
                auto [vertexIndicees, newBallPosition] = optionalVertexIndicees.value();
                bool foundValidVertex = multiInsert(edge, vertexIndicees, newBallPosition);
                if (foundValidVertex) return;
            }
            std::cout << "marking as boundary" << std::endl;
            front.markAsBoundary(edge);
        } else {
            done = true;
            if (front.nonemptyBoundary()) {
                std::cout << "boundary was found" << std::endl;
            }
        }
    }
}

std::list<Triangle> BPA::getFaces() {
    return faces;
}

bool BPA::multiInsert(Edge edge, std::vector<VertexIndex> vertexIndicees, Vertex ballPosition) {
    assert(vertexIndicees.size() > 0);
    bool foundValidVertex = false;
    for (auto [subEdge, vertexIndex] : triangulatePlanar(edge, vertexIndicees)) { // we assert planar vertices
        if (!used(vertexIndex) || front.contains(vertexIndex)) {
            Triangle newFace = {subEdge.i, vertexIndex, subEdge.j};
            for (Triangle face : faces) {
                assert(face != newFace);
            }
            faces.push_back(newFace);
            std::cout << "new triangle: "; printIndexFace(faces.back());
            usedVertices.push_back(vertexIndex);
            front.join(subEdge, vertexIndex, ballPosition);
            assert(!front.contains(subEdge));
            if (front.contains({vertexIndex, subEdge.i})) {
                front.glue({subEdge.i, vertexIndex}, {vertexIndex, subEdge.i});
            }
            if (front.contains({subEdge.j, vertexIndex})) {
                front.glue({vertexIndex, subEdge.j}, {subEdge.j, vertexIndex});
            }
            foundValidVertex = true;
        }
    }
    return foundValidVertex;
}

std::optional<std::tuple<Edge, std::vector<VertexIndex>, Vertex>> BPA::findSeedTriangles() {
    VertexIndex minVertex = 0;
    for (VertexIndex i = 0; i < vertices.size(); i++) {
        if (vertices[i].x < vertices[minVertex].x) {
            minVertex = i;
        }
    }
    std::vector<VertexIndex> neighbours = query.getNeighbourhood(minVertex);

    Vector awayFromFace = {-1, 0, 0};
    std::vector<VertexIndex> maxDotProductIndicees;
    float maxDotProduct = -1;
    for (VertexIndex neighbour : neighbours) {
        Vector fromMinToNeighbour = conn(vertices[minVertex], vertices[neighbour]);
        if (len(fromMinToNeighbour) >= 2 * ballRadius) continue;
        float dotProduct = awayFromFace * setMag(fromMinToNeighbour, 1.f);
        // TODO some values could get cut off
        if (dotProduct > maxDotProduct) {
            maxDotProductIndicees = {neighbour};
            maxDotProduct = dotProduct;
        } else if (Helpers::equals(dotProduct, maxDotProduct)) {
            maxDotProductIndicees.push_back(neighbour);
        }
    }
    if (maxDotProductIndicees.size() == 0) {
        std::cout << "no neighbour found for initial vertex" << std::endl;
        return std::nullopt;
    }
    VertexIndex maxDotProductIndex = maxDotProductIndicees[0];
    for (VertexIndex dotProductIndex : maxDotProductIndicees) {
        if (len(conn(vertices[minVertex], vertices[dotProductIndex])) < len(conn(vertices[minVertex], vertices[maxDotProductIndex]))) {
            maxDotProductIndex = dotProductIndex;
        }
    }
    Vector fromMinToNeighbour = conn(vertices[minVertex], vertices[maxDotProductIndex]);
    // find ball position for initial rolling
    Vector pseudoNormal = cross(fromMinToNeighbour, awayFromFace); // this pseudo normal is not normalized (not needed)
    float halfTriangleBaseLength = len(fromMinToNeighbour) * 0.5;
    Vector triangleHeight = setMag(cross(pseudoNormal, fromMinToNeighbour), std::sqrt(ballRadius*ballRadius - halfTriangleBaseLength*halfTriangleBaseLength));
    Vertex ballPosition = toVertex(conn({0,0,0}, vertices[minVertex]) + (0.5 * fromMinToNeighbour) + triangleHeight);
    assertm(Helpers::equals(len(conn(vertices[minVertex], ballPosition)), ballRadius), "inital ball is placed incorrectly");
    assertm(Helpers::equals(len(conn(vertices[maxDotProductIndex], ballPosition)), ballRadius), "inital ball is placed incorrectly");
    Edge firstEdge = {minVertex, maxDotProductIndex};
    // try rolling the ball in one direction
    auto optionalVertexIndicees = ballPivot(firstEdge, ballPosition, std::nullopt);
    if (!optionalVertexIndicees.has_value()) {
        // try rolling into other direction
        firstEdge = {firstEdge.j, firstEdge.i};
        optionalVertexIndicees = ballPivot(firstEdge, ballPosition, std::nullopt);
        if (!optionalVertexIndicees.has_value()) {
            std::cout << "inital ball rolling touched no vertex" << std::endl;
            return std::nullopt;
        }
        auto [vertexIndicees, newBallPosition] = optionalVertexIndicees.value();
        return std::make_tuple(firstEdge, vertexIndicees, newBallPosition);
    }
    auto [vertexIndicees, newBallPosition] = optionalVertexIndicees.value();
    return std::make_tuple(firstEdge, vertexIndicees, newBallPosition);
}

std::optional<std::tuple<std::vector<VertexIndex>, Vertex>> BPA::ballPivot(const Edge edge, const Vertex ballPosition, const std::optional<VertexIndex> correspondingVertex) {

    std::vector<VertexIndex> neighbours = query.getNeighbourhood(edge);
    Vertex e_i = vertices[edge.i];
    Vertex e_j = vertices[edge.j];
    std::cout << "rolling on: (" << edge.i << "," << edge.j << ")" << std::endl;
    Vertex b = ballPosition;
    float r_b = ballRadius;

    Vector e_i_to_e_j = conn(e_i, e_j); // circle plane normal, not normalized
    Vertex m = toVertex(conn({0,0,0}, e_i) + (0.5 * e_i_to_e_j));
    Vector m_to_b_normalized = setMag(conn(m, b), 1.f);
    float l_e = len(e_i_to_e_j);
    float r_c = std::sqrt(r_b*r_b - 0.25*l_e*l_e);
    assertm(Helpers::equals(len(conn(e_i, b)), r_b), "ball is placed incorrectly");
    assertm(Helpers::equals(len(conn(e_j, b)), r_b), "ball is placed incorrectly");
    Vector n = setMag(cross(e_i_to_e_j, m_to_b_normalized), 1.f);
    assertm(Helpers::equals(n*m_to_b_normalized, 0.f), "normal is not perpendicular to plane(e_i, e_j, b)");
    std::vector<std::tuple<VertexIndex, Vertex>> newVertexIndiceesWithIntersection;
    float maxDotProduct = correspondingVertex.has_value() ? calcStartingScalarProduct(e_i, e_j, vertices[correspondingVertex.value()], b) : -1.f;
    std::cout << "starting dot product: " << maxDotProduct << std::endl;
    Vertex newBallPosition;
    for (VertexIndex neighbour : neighbours) {
        if (correspondingVertex.has_value() && correspondingVertex.value() == neighbour) {
            continue;
        }
        std::vector<Vertex> intersections = intersectCircleSphere(m, r_c, setMag(e_i_to_e_j, 1.f), vertices[neighbour], r_b);
        for (Vertex i : intersections) {
            assertm(Helpers::equals(len(conn(e_i, i)), r_b), "intersection is not ball radius away from edgepoint i");
            assertm(Helpers::equals(len(conn(e_j, i)), r_b), "intersection is not ball radius away from edgepoint j");
            assertm(Helpers::equals(len(conn(vertices[neighbour], i)), r_b), "intersection is not ball radius away from neighbour");
            Vector m_to_i_normalized = setMag(conn(m, i), 1.f);
            float p = m_to_b_normalized * m_to_i_normalized;
            if (Helpers::equals(len(conn(i, b)), 0.f)) {
                // we have not rolled the ball yet
                assertm(used(neighbour), "unused vertices should not be found before rolling (they should already have been added in a prior rolling step)");
                continue;
            }
            if (n * m_to_i_normalized < 0.f) {
                // in this case ball is rotated more than 180°, we use the "extended" scalar product 
                p = -p - 2.f;
            }
            if (p > maxDotProduct) {
                maxDotProduct = p;
                // keep all near intersections
                std::vector<std::tuple<VertexIndex, Vertex>> tmp = {std::make_tuple(neighbour, i)};
                for (auto it = newVertexIndiceesWithIntersection.begin(); it != newVertexIndiceesWithIntersection.end(); it++) {
                    auto [_, intersection] = *it;
                    if (Helpers::equals(len(conn(intersection, i)), 0.f)) tmp.push_back(*it);
                }
                newVertexIndiceesWithIntersection = tmp;
                newBallPosition = i;
            } else if (Helpers::equals(len(conn(i, newBallPosition)), 0.f)) {
                newVertexIndiceesWithIntersection.push_back(std::make_tuple(neighbour, i));
            }
        }
    }
    std::vector<VertexIndex> newVertexIndicees;
    for (auto [vertexIndex, _] : newVertexIndiceesWithIntersection) {
        newVertexIndicees.push_back(vertexIndex);
    }
    if (newVertexIndicees.size() == 0) {
        std::cout << "ball touched no vertex, checked " << neighbours.size() << " neighbours" << std::endl;
        return std::nullopt;
    }
    std::cout << "new ball pos: (" << newBallPosition.x << "," << newBallPosition.y << "," << newBallPosition.z << ")" << std::endl;
    assertm(Helpers::equals(len(conn(e_i, newBallPosition)), r_b), "new ball is not ball radius away from edgepoint i");
    assertm(Helpers::equals(len(conn(e_j, newBallPosition)), r_b), "new ball is not ball radius away from edgepoint j");
    for (VertexIndex newIndex : newVertexIndicees) {
        assertm(Helpers::equals(len(conn(newBallPosition, vertices[newIndex])), r_b), "new ball is not ball radius away from new vertex");
    }
    for (VertexIndex neighbour : neighbours) {
        if (std::find(newVertexIndicees.begin(), newVertexIndicees.end(), neighbour) != newVertexIndicees.end()) continue;
        if (Helpers::equals(std::abs(len(conn(newBallPosition, vertices[neighbour])) - ballRadius), 0.f)) {
            std::cout << "assertion fail for on surface: " << len(conn(newBallPosition, vertices[neighbour])) - ballRadius << " for vertex: " << neighbour << std::endl;
        }
        assertm(!Helpers::equals(std::abs(len(conn(newBallPosition, vertices[neighbour])) - ballRadius), 0.f), "vertices lay on the ball surface, those should have been captured before");
        if (len(conn(newBallPosition, vertices[neighbour])) <= ballRadius) {
            std::cout << "found vertex " << neighbour << " in ball" << std::endl;
        }
        assertm(len(conn(newBallPosition, vertices[neighbour])) > ballRadius, "vertices lay inside the ball");
    }
    return std::make_tuple(newVertexIndicees, newBallPosition);
}

float BPA::calcStartingScalarProduct(const Vertex edgeI, const Vertex edgeJ, const Vertex correspondingVertex, const Vertex ballPosition) {
    Vector edgeIJ = conn(edgeI, edgeJ);
    Vector planeNormal = setMag(cross(edgeIJ, conn(edgeI, correspondingVertex)), 1.f);
    Vector awayFromEdgeIJ = setMag(cross(edgeIJ, planeNormal), 1.f);
    Vertex midPoint = toVertex(conn({0,0,0}, edgeI) + (0.5 * edgeIJ));
    Vector midPointToBallPos = setMag(conn(midPoint, ballPosition), 1.f);
    // could be optimized: is it possible to only calculate in scalar product?
    float radian = std::acos(std::clamp(midPointToBallPos*planeNormal, -1.f, 1.f));
    assertm(radian >= 0.f, "ball is on wrong side");
    float alpha = midPointToBallPos*awayFromEdgeIJ < 0.f ? M_PI_2-radian+0.0001f : M_PI_2+radian+0.0001f;
    float maxPossibleAngle = 2*M_PI - 2*alpha;
    maxPossibleAngle = maxPossibleAngle * 0.9f;
    if (maxPossibleAngle > M_PI) return -std::cos(maxPossibleAngle) - 2.f;
    return std::cos(maxPossibleAngle);
}

std::vector<std::tuple<Edge, VertexIndex>> BPA::triangulatePlanar(const Edge edge, const std::vector<VertexIndex> &vertexIndicees) {
    // assert planar vertices
    assert(vertexIndicees.size() > 0);
    Vector JtoINormalized = setMag(conn(vertices[edge.j], vertices[edge.i]), 1.f);

    auto sorting = [JtoINormalized, edge, this](const VertexIndex vertexIndex1, const VertexIndex vertexIndex2) {
        Vector JtoVertex1Normalized = setMag(conn(vertices[edge.j], vertices[vertexIndex1]), 1.f);
        Vector JtoVertex2Normalized = setMag(conn(vertices[edge.j], vertices[vertexIndex2]), 1.f);
        return JtoINormalized * JtoVertex1Normalized > JtoINormalized * JtoVertex2Normalized;
    };

    std::vector<VertexIndex> sorted(vertexIndicees.begin(), vertexIndicees.end());
    sorted.push_back(edge.i);
    std::sort(sorted.begin(), sorted.end(), sorting);

    std::vector<std::tuple<Edge, VertexIndex>> triangulated;
    for (auto it = sorted.begin(); it != std::prev(sorted.end()); it++) {
        Edge e = {*it, edge.j};
        triangulated.push_back(std::make_tuple(e, *std::next(it)));
    }
    assert(triangulated.size() + 1 == sorted.size());
    return triangulated;
}

std::vector<Vertex> BPA::intersectCircleSphere(const Vertex circleCenter, const float circleRadius, const Vector circleNormal, const Vertex sphereCenter, const float sphereRadius) {
    assert(!std::isnan(circleCenter.x) && !std::isnan(circleCenter.y) && !std::isnan(circleCenter.z));
    assert(!std::isnan(sphereCenter.x) && !std::isnan(sphereCenter.y) && !std::isnan(sphereCenter.z));
    assert(!std::isnan(circleNormal.x) && !std::isnan(circleNormal.y) && !std::isnan(circleNormal.z));
    assert(circleRadius > 0.f);
    assert(sphereRadius > 0.f);
    assertm(Helpers::equals(len(circleNormal), 1.f), "circle normal is not normalized");
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
    if (Helpers::equals(r_0, std::abs(a))) {
        intersections.push_back(toVertex(conn({0,0,0}, P_2)));
        return intersections;
    }
    float h = std::sqrt(r_0*r_0 - a*a);
    Vector dir_h = cross(circleNormal, P_0_to_P_1);
    Vertex i1 = toVertex(conn({0,0,0}, P_2) + setMag(dir_h, h));
    Vertex i2 = toVertex(conn({0,0,0}, P_2) + setMag(dir_h, -h));
    assertm(Helpers::equals(len(conn(i1, circleCenter)), circleRadius), "intersection is not circle radius away from circle center");
    assertm(Helpers::equals(len(conn(i1, sphereCenter)), sphereRadius), "intersection is not sphere radius away from sphere center");
    intersections.push_back(i1);
    intersections.push_back(i2);
    return intersections;
}