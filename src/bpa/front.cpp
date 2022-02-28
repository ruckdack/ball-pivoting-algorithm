#include "front.h"

#include "bpa.h"
#include "../helpers/helpers.h"
#include "../trace.h"

#include <cassert>
#include <iostream>

std::optional<Front::ActiveEdge> Front::getActiveEdge() {
    ASSERT(integrity());
    for (Loop loop : front) {
        for (Edge edge : loop) {
            if (!boundaryContains(edge)) {
                ActiveEdge activeEdge = {};
                activeEdge.edge = edge;
                activeEdge.ballPosition = ballPositions[toString(edge)];
                activeEdge.correspVertexIndex = correspVertexIndexMap[toString(edge)];
                if (additionalCorrespVertexIndiceesMap.find(toString(edge)) != additionalCorrespVertexIndiceesMap.end()) {
                    activeEdge.additionalCorrespVertexIndicees = additionalCorrespVertexIndiceesMap[toString(edge)];
                } else {
                    activeEdge.additionalCorrespVertexIndicees = std::vector<VertexIndex>();
                }
                return activeEdge;
            }
        }
    }
    DBOUT << "no active edges" << std::endl;
    return std::nullopt;
}

// void insertCorrespondingVertex(Edge edge, VertexIndex vertexIndex) {
//     std::string key = toString(edge);
//     if (correspondingVertices.find(key) == correspondingVertices.end()) {
//         correspondingVertices[key] = std::vector<VertexIndex>{vertexIndex};
//     } else {
//         correspondingVertices[key].push_back(vertexIndex);
//     }
// }

// some ballPosition and correspondingVertices entries could be deleted here
void Front::join(const Edge edge, const VertexIndex vertexIndex, const Vertex ballPosition, const std::vector<VertexIndex> additionalCorrespVertexIndicees) {
    ASSERT(integrity());
    for (Loop &loop : front) {
        auto edgeIter = std::find(loop.begin(), loop.end(), edge);
        if (edgeIter != loop.end()) {
            Edge edge1 = {edge.i, vertexIndex};
            Edge edge2 = {vertexIndex, edge.j};
            loop.insert(edgeIter, edge1);
            loop.insert(edgeIter, edge2);
            ballPositions[toString(edge1)] = ballPosition;
            ballPositions[toString(edge2)] = ballPosition;
            correspVertexIndexMap[toString(edge1)] = edge.j;
            correspVertexIndexMap[toString(edge2)] = edge.i;
            if (additionalCorrespVertexIndicees.size() > 0) {
                additionalCorrespVertexIndiceesMap[toString(edge1)] = additionalCorrespVertexIndicees;
                additionalCorrespVertexIndiceesMap[toString(edge2)] = additionalCorrespVertexIndicees;
            }
            loop.erase(edgeIter);
            ASSERT(integrity());
            ASSERT(!contains(edge));
            return;
        }
    }
    ASSERT(false);
}

