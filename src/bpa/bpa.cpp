#include "bpa.h"

#include "../helpers/helpers.h"
#include "../io/io.h"
#include "../trace.h"

#include <cassert>
#include <optional>
#include <cmath>
#include <set>
#include <iostream>
#include <map>

#define _USE_MATH_DEFINES
#define ASSERTM(eq, msg) assert(((void)msg, eq))
#define ASSERT(eq) assert(eq)

BPA::BPA(const Vertices &v, const double ballRad)
: vertices(v)
, ballRadius(ballRad)
, query(vertices, 2*ballRadius)
, front(vertices)
, done(false) {}

bool BPA::used(VertexIndex vertexIndex) {
    return usedVertices.end() != std::find(usedVertices.begin(), usedVertices.end(), vertexIndex);
}

void BPA::printIndexFace(Triangle triangle) {
    DBOUT << "(" << triangle.i << "," << triangle.j << "," << triangle.k << ")"<< std::endl;
}

bool BPA::isDone() {
    return done;
}

void BPA::step() {
    if (faces.size() == 0) {
        if (auto optionalSeedTriangle = findSeedTriangle()) {
            auto [seedTriangle, newBallPosition] = optionalSeedTriangle.value();
            faces.push_back(seedTriangle);
            DBOUT << "inital triangle: " << std::endl; 
            printIndexFace(seedTriangle);
            VertexIndex i = seedTriangle.i;
            VertexIndex j = seedTriangle.j;
            VertexIndex k = seedTriangle.k;
            usedVertices.push_back(i);
            usedVertices.push_back(j);
            usedVertices.push_back(k);
            front.insertSeedTriangle({i, j}, {j, k}, {k, i}, newBallPosition);
        } else {
            done = true;
            INFOUT << "Found no seed triangle." << std::endl;
        }
    } else {
        if (auto activeEdge = front.getActiveEdge()) {
            auto [edge, ballPosition, correspondingVertex] = activeEdge.value();
            auto optionalVertexIndex = ballPivot(edge, ballPosition, correspondingVertex);
            if (optionalVertexIndex) {
                auto [vertexIndex, newBallPosition] = optionalVertexIndex.value();
                if (!used(vertexIndex) || front.contains(vertexIndex)) {
                    Triangle newFace = {edge.i, vertexIndex, edge.j};
                    for (Triangle face : faces) {
                        ASSERTM(face != newFace, "cannot add already reconstructed face again");
                    }
                    DBOUT << "new triangle: " << std::endl;
                    printIndexFace(newFace);
                    faces.push_back(newFace);
                    usedVertices.push_back(vertexIndex);
                    front.join(edge, vertexIndex, newBallPosition);
                    ASSERT(!front.contains(edge));
                    if (front.contains({vertexIndex, edge.i})) {
                        front.glue({edge.i, vertexIndex}, {vertexIndex, edge.i});
                    }
                    if (front.contains({edge.j, vertexIndex})) {
                        front.glue({vertexIndex, edge.j}, {edge.j, vertexIndex});
                    }
                    return;   
                }
            }
            DBOUT << "marking as boundary" << std::endl;
            front.markAsBoundary(edge);
        } else {
            done = true;
        }
    }
}

bool BPA::boundaryWasFound() {
    return front.nonemptyBoundary();
}

size_t BPA::numOfUsedVertices() {
    return usedVertices.size();
}

std::list<Triangle> BPA::getFaces() {
    return faces;
}

