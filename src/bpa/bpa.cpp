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

BPA::BPA(const Points &p, const double ballRad)
: vertices(p.vertices)
, normals(p.normals)
, ballRadius(ballRad)
, query(vertices, 2*ballRadius)
, done(false) {
    ASSERT(p.size == vertices.size());
    ASSERT(p.size == normals.size());
}

BPA::Result BPA::run() {
    size_t counter = 1;
    while (!done) {
        step();
        // only for debugging
        #ifdef DEBUG
        if (true) {
            std::string path = "../output/debug/debug_" + std::to_string(counter) + ".obj";
            Points points = {};
            points.vertices = vertices;
            points.normals = normals;
            points.size = vertices.size();
            IO::writeMesh(path, points, faces);
        }
        counter++;
        DBOUT << "count: " << counter << std::endl;
        #endif
    }
    Result result;
    result.triangles = faces;
    result.boundaryExists = front.nonemptyBoundary();
    result.numOfUsedVertices = usedVertices.size();
    return result;
}

void BPA::insertSeedTriangle(BPA::PivotResultStep pivotResultStep) {
    Triangle seedTriangle = { pivotResultStep.edge.j, pivotResultStep.edge.i, pivotResultStep.vertex};
    Vertex newBallPosition = pivotResultStep.ballPosition;
    faces.push_back(seedTriangle);
    DBOUT << "inital triangle: " << std::endl;
    printFaceIndicees(seedTriangle);
    VertexIndex i = seedTriangle.i;
    VertexIndex j = seedTriangle.j;
    VertexIndex k = seedTriangle.k;
    usedVertices.push_back(i);
    usedVertices.push_back(j);
    usedVertices.push_back(k);
    front.insertSeedTriangle({i, j}, {j, k}, {k, i}, newBallPosition);
}

bool BPA::insertPivotResultStep(BPA::PivotResultStep pivotResultStep) {
    Edge edge = pivotResultStep.edge;
    VertexIndex vertexIndex = pivotResultStep.vertex;
    Vertex newBallPosition = pivotResultStep.ballPosition;
    std::vector<VertexIndex> additionalCorrespVertexIndicees = pivotResultStep.additionalCorrespVertexIndicees;
    if (!used(vertexIndex) || front.contains(vertexIndex)) {
        Triangle newFace = {edge.i, vertexIndex, edge.j};
        for (Triangle face : faces) {
            ASSERTM(face != newFace, "cannot add already reconstructed face again");
        }
        faces.push_back(newFace);
        usedVertices.push_back(vertexIndex);
        front.join(edge, vertexIndex, newBallPosition, additionalCorrespVertexIndicees);
        DBOUT << "new triangle: " << std::endl;
        printFaceIndicees(newFace);
        ASSERT(!front.contains(edge));
        if (front.contains({vertexIndex, edge.i})) {
            front.glue({edge.i, vertexIndex}, {vertexIndex, edge.i});
        }
        if (front.contains({edge.j, vertexIndex})) {
            front.glue({vertexIndex, edge.j}, {edge.j, vertexIndex});
        }
        return true;
    }
    return false;
}

void BPA::step() {
    std::optional<Edge> boundaryEdge;
    if (faces.size() == 0) {
        if (PivotResult pivotResult = findSeedTriangle(); pivotResult.vertices.size() != 0) {
            std::vector<PivotResultStep> serialized = serializePivotResult(pivotResult);
            insertSeedTriangle(serialized[0]);
            for (size_t i = 1; i < serialized.size(); i++) {
                if (!insertPivotResultStep(serialized[i])) {
                    boundaryEdge = serialized[i].edge;
                }
            }
        } else {
            done = true;
            INFOUT << "Found no seed triangle." << std::endl;
        }
    } else {
        if (std::optional<Front::ActiveEdge> optionalActiveEdge = front.getActiveEdge()) {
            Edge edge = optionalActiveEdge.value().edge;
            Vertex ballPosition = optionalActiveEdge.value().ballPosition;
            VertexIndex correspVertexIndex = optionalActiveEdge.value().correspVertexIndex;
            std::vector<VertexIndex> additionalCorrespVertexIndicees = optionalActiveEdge.value().additionalCorrespVertexIndicees;
            // TODO corresponding Vertices verarzten
            PivotResult pivotResult = ballPivot(edge, ballPosition, correspVertexIndex, additionalCorrespVertexIndicees);
            if (pivotResult.vertices.size() == 0) {
                boundaryEdge = edge;
            } else {
                std::vector<PivotResultStep> serialized = serializePivotResult(pivotResult);
                for (PivotResultStep &s : serialized) {
                    if (!insertPivotResultStep(s)) {
                        boundaryEdge = s.edge;
                    }
                }
            }
        } else {
            done = true;
        }
    }
    if (boundaryEdge) {
        DBOUT << "marking as boundary" << std::endl;
        front.markAsBoundary(boundaryEdge.value());
    }
}

