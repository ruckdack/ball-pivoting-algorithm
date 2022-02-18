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

    Sphere minVertexSphere = {};
    minVertexSphere.center = vertices[minVertex];
    minVertexSphere.radius = ballRadius;
    bool foundNeighbour = false;
    VertexIndex rollableNeighbour;
    Vertex ballPosition;
    for (VertexIndex neighbour : neighbours) {
        Sphere neighbourSphere = {};
        neighbourSphere.center = vertices[neighbour];
        neighbourSphere.radius = ballRadius; 
        std::optional<Circle> intersection = intersectSphereSphere(minVertexSphere, neighbourSphere);
        if (!intersection) continue;
        std::optional<Vertex> minXIntersection = calcMinAlongXAxis(intersection.value());
        if (!minXIntersection) continue;
        if (!foundNeighbour) {
            foundNeighbour = true;
            rollableNeighbour = neighbour;
            ballPosition = minXIntersection.value();
            continue;
        }
        if (minXIntersection.value().x < ballPosition.x) {
            ballPosition = minXIntersection.value();
            rollableNeighbour = neighbour;
        }

    }
    if (!foundNeighbour) {
        DBOUT << "no neighbour found for initial vertex" << std::endl;
        return std::nullopt;
    }
    ASSERTM(equals(len(conn(vertices[minVertex], ballPosition)), ballRadius), "inital ball is placed incorrectly");
    ASSERTM(equals(len(conn(vertices[rollableNeighbour], ballPosition)), ballRadius), "inital ball is placed incorrectly");
    Edge firstEdge = {minVertex, rollableNeighbour};
    // try rolling the ball in one direction
    auto optionalVertexIndex = ballPivot(firstEdge, ballPosition, std::nullopt);
    if (!optionalVertexIndex) {
        // try rolling into other direction
        firstEdge = {firstEdge.j, firstEdge.i};
        optionalVertexIndex = ballPivot(firstEdge, ballPosition, std::nullopt);
        if (!optionalVertexIndex) {
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

    std::optional<VertexIndex> skipVertex = std::nullopt;
    if (correspondingVertex && ignoreForEdges.find(toString(edge) + "@" + std::to_string(correspondingVertex.value())) != ignoreForEdges.end()) {
        skipVertex = ignoreForEdges[toString(edge) + "@" + std::to_string(correspondingVertex.value())];
        DBOUT << "skipping: " << skipVertex.value() << std::endl;
    }

    std::vector<VertexIndex> neighbours = query.getNeighbourhood(edge);
    Vertex e_i = vertices[edge.i];
    Vertex e_j = vertices[edge.j];
    DBOUT << "rolling on: (" << edge.i << "," << edge.j << ")" << std::endl;
    DBOUT << "in coords: (" << e_i.x << "," << e_i.y << "," << e_i.z << ") and (" << e_j.x << "," << e_j.y << "," << e_j.z << ")" << std::endl;
    DBOUT << "ball is at: (" << ballPosition.x << "," << ballPosition.y << "," << ballPosition.z << ")" << std::endl;
    Vertex b = ballPosition;
    float r_b = ballRadius;

    Vector e_i_to_e_j = conn(e_i, e_j); // circle plane normal, not normalized
    Vertex m = toVertex(conn({0,0,0}, e_i) + (0.5 * e_i_to_e_j));
    Vector m_to_b = conn(m, b);
    float l_e = len(e_i_to_e_j);
    float r_c = std::sqrt(r_b*r_b - 0.25*l_e*l_e);
    float r_c_sq = r_c*r_c;
    ASSERTM(equals(len(conn(e_i, b)), r_b), "ball is placed incorrectly");
    ASSERTM(equals(len(conn(e_j, b)), r_b), "ball is placed incorrectly");
    Vector n = setMag(cross(e_i_to_e_j, m_to_b), 1.f);
    ASSERTM(equals(n*m_to_b, 0.f), "normal is not perpendicular to plane(e_i, e_j, b)");
    VertexIndex newVertexIndex;
    std::optional<VertexIndex> alternativeVertexIndex = std::nullopt;
    bool foundNeighbour = false;
    float maxDotProduct = correspondingVertex ? calcStartingScalarProduct(e_i, e_j, vertices[correspondingVertex.value()], b) : -1.f;
    maxDotProduct *= r_c_sq;
    DBOUT << "starting dot product: " << maxDotProduct << ", lays in [-3R², R²]: [" << -3*r_c_sq << ", " << r_c_sq << "]" << std::endl;
    Vertex newBallPosition;
    for (VertexIndex neighbour : neighbours) {
        if (correspondingVertex && correspondingVertex.value() == neighbour || skipVertex && skipVertex.value() == neighbour) {
            continue;
        }
        std::vector<Vertex> intersections = intersectCircleSphere(m, r_c, setMag(e_i_to_e_j, 1.f), vertices[neighbour], r_b);
        for (Vertex i : intersections) {
            ASSERTM(equals(len(conn(e_i, i)), r_b), "intersection is not ball radius away from edgepoint i");
            ASSERTM(equals(len(conn(e_j, i)), r_b), "intersection is not ball radius away from edgepoint j");
            ASSERTM(equals(len(conn(vertices[neighbour], i)), r_b), "intersection is not ball radius away from neighbour");
            Vector m_to_i = conn(m, i);
            float p = m_to_b * m_to_i;
            if (n * m_to_i < 0.f) {
                // in this case ball is rotated more than 180°, we use the "extended" scalar product 
                p = -p - 2*r_c_sq;
            }
            if (same(i, newBallPosition)) {
                alternativeVertexIndex = neighbour;
            }
            if (p > maxDotProduct) {
                maxDotProduct = p;
                newVertexIndex = neighbour;
                newBallPosition = i;
                foundNeighbour = true;
                alternativeVertexIndex = std::nullopt;
            }
        }
    }
    if (!foundNeighbour) {
        DBOUT << "ball touched no vertex, checked " << neighbours.size() << " neighbours" << std::endl;
        return std::nullopt;
    }
    if (alternativeVertexIndex) {
        ASSERT(false);
        // DBOUT << "rolled onto multiple points" << std::endl;
        // Vector e_i_to_new = conn(e_i, vertices[newVertexIndex]);
        // Vector e_i_to_alt = conn(e_i, vertices[alternativeVertexIndex.value()]);
        // if (e_i_to_new*e_i_to_new > e_i_to_alt*e_i_to_alt) {
        //     DBOUT << "swapping" << std::endl;
        //     auto tmp = newVertexIndex;
        //     newVertexIndex = alternativeVertexIndex.value();
        //     alternativeVertexIndex = tmp;
        // }
        // Edge iNew = {edge.i, newVertexIndex};
        // ignoreForEdges.insert(std::make_pair(toString(iNew) + "@" + std::to_string(edge.j), alternativeVertexIndex.value()));
        // Edge newAlt = {newVertexIndex, alternativeVertexIndex.value()};
        // ignoreForEdges.insert(std::make_pair(toString(newAlt) + "@" + std::to_string(edge.j), edge.i));
        // Edge altJ = {alternativeVertexIndex.value(), edge.j};
        // ignoreForEdges.insert(std::make_pair(toString(altJ) + "@" + std::to_string(newVertexIndex), edge.i));
    }
    #ifdef DEBUG // these are only assertions
        DBOUT << "found: " << newVertexIndex << std::endl;
        DBOUT << "in coords: " << vertices[newVertexIndex].x << "," << vertices[newVertexIndex].y << "," << vertices[newVertexIndex].z << ")" << std::endl;
        if (alternativeVertexIndex) DBOUT << "alt: " << alternativeVertexIndex.value() << std::endl;
        DBOUT << "rotating ball to position: (" << newBallPosition.x << "," << newBallPosition.y << "," << newBallPosition.z << "), or " << maxDotProduct << " in dot product" << std::endl;
        ASSERTM(equals(len(conn(e_i, newBallPosition)), r_b), "new ball is not ball radius away from edgepoint i");
        ASSERTM(equals(len(conn(e_j, newBallPosition)), r_b), "new ball is not ball radius away from edgepoint j");
        ASSERTM(equals(len(conn(newBallPosition, vertices[newVertexIndex])), r_b), "new ball is not ball radius away from new vertex");
        for (VertexIndex neighbour : neighbours) {
            if (newVertexIndex == neighbour || alternativeVertexIndex && alternativeVertexIndex.value() == neighbour || skipVertex && skipVertex.value() == neighbour) continue;
            if (equals(std::abs(len(conn(newBallPosition, vertices[neighbour])) - ballRadius), 0.f)) {
                DBOUT << "[assertion fail] found vertex " << neighbour << " on ball surface with distance to ball: " << len(conn(newBallPosition, vertices[neighbour])) - ballRadius << std::endl;
                std::vector<Vertex> intersections = intersectCircleSphere(m, r_c, setMag(e_i_to_e_j, 1.f), vertices[neighbour], r_b);
                DBOUT << "[assertion fail] found following intersections for neighbour:" << std::endl;
                for (Vertex i : intersections) {
                    DBOUT << "[assertion fail] (" << i.x << "," << i.y << "," << i.z << "), which is a dot product of: " << m_to_b*conn(m, i) << std::endl;
                    DBOUT << "[assertion fail] this intersection " << (same(newBallPosition, i) ? "equals " : "does not equal ") << "the new ball position" << std::endl;
                    DBOUT << "[assertion fail] this intersection and the ball position have a distance of " << len(conn(newBallPosition, i)) << std::endl;
                    ASSERTM(!same(newBallPosition, i), "vertices lay on the ball surface");
                }
            }
            if (len(conn(newBallPosition, vertices[neighbour])) < ballRadius) {
                DBOUT << "[assertion fail] found vertex " << neighbour << " in ball" << std::endl;
                DBOUT << "[assertion fail] in coords: (" << vertices[neighbour].x << "," << vertices[neighbour].y << "," << vertices[neighbour].z << ")" << std::endl;
                DBOUT << "[assertion fail] " << neighbour << " has distance " << len(conn(newBallPosition, vertices[neighbour])) << " to ball center" << std::endl;
            }
            ASSERTM(len(conn(newBallPosition, vertices[neighbour])) >= ballRadius, "vertices lay inside the ball");
        }
    #endif
    DBOUT << std::endl; 
    return std::make_tuple(newVertexIndex, newBallPosition);
}

float BPA::calcStartingScalarProduct(const Vertex edgeI, const Vertex edgeJ, const Vertex correspondingVertex, const Vertex ballPosition) {
    Vector edgeIJ = conn(edgeI, edgeJ);
    Vector planeNormal = setMag(cross(edgeIJ, conn(edgeI, correspondingVertex)), 1.f);
    Vector awayFromEdgeIJ = setMag(cross(edgeIJ, planeNormal), 1.f);
    Vertex midPoint = toVertex(conn({0,0,0}, edgeI) + (0.5 * edgeIJ));
    Vector midPointToBallPos = setMag(conn(midPoint, ballPosition), 1.f);
    // could be optimized: is it possible to only calculate in scalar product?
    float radian = std::acos(std::clamp(midPointToBallPos*planeNormal, -1.f, 1.f));
    ASSERTM(radian >= 0.f, "ball is on wrong side");
    float alpha = midPointToBallPos*awayFromEdgeIJ < 0.f ? M_PI_2-radian+0.0001f : M_PI_2+radian+0.0001f;
    float maxPossibleAngle = 2*M_PI - 2*alpha;
    maxPossibleAngle = maxPossibleAngle * 0.9f;
    if (maxPossibleAngle > M_PI) return -std::cos(maxPossibleAngle) - 2.f;
    return std::cos(maxPossibleAngle);
}


std::vector<Vertex> BPA::intersectCircleSphere(const Vertex circleCenter, const float circleRadius, const Vector circleNormal, const Vertex sphereCenter, const float sphereRadius) {
    ASSERT(!std::isnan(circleCenter.x) && !std::isnan(circleCenter.y) && !std::isnan(circleCenter.z));
    ASSERT(!std::isnan(sphereCenter.x) && !std::isnan(sphereCenter.y) && !std::isnan(sphereCenter.z));
    ASSERT(!std::isnan(circleNormal.x) && !std::isnan(circleNormal.y) && !std::isnan(circleNormal.z));
    ASSERT(circleRadius > 0.f);
    ASSERT(sphereRadius > 0.f);
    ASSERTM(equals(len(circleNormal), 1.f), "circle normal is not normalized");
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
    if (equals(r_0, std::abs(a))) {
        intersections.push_back(toVertex(conn({0,0,0}, P_2)));
        return intersections;
    }
    float h = std::sqrt(r_0*r_0 - a*a);
    Vector dir_h = cross(circleNormal, P_0_to_P_1);
    Vertex i1 = toVertex(conn({0,0,0}, P_2) + setMag(dir_h, h));
    Vertex i2 = toVertex(conn({0,0,0}, P_2) + setMag(dir_h, -h));
    ASSERTM(equals(len(conn(i1, circleCenter)), circleRadius), "intersection is not circle radius away from circle center");
    ASSERTM(equals(len(conn(i1, sphereCenter)), sphereRadius), "intersection is not sphere radius away from sphere center");
    intersections.push_back(i1);
    intersections.push_back(i2);
    return intersections;
}

std::optional<Circle> BPA::intersectSphereSphere(const Sphere sphere1, const Sphere sphere2) {
    Vertex P_0 = sphere1.center;
    float r_0 = sphere1.radius;
    Vertex P_1 = sphere2.center;
    float r_1 = sphere2.radius;

    Vector P_0_to_P_1 = conn(P_0, P_1);
    float d = len(P_0_to_P_1);
    if (d < std::abs(r_0 - r_1)) return std::nullopt;
    if (d > r_0 + r_1) return std::nullopt;
    float a = (r_0*r_0 - r_1*r_1 + d*d) / (2*d);
    Vertex P_2 = toVertex(conn({0,0,0}, P_0) + setMag(P_0_to_P_1, a));
    if (equals(r_0, std::abs(a))) {
        return std::nullopt;
    }
    float h = std::sqrt(r_0*r_0 - a*a);
    Circle intersections = {};
    intersections.center = toVertex(conn({0,0,0}, P_2));
    intersections.radius = h;
    intersections.normal = setMag(P_0_to_P_1, 1.f); 
    return intersections;
}

std::optional<Vertex> BPA::calcMinAlongXAxis(const Circle circle) {
    ASSERTM(equals(len(circle.normal), 1.f), "circle normal is not normalized");
    Vector negXAxis = {-1,0,0};
    if (equals(circle.normal.y, 0.f) && equals(circle.normal.z, 0.f)) {
        // circle lays in y-z-plane, no min x
        return std::nullopt;
    }
    float angleToNegXAxis = std::acos(negXAxis*circle.normal);
    // cos(angle - 90deg) = sin(angle)
    float minXValue = -circle.radius * std::sin(angleToNegXAxis);
    Vector minXValueVector = conn({0,0,0}, circle.center) + Vector({minXValue, 0.f, 0.f});
    // project normal onto y-z-plane at minXValue
    Vector proj = {0.f, circle.normal.y, circle.normal.z};
    // intersect proj with circle plane
    // notation from https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
    // p_0: circle.center
    // n: circle.normal
    // l_0: minXValueVector
    // l: proj
    float t = ((conn({0,0,0}, circle.center) - minXValueVector) * circle.normal) / (proj * circle.normal);
    Vertex intersection = toVertex(minXValueVector + t * proj);
    return intersection;
}