std::optional<std::tuple<Triangle, Vertex>> BPA::findSeedTriangle() {
    VertexIndex minVertex = 0;
    for (VertexIndex i = 0; i < vertices.size(); i++) {
        if (vertices[i].x < vertices[minVertex].x) {
            minVertex = i;
        }
    }
    std::vector<VertexIndex> neighbours = query.getNeighbourhood(minVertex);

    Vector awayFromFace = {-1, 0, 0};
    VertexIndex maxDotProductIndex;
    float foundNeighbour = true;
    double maxDotProduct = -1;
    for (VertexIndex neighbour : neighbours) {
        Vector fromMinToNeighbour = conn(vertices[minVertex], vertices[neighbour]);
        if (len(fromMinToNeighbour) >= 2 * ballRadius) continue;
        double dotProduct = awayFromFace * setMag(fromMinToNeighbour, 1.0);
        if (dotProduct > maxDotProduct) {
            maxDotProductIndex = neighbour;
            maxDotProduct = dotProduct;
            foundNeighbour = true;
        }
    }
    if (!foundNeighbour) {
        DBOUT << "no neighbour found for initial vertex" << std::endl;
        return std::nullopt;
    }
    Vector fromMinToNeighbour = conn(vertices[minVertex], vertices[maxDotProductIndex]);
    // find ball position for initial rolling
    Vector pseudoNormal = cross(fromMinToNeighbour, awayFromFace); // this pseudo normal is not normalized (not needed)
    double halfTriangleBaseLength = len(fromMinToNeighbour) * 0.5;
    Vector triangleHeight = setMag(cross(pseudoNormal, fromMinToNeighbour), std::sqrt(ballRadius*ballRadius - halfTriangleBaseLength*halfTriangleBaseLength));
    Vertex ballPosition = toVertex(conn({0,0,0}, vertices[minVertex]) + (0.5 * fromMinToNeighbour) + triangleHeight);
    ASSERTM(equals(len(conn(vertices[minVertex], ballPosition)), ballRadius), "inital ball is placed incorrectly");
    ASSERTM(equals(len(conn(vertices[maxDotProductIndex], ballPosition)), ballRadius), "inital ball is placed incorrectly");
    Edge firstEdge = {minVertex, maxDotProductIndex};
    // try rolling the ball in one direction
    auto optionalVertexIndex = ballPivot(firstEdge, ballPosition, std::nullopt);
    if (!optionalVertexIndex.has_value()) {
        // try rolling into other direction
        firstEdge = {firstEdge.j, firstEdge.i};
        optionalVertexIndex = ballPivot(firstEdge, ballPosition, std::nullopt);
        if (!optionalVertexIndex.has_value()) {
            DBOUT << "inital ball rolling touched no vertex" << std::endl;
            return std::nullopt;
        }
        auto [vertexIndex, newBallPosition] = optionalVertexIndex.value();
        Triangle seedTriangle = {firstEdge.i, firstEdge.j, vertexIndex};
        return std::make_tuple(seedTriangle, newBallPosition);
    }
    auto [vertexIndex, newBallPosition] = optionalVertexIndex.value();
    Triangle seedTriangle = {firstEdge.j, firstEdge.i, vertexIndex};
    return std::make_tuple(seedTriangle, newBallPosition);
}