BPA::PivotResult BPA::findSeedTriangle() {
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
        PivotResult pivotResult = {};
        pivotResult.vertices = std::vector<VertexIndex>();
        return pivotResult;
    }
    ASSERTM(equals(len(conn(vertices[minVertex], ballPosition)), ballRadius), "inital ball is placed incorrectly");
    ASSERTM(equals(len(conn(vertices[rollableNeighbour], ballPosition)), ballRadius), "inital ball is placed incorrectly");
    Edge firstEdge = {minVertex, rollableNeighbour};
    // try rolling the ball in one direction
    PivotResult pivotResult = ballPivot(firstEdge, ballPosition, std::nullopt, std::vector<VertexIndex>());
    if (pivotResult.vertices.size() == 0) {
        // try rolling into other direction
        firstEdge = {firstEdge.j, firstEdge.i};
        PivotResult pivotResult = ballPivot(firstEdge, ballPosition, std::nullopt, std::vector<VertexIndex>());
        if (pivotResult.vertices.size() == 0) {
            DBOUT << "inital ball rolling touched no vertex" << std::endl;
            return pivotResult;
        }
        return pivotResult;
    }
    return pivotResult;
}

BPA::PivotResult BPA::ballPivot(const Edge edge, const Vertex ballPosition, const std::optional<VertexIndex> correspVertexIndex, const std::vector<VertexIndex> additionalCorrespVertexIndicees) {

    std::vector<VertexIndex> neighbours = query.getNeighbourhood(edge);
    Vertex e_i = vertices[edge.i];
    Vertex e_j = vertices[edge.j];
    DBOUT << std::endl;
    DBOUT << "rolling on: (" << edge.i << "," << edge.j << ")" << std::endl;
    DBOUT << "in coords: (" << e_i.x << "," << e_i.y << "," << e_i.z << ") and (" << e_j.x << "," << e_j.y << "," << e_j.z << ")" << std::endl;
    DBOUT << "ball is at: (" << ballPosition.x << "," << ballPosition.y << "," << ballPosition.z << ")" << std::endl;
    Vertex b = ballPosition;
    double r_b = ballRadius;

    Vector e_i_to_e_j = conn(e_i, e_j); // circle plane normal, not normalized
    Vertex m = toVertex(conn({0,0,0}, e_i) + (0.5 * e_i_to_e_j));
    Vector m_to_b = conn(m, b);
    double l_e = len(e_i_to_e_j);
    double r_c = std::sqrt(r_b*r_b - 0.25*l_e*l_e);
    ASSERTM(equals(len(m_to_b), r_c), "m_to_b is not circle radius away from edge midpoint");
    double r_c_sq = r_c*r_c;
    ASSERTM(equals(len(conn(e_i, b)), r_b), "ball is placed incorrectly");
    ASSERTM(equals(len(conn(e_j, b)), r_b), "ball is placed incorrectly");
    Vector n = setMag(cross(e_i_to_e_j, m_to_b), 1.0);
    ASSERTM(equals(n*m_to_b, 0.0), "normal is not perpendicular to plane(e_i, e_j, b)");
    VertexIndex newVertexIndex;
    std::optional<VertexIndex> alternativeVertexIndex = std::nullopt;
    bool foundNeighbour = false;
    double maxDotProduct = correspVertexIndex.has_value() ? calcStartingScalarProduct(e_i, e_j, vertices[correspVertexIndex.value()], b) : -1.0;
    maxDotProduct *= r_c_sq;
    DBOUT << "starting dot product: " << maxDotProduct << ", lays in [-3R², R²]: [" << -3*r_c_sq << ", " << r_c_sq << "]" << std::endl;
    Vertex newBallPosition;
    Circle circle = {};
    circle.center = m;
    circle.radius = r_c;
    circle.normal = setMag(e_i_to_e_j, 1.0);
    for (VertexIndex neighbour : neighbours) {
        if (correspVertexIndex && correspVertexIndex.value() == neighbour) {
            continue;
        }
        if (std::find(additionalCorrespVertexIndicees.begin(), additionalCorrespVertexIndicees.end(), neighbour) != additionalCorrespVertexIndicees.end()) {
            continue;
        }
        Sphere sphere = {};
        sphere.center = vertices[neighbour];
        sphere.radius = r_b;
        std::vector<Vertex> intersections = intersectCircleSphere(circle, sphere);
        if (neighbour == 162 && edge.i == 160 && edge.j == 161) {
            DBOUT << "test" << std::endl;
        }
        for (Vertex i : intersections) {
            ASSERTM(equals(len(conn(e_i, i)), r_b), "intersection is not ball radius away from edgepoint i");
            ASSERTM(equals(len(conn(e_j, i)), r_b), "intersection is not ball radius away from edgepoint j");
            ASSERTM(equals(len(conn(vertices[neighbour], i)), r_b), "intersection is not ball radius away from neighbour");
            Vector m_to_i = conn(m, i);
            ASSERTM(equals(len(m_to_i), r_c), "intersection is not circle radius away from edge midpoint");
            double p = m_to_b * m_to_i;
            if (n * m_to_i < 0.0) {
                if (p > r_c_sq - 0.000001) {
                    // ball can be rotated backwards by a small epsilon
                    p = 2 - p;
                } else {
                    // in this case ball is rotated more than 180°, we use the "extended" scalar product
                    p = -p - 2*r_c_sq;
                }
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
        PivotResult pivotResult = {};
        pivotResult.vertices = std::vector<VertexIndex>();
        return pivotResult;
    }
    if (alternativeVertexIndex) {
        // ASSERT(false);
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
            if (newVertexIndex == neighbour || alternativeVertexIndex && alternativeVertexIndex.value() == neighbour) continue;
            if (equals(std::abs(len(conn(newBallPosition, vertices[neighbour])) - ballRadius), 0.0)) {
                Sphere sphere = {};
                sphere.center = vertices[neighbour];
                sphere.radius = r_b;
                std::vector<Vertex> intersections = intersectCircleSphere(circle, sphere);
                for (Vertex i : intersections) {
                    if (same(newBallPosition, i)) {
                        DBOUT << "[assertion fail] found vertex " << neighbour << " on ball surface" << std::endl;
                        DBOUT << "[assertion fail] in coords: " << vertices[neighbour].x << "," << vertices[neighbour].y << "," << vertices[neighbour].z << ")" << std::endl;
                        DBOUT << "[assertion fail] intersection which equals new ball pos: (" << i.x << "," << i.y << "," << i.z << "), which is a dot product of: " << m_to_b*conn(m, i) << std::endl;
                    }
                    ASSERTM(!same(newBallPosition, i), "vertices lay on the ball surface");
                }
            }
            if (len(conn(newBallPosition, vertices[neighbour])) < ballRadius) {
                DBOUT << "[assertion fail] found vertex " << neighbour << " in ball" << std::endl;
                DBOUT << "[assertion fail] in coords: (" << vertices[neighbour].x << "," << vertices[neighbour].y << "," << vertices[neighbour].z << ")" << std::endl;
                DBOUT << "[assertion fail] " << neighbour << " has distance " << len(conn(newBallPosition, vertices[neighbour])) << " to ball center" << std::endl;
            }
            // ASSERTM(len(conn(newBallPosition, vertices[neighbour])) >= ballRadius, "vertices lay inside the ball");
        }
    #endif
    PivotResult pivotResult = {};
    pivotResult.edge = edge;
    pivotResult.vertices = {newVertexIndex};
    if (alternativeVertexIndex) {
        pivotResult.vertices.push_back(alternativeVertexIndex.value());
    }
    pivotResult.ballPosition = newBallPosition;
    return pivotResult;
}

double BPA::calcStartingScalarProduct(const Vertex edgeI, const Vertex edgeJ, const Vertex correspVertex, const Vertex ballPosition) {
    Vector edgeIJ = conn(edgeI, edgeJ);
    Vector planeNormal = setMag(cross(edgeIJ, conn(edgeI, correspVertex)), 1.0);
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


std::vector<Vertex> BPA::intersectCircleSphere(const Circle circle, const Sphere sphere) {
    ASSERT(!std::isnan(circle.center.x) && !std::isnan(circle.center.y) && !std::isnan(circle.center.z));
    ASSERT(!std::isnan(sphere.center.x) && !std::isnan(sphere.center.y) && !std::isnan(sphere.center.z));
    ASSERT(!std::isnan(circle.normal.x) && !std::isnan(circle.normal.y) && !std::isnan(circle.normal.z));
    ASSERT(circle.radius > 0.0);
    ASSERT(sphere.radius > 0.0);
    ASSERTM(equals(len(circle.normal), 1.0), "circle normal is not normalized");
    std::vector<Vertex> intersections;
    if (len(conn(circle.center, sphere.center)) > circle.radius + sphere.radius) {
        return intersections;
    }
    // first intersect circle plane with sphere -> we get a second circle in the same plane
    // intersect line with direction of circle.normal through sphere.center with circle plane
    double scale = conn(sphere.center, circle.center) * circle.normal;
    if (std::abs(scale) > sphere.radius) return intersections;
    Vertex intersectionCircleCenter = toVertex(conn({0,0,0}, sphere.center) + setMag(circle.normal, scale));
    double intersectionCircleRadius = std::sqrt(sphere.radius*sphere.radius - scale*scale);


    // second we intersect both circles
    // we use notation from http://paulbourke.net/geometry/circlesphere/

    Vertex P_0 = circle.center;
    double r_0 = circle.radius;
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
    Vector dir_h = cross(circle.normal, P_0_to_P_1);
    Vertex i1 = toVertex(conn({0,0,0}, P_2) + setMag(dir_h, h));
    Vertex i2 = toVertex(conn({0,0,0}, P_2) + setMag(dir_h, -h));
    ASSERTM(equals(len(conn(i1, circle.center)), circle.radius), "intersection is not circle radius away from circle center");
    ASSERTM(equals(len(conn(i1, sphere.center)), sphere.radius), "intersection is not sphere radius away from sphere center");
    intersections.push_back(i1);
    intersections.push_back(i2);
    return intersections;
}

std::optional<Circle> BPA::intersectSphereSphere(const Sphere sphere1, const Sphere sphere2) {
    Vertex P_0 = sphere1.center;
    double r_0 = sphere1.radius;
    Vertex P_1 = sphere2.center;
    double r_1 = sphere2.radius;

    Vector P_0_to_P_1 = conn(P_0, P_1);
    double d = len(P_0_to_P_1);
    if (d < std::abs(r_0 - r_1)) return std::nullopt;
    if (d > r_0 + r_1) return std::nullopt;
    double a = (r_0*r_0 - r_1*r_1 + d*d) / (2*d);
    Vertex P_2 = toVertex(conn({0,0,0}, P_0) + setMag(P_0_to_P_1, a));
    if (equals(r_0, std::abs(a))) {
        return std::nullopt;
    }
    double h = std::sqrt(r_0*r_0 - a*a);
    Circle intersections = {};
    intersections.center = toVertex(conn({0,0,0}, P_2));
    intersections.radius = h;
    intersections.normal = setMag(P_0_to_P_1, 1.0); 
    return intersections;
}

std::optional<Vertex> BPA::calcMinAlongXAxis(const Circle circle) {
    ASSERTM(equals(len(circle.normal), 1.0), "circle normal is not normalized");
    Vector negXAxis = {-1,0,0};
    if (equals(circle.normal * negXAxis, -1.0)) {
    //     // circle lays in y-z-plane, no min x
        return std::nullopt;
    }
    double angleToNegXAxis = std::acos(negXAxis*circle.normal);
    // cos(angle - 90deg) = sin(angle)
    double minXValue = -circle.radius * std::sin(angleToNegXAxis);
    Vector minXValueVector = conn({0,0,0}, circle.center) + Vector({minXValue, 0.0, 0.0});
    // project normal onto y-z-plane at minXValue
    Vector proj = {0.0, circle.normal.y, circle.normal.z};
    // intersect proj with circle plane
    // notation from https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
    // t = ((p_0 - l_0) * n) / (l * n)
    // p_0: circle.center
    // n: circle.normal
    // l_0: minXValueVector
    // l: proj
    double t = ((conn({0,0,0}, circle.center) - minXValueVector) * circle.normal) / (proj * circle.normal);
    Vertex intersection = toVertex(minXValueVector + t * proj);
    ASSERTM(equals(len(conn(intersection, circle.center)), circle.radius), "found intersection is not circle.radius away from circle.center => not on circle");
    return intersection;
}

std::vector<BPA::PivotResultStep> BPA::serializePivotResult(BPA::PivotResult pivotResult) {
    // assert planar vertices
    ASSERT(pivotResult.vertices.size() > 0);
    if (pivotResult.vertices.size() == 1) {
        PivotResultStep pivotResultStep;
        pivotResultStep.edge = pivotResult.edge;
        pivotResultStep.vertex = pivotResult.vertices[0];
        pivotResultStep.ballPosition = pivotResult.ballPosition;
        pivotResultStep.additionalCorrespVertexIndicees = std::vector<VertexIndex>();
        return std::vector<PivotResultStep>(1, pivotResultStep);
    }
    Vector JtoINormalized = setMag(conn(vertices[pivotResult.edge.j], vertices[pivotResult.edge.i]), 1.0);

    auto sorting = [JtoINormalized, pivotResult, this](const VertexIndex vertexIndex1, const VertexIndex vertexIndex2) {
        Vector JtoVertex1Normalized = setMag(conn(vertices[pivotResult.edge.j], vertices[vertexIndex1]), 1.0);
        Vector JtoVertex2Normalized = setMag(conn(vertices[pivotResult.edge.j], vertices[vertexIndex2]), 1.0);
        return JtoINormalized * JtoVertex1Normalized > JtoINormalized * JtoVertex2Normalized;
    };

    std::vector<VertexIndex> sorted(pivotResult.vertices.begin(), pivotResult.vertices.end());
    sorted.push_back(pivotResult.edge.i);
    std::sort(sorted.begin(), sorted.end(), sorting);

    std::vector<PivotResultStep> serialized;
    for (auto it = sorted.begin(); it != std::prev(sorted.end()); it++) {
        PivotResultStep pivotResultStep = {};
        pivotResultStep.edge = {*it, pivotResult.edge.j};
        pivotResultStep.vertex = *std::next(it);
        pivotResultStep.ballPosition = pivotResult.ballPosition;
        std::vector<VertexIndex> additionalCorrespVertexIndicees;
        additionalCorrespVertexIndicees.reserve(sorted.size() - 1);
        for (VertexIndex vertexIndex : sorted) {
            if (vertexIndex == pivotResultStep.vertex) continue;
            if (vertexIndex == pivotResultStep.edge.i) continue;
            additionalCorrespVertexIndicees.push_back(vertexIndex);
        }
        pivotResultStep.additionalCorrespVertexIndicees = additionalCorrespVertexIndicees;
        serialized.push_back(pivotResultStep);
    }
    ASSERT(serialized.size() + 1 == sorted.size());
    ASSERT(serialized.size() == pivotResult.vertices.size());
    return serialized;
}

bool BPA::used(VertexIndex vertexIndex) {
    return usedVertices.end() != std::find(usedVertices.begin(), usedVertices.end(), vertexIndex);
}

void BPA::printFaceIndicees(Triangle triangle) {
    DBOUT << "(" << triangle.i << "," << triangle.j << "," << triangle.k << ")"<< std::endl;
}