void Front::glue(Edge edge1, Edge edge2) {
    ASSERT(edge1.i == edge2.j && edge1.j == edge2.i);
    std::list<Loop>::iterator loop1Iterator;
    std::list<Edge>::iterator edge1Iterator;
    std::list<Loop>::iterator loop2Iterator;
    std::list<Edge>::iterator edge2Iterator;
    for (auto loopIter = front.begin(); loopIter != front.end(); loopIter++) {
        Loop &loop = *loopIter;
        auto it1 = std::find(loop.begin(), loop.end(), edge1);
        auto it2 = std::find(loop.begin(), loop.end(), edge2);
        if (it1 != loop.end() && it2 != loop.end()) {
            // on same loop
            if (loop.size() <= 2) {
                // trivial loop
                DBOUT << "trivial loop: (" << edge1.i << "," << edge1.j << ")" << std::endl;
                front.erase(loopIter);
                ASSERT(!contains(edge1) && !contains(edge2));
                ASSERT(integrity());
                return;

            }
            if (areConsecutive(loop, it1, it2)) {
                // consecutive on same loop
                DBOUT << "consec on same loop" << std::endl;
                loop.erase(it1);
                loop.erase(it2);
                ASSERT(!contains(edge1) && !contains(edge2));
                ASSERT(integrity());
                return;
            }
            // not consecutive on same loop
            DBOUT << "not consec on same loop" << std::endl;
            Loop secondLoop;
            if (std::distance(loop.begin(), it1) > std::distance(loop.begin(), it2)) {
                auto temp = it2;
                it2 = it1;
                it1 = temp;
            }
            ASSERT(std::distance(loop.begin(), it1) <= std::distance(loop.begin(), it2));
            secondLoop.splice(secondLoop.begin(), loop, it1, it2);
            secondLoop.erase(it1);
            loop.erase(it2);
            front.push_back(secondLoop);
            ASSERT(!contains(edge1) && !contains(edge2));
            ASSERT(integrity());
            return;
        }
        if (it1 != loop.end()) {
            loop1Iterator = loopIter;
            edge1Iterator = it1;
        }
        if (it2 != loop.end()) {
            loop2Iterator = loopIter;
            edge2Iterator = it2;
        }
    }
    // edges are not on same loop
    DBOUT << "not on same loop" << std::endl;
    loop1Iterator->splice(edge1Iterator, *loop2Iterator, std::next(edge2Iterator), loop2Iterator->end());
    loop1Iterator->splice(edge1Iterator, *loop2Iterator, loop2Iterator->begin(), std::next(edge2Iterator));
    loop1Iterator->erase(edge1Iterator);
    loop1Iterator->erase(edge2Iterator);
    front.erase(loop2Iterator);
    ASSERT(!contains(edge1) && !contains(edge2));
    ASSERT(integrity());
}

void Front::insertSeedTriangle(Edge edge1, Edge edge2, Edge edge3, Vertex ballPosition) {
    ASSERT(edge1.j == edge2.i);
    ASSERT(edge2.j == edge3.i);
    ASSERT(edge3.j == edge1.i);
    Loop loop = { edge1, edge2, edge3 };
    front.push_back(loop);
    ballPositions[toString(edge1)] = ballPosition;
    ballPositions[toString(edge2)] = ballPosition;
    ballPositions[toString(edge3)] = ballPosition;
    correspVertexIndexMap[toString(edge1)] = edge2.j;
    correspVertexIndexMap[toString(edge2)] = edge3.j;
    correspVertexIndexMap[toString(edge3)] = edge1.j;
    ASSERT(integrity());
}

bool Front::contains(VertexIndex vertexIndex) {
    for (Loop loop : front) {
        for (Edge e : loop) {
            if (e.i == vertexIndex || e.j == vertexIndex) return true;
        }
    }
    return false;
}

bool Front::contains(Edge edge) {
    for (Loop loop : front) {
        for (Edge e : loop) {
            if (e == edge) return true;
        }
    }
    return false;
}

bool Front::boundaryContains(Edge edge) {
    for (Edge boundaryEdge : boundary) {
        if (edge == boundaryEdge) {
            return true;
        }
    }
    return false;
}

void Front::markAsBoundary(Edge edge) {
    ASSERT(!boundaryContains(edge));
    boundary.push_back(edge);
}

bool Front::areConsecutive(Loop &loop, Loop::iterator it1, Loop::iterator it2) {
    ASSERT(loop.size() >= 2);
    if (it1 == std::prev(loop.end())) {
        return it2 == std::prev(it1) || it2 == loop.begin();
    }
    if (it2 == std::prev(loop.end())) {
        return it1 == std::prev(it2) || it1 == loop.begin();
    }
    return std::next(it1) == it2 || std::next(it2) == it1;
}

bool Front::loopIntegrity(Loop &loop) {
    for (auto it = std::next(loop.begin()); it != loop.end(); it++) {
        if (it->i != std::prev(it)->j) return false;
    }
    return true;
}

// checks for loop order and double edges
bool Front::integrity() {
    for (Loop &loop : front) {
        if (!loopIntegrity(loop)) {
            DBOUT << "loop order corrupted" << std::endl;
            return false;
        }
    }
    for (Loop &loop: front) {
        for (Edge &edge: loop) {
            // check if edge exists on other loop
            for (auto it = front.begin(); it != front.end(); it++) {
                if (*it != loop) {
                    for (Edge &e : *it) {
                        if (edge == e) {
                            DBOUT << "front contains double edge" << std::endl;
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool Front::nonemptyBoundary() {
    return boundary.size() != 0;
}