std::optional<std::tuple<VertexIndex, Vertex>> BPA::ballPivot(const Edge edge, const Vertex ballPosition, const std::optional<VertexIndex> correspondingVertex) {

    std::vector<VertexIndex> neighbours = query.getNeighbourhood(edge);
    Vertex e_i = vertices[edge.i];
    Vertex e_j = vertices[edge.j];
    DBOUT << std::endl; 
    DBOUT << "rolling on: (" << edge.i << "," << edge.j << ")" << std::endl;
    DBOUT << "ball is at: (" << ballPosition.x << "," << ballPosition.y << "," << ballPosition.z << ")" << std::endl;
    Vertex b = ballPosition;
    double r_b = ballRadius;

    Vector e_i_to_e_j = conn(e_i, e_j); // circle plane normal, not normalized
    Vertex m = toVertex(conn({0,0,0}, e_i) + (0.5 * e_i_to_e_j));
    Vector m_to_b_normalized = setMag(conn(m, b), 1.0);
    double l_e = len(e_i_to_e_j);
    double r_c = std::sqrt(r_b*r_b - 0.25*l_e*l_e);
    // ASSERTM(equals(len(conn(e_i, b)), r_b), "ball is placed incorrectly");
    // ASSERTM(equals(len(conn(e_j, b)), r_b), "ball is placed incorrectly");
    Vector n = setMag(cross(e_i_to_e_j, m_to_b_normalized), 1.0);
    ASSERTM(equals(n*m_to_b_normalized, 0.0), "normal is not perpendicular to plane(e_i, e_j, b)");
    VertexIndex newVertexIndex;
    bool foundNeighbour = false;
    double maxDotProduct = correspondingVertex.has_value() ? calcStartingScalarProduct(e_i, e_j, vertices[correspondingVertex.value()], b) : -1.0;
    DBOUT << "starting dot product: " << maxDotProduct << std::endl;
    Vertex newBallPosition;
    for (VertexIndex neighbour : neighbours) {
        if (correspondingVertex.has_value() && correspondingVertex.value() == neighbour) {
            continue;
        }
        std::vector<Vertex> intersections = intersectCircleSphere(m, r_c, setMag(e_i_to_e_j, 1.0), vertices[neighbour], r_b);
        for (Vertex i : intersections) {
            // ASSERTM(equals(len(conn(e_i, i)), r_b), "intersection is not ball radius away from edgepoint i");
            // ASSERTM(equals(len(conn(e_j, i)), r_b), "intersection is not ball radius away from edgepoint j");
            // ASSERTM(equals(len(conn(vertices[neighbour], i)), r_b), "intersection is not ball radius away from neighbour");
            Vector m_to_i_normalized = setMag(conn(m, i), 1.0);
            double p = m_to_b_normalized * m_to_i_normalized;
            // if (equals(i, b)) {
            //     // ball is not moving in this case
            //     ASSERTM(used(neighbour), "unused vertices should not be found before rolling (they should already have been added in a prior rolling step)");
            //     continue;
            // }
            if (n * m_to_i_normalized < 0.0) {
                // in this case ball is rotated more than 180°, we use the "extended" scalar product 
                p = -p - 2.0;
            }
            if (p > maxDotProduct) {
                maxDotProduct = p;
                newVertexIndex = neighbour;
                newBallPosition = i;
                foundNeighbour = true;
            }
        }
    }
    if (!foundNeighbour) {
        DBOUT << "ball touched no vertex, checked " << neighbours.size() << " neighbours" << std::endl;
        return std::nullopt;
    }
#ifdef DEBUG
    DBOUT << "found: " << newVertexIndex << std::endl;
    DBOUT << "rotating ball to position: (" << newBallPosition.x << "," << newBallPosition.y << "," << newBallPosition.z << "), or " << maxDotProduct << " in dot product" << std::endl;
    ASSERTM(equals(len(conn(e_i, newBallPosition)), r_b), "new ball is not ball radius away from edgepoint i");
    ASSERTM(equals(len(conn(e_j, newBallPosition)), r_b), "new ball is not ball radius away from edgepoint j");
    ASSERTM(equals(len(conn(newBallPosition, vertices[newVertexIndex])), r_b), "new ball is not ball radius away from new vertex");
    for (VertexIndex neighbour : neighbours) {
        if (newVertexIndex == neighbour) continue;
        // if (equals(std::abs(len(conn(newBallPosition, vertices[neighbour])) - ballRadius), 0.0)) {
        //     DBOUT << "[assertion fail] found vertex " << neighbour << " on ball surface with distance to ball: " << len(conn(newBallPosition, vertices[neighbour])) - ballRadius << std::endl;
        //     std::vector<Vertex> intersections = intersectCircleSphere(m, r_c, setMag(e_i_to_e_j, 1.0), vertices[neighbour], r_b);
        //     DBOUT << "[assertion fail] found following intersections for neighbour:" << std::endl;
        //     for (Vertex i : intersections) {
        //         DBOUT << "[assertion fail] (" << i.x << "," << i.y << "," << i.z << "), which is a dot product of: " << m_to_b_normalized*setMag(conn(m, i), 1.0) << std::endl;
        //         DBOUT << "[assertion fail] this intersection " << (equals(newBallPosition, i) ? "equals " : "does not equal ") << "the new ball position" << std::endl;
        //         DBOUT << "[assertion fail] this intersection and the ball position have a distance of " << len(conn(newBallPosition, i)) << std::endl;
        //     }
        // }
        // ASSERTM(!equals(std::abs(len(conn(newBallPosition, vertices[neighbour])) - ballRadius), 0.0), "vertices lay on the ball surface, those should have been captured before");
        // if (len(conn(newBallPosition, vertices[neighbour])) <= ballRadius) {
        //     DBOUT << "[assertion fail] found vertex " << neighbour << " in ball" << std::endl;
        //     DBOUT << "[assertion fail] " << neighbour << " has distance " << len(conn(newBallPosition, vertices[neighbour])) << " to ball center" << std::endl;
        // }
        // ASSERTM(len(conn(newBallPosition, vertices[neighbour])) > ballRadius, "vertices lay inside the ball");
    }
#endif
    return std::make_tuple(newVertexIndex, newBallPosition);
}

double BPA::calcStartingScalarProduct(const Vertex edgeI, const Vertex edgeJ, const Vertex correspondingVertex, const Vertex ballPosition) {
    Vector edgeIJ = conn(edgeI, edgeJ);
    Vector planeNormal = setMag(cross(edgeIJ, conn(edgeI, correspondingVertex)), 1.0);
    Vector awayFromEdgeIJ = setMag(cross(edgeIJ, planeNormal), 1.0);
    Vertex midPoint = toVertex(conn({0,0,0}, edgeI) + (0.5 * edgeIJ));
    Vector midPointToBallPos = setMag(conn(midPoint, ballPosition), 1.0);
    // could be optimized: is it possible to only calculate in scalar product?
    double radian = std::acos(std::clamp(midPointToBallPos*planeNormal, -1.0, 1.0));
    ASSERTM(radian >= 0.0, "ball is on wrong side");
    double alpha = midPointToBallPos*awayFromEdgeIJ < 0.0 ? M_PI_2-radian+0.0001f : M_PI_2+radian+0.0001f;
    double maxPossibleAngle = 2*M_PI - 2*alpha;
    maxPossibleAngle = maxPossibleAngle * 0.9f;
    if (maxPossibleAngle > M_PI) return -std::cos(maxPossibleAngle) - 2.0;
    return std::cos(maxPossibleAngle);
}

// std::vector<std::tuple<Edge, VertexIndex>> BPA::triangulatePlanar(const Edge edge, const std::vector<VertexIndex> &vertexIndicees) {
//     // ASSERT planar vertices
//     ASSERT(vertexIndicees.size() > 0);
//     Vector JtoINormalized = setMag(conn(vertices[edge.j], vertices[edge.i]), 1.0);

//     auto sorting = [JtoINormalized, edge, this](const VertexIndex vertexIndex1, const VertexIndex vertexIndex2) {
//         Vector JtoVertex1Normalized = setMag(conn(vertices[edge.j], vertices[vertexIndex1]), 1.0);
//         Vector JtoVertex2Normalized = setMag(conn(vertices[edge.j], vertices[vertexIndex2]), 1.0);
//         return JtoINormalized * JtoVertex1Normalized > JtoINormalized * JtoVertex2Normalized;
//     };

//     std::vector<VertexIndex> sorted(vertexIndicees.begin(), vertexIndicees.end());
//     sorted.push_back(edge.i);
//     std::sort(sorted.begin(), sorted.end(), sorting);

//     std::vector<std::tuple<Edge, VertexIndex>> triangulated;
//     for (auto it = sorted.begin(); it != std::prev(sorted.end()); it++) {
//         Edge e = {*it, edge.j};
//         triangulated.push_back(std::make_tuple(e, *std::next(it)));
//     }
//     ASSERT(triangulated.size() + 1 == sorted.size());
//     return triangulated;
// }

std::vector<Vertex> BPA::intersectCircleSphere(const Vertex circleCenter, const double circleRadius, const Vector circleNormal, const Vertex sphereCenter, const double sphereRadius) {
    ASSERT(!std::isnan(circleCenter.x) && !std::isnan(circleCenter.y) && !std::isnan(circleCenter.z));
    ASSERT(!std::isnan(sphereCenter.x) && !std::isnan(sphereCenter.y) && !std::isnan(sphereCenter.z));
    ASSERT(!std::isnan(circleNormal.x) && !std::isnan(circleNormal.y) && !std::isnan(circleNormal.z));
    ASSERT(circleRadius > 0.0);
    ASSERT(sphereRadius > 0.0);
    ASSERTM(equals(len(circleNormal), 1.0), "circle normal is not normalized");
    std::vector<Vertex> intersections;
    if (len(conn(circleCenter, sphereCenter)) > circleRadius + sphereRadius) {
        return intersections;
    }
    // first intersect circle plane with sphere -> we get a second circle in the same plane
    // intersect line with direction of circleNormal through sphereCenter with circle plane
    double scale = conn(sphereCenter, circleCenter) * circleNormal;
    if (std::abs(scale) > sphereRadius) return intersections;
    Vertex intersectionCircleCenter = toVertex(conn({0,0,0}, sphereCenter) + setMag(circleNormal, scale));
    double intersectionCircleRadius = std::sqrt(sphereRadius*sphereRadius - scale*scale);


    // second we intersect both circles
    // we use notation from http://paulbourke.net/geometry/circlesphere/

    Vertex P_0 = circleCenter;
    double r_0 = circleRadius;
    Vertex P_1 = intersectionCircleCenter;
    double r_1 = intersectionCircleRadius;

    Vector P_0_to_P_1 = conn(P_0, P_1);
    double d = len(P_0_to_P_1);
    if (d < std::abs(r_0 - r_1)) return intersections;
    if (d > r_0 + r_1) return intersections;
    double a = (r_0*r_0 - r_1*r_1 + d*d) / (2*d);
    Vertex P_2 = toVertex(conn({0,0,0}, P_0) + setMag(P_0_to_P_1, a));
    if (equals(r_0, std::abs(a))) {
        intersections.push_back(toVertex(conn({0,0,0}, P_2)));
        return intersections;
    }
    double h = std::sqrt(r_0*r_0 - a*a);
    Vector dir_h = cross(circleNormal, P_0_to_P_1);
    Vertex i1 = toVertex(conn({0,0,0}, P_2) + setMag(dir_h, h));
    Vertex i2 = toVertex(conn({0,0,0}, P_2) + setMag(dir_h, -h));
    ASSERTM(equals(len(conn(i1, circleCenter)), circleRadius), "intersection is not circle radius away from circle center");
    ASSERTM(equals(len(conn(i1, sphereCenter)), sphereRadius), "intersection is not sphere radius away from sphere center");
    intersections.push_back(i1);
    intersections.push_back(i2);
    return intersections;
}