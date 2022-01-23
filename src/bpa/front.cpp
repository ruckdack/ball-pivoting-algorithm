#include "bpa_details.h"
#include "../helpers/helpers.h"

using namespace BPA;

#include <cassert>
#include <iostream>

Front::Front(Vertices &v) : vertices(v) {}

std::optional<Edge> Front::getActiveEdge() {
    for (Loop loop : front) {
        for (Edge edge : loop) {
            if (!boundaryContains(edge)) {
                return edge;
            }
        }
    }
    std::cout << "no active edges" << std::endl;
    return std::nullopt;
}

void Front::join(Edge edge, VertexIndex vertexIndex) {
    for (Loop &loop : front) {
        auto edgeIter = std::find(loop.begin(), loop.end(), edge);
        if (edgeIter != loop.end()) {
            loop.insert(edgeIter, {edge.i, vertexIndex});
            loop.insert(edgeIter, {vertexIndex, edge.j});
            loop.erase(edgeIter);
            assert(loopIntegrity(loop));
            return;
        }
    }
    assert(false);
}

void Front::glue(Edge edge1, Edge edge2) {
    assert(edge1.i == edge2.j && edge1.j == edge2.i);
    // std::cout << "glueing" << std::endl;
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
                std::cout << "trivial loop" << std::endl;
                front.erase(loopIter);
                return;

            }
            if (areConsecutive(loop, it1, it2)) {
                // consecutive on same loop
                std::cout << "consec on same loop" << std::endl;
                loop.erase(it1);
                loop.erase(it2);
                assert(loopIntegrity(loop));
                return;
            }
            // not consecutive on same loop
            std::cout << "not consec on same loop" << std::endl;
            Loop secondLoop;
            if (std::distance(loop.begin(), it1) <= std::distance(loop.begin(), it2)) {
                auto temp = it2;
                it2 = it1;
                it1 = temp;
            }
            assert(std::distance(loop.begin(), it1) <= std::distance(loop.begin(), it2));
            secondLoop.splice(secondLoop.begin(), loop, it1, it2);
            secondLoop.erase(it1);
            loop.erase(it2);
            assert(loopIntegrity(loop));
            assert(loopIntegrity(secondLoop));
            front.push_back(secondLoop);
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
    std::cout << "not on same loop" << std::endl;
    loop1Iterator->splice(edge1Iterator, *loop2Iterator, std::next(edge2Iterator), loop2Iterator->end());
    loop1Iterator->splice(edge1Iterator, *loop2Iterator, loop2Iterator->begin(), std::next(edge2Iterator));
    loop1Iterator->erase(edge1Iterator);
    loop1Iterator->erase(edge2Iterator);
    front.erase(loop2Iterator);
    assert(loopIntegrity(*loop1Iterator));
}

void Front::insertSeedTriangle(Edge edge1, Edge edge2, Edge edge3) {
    Loop loop = { edge1, edge2, edge3 };
    front.push_back(loop);
    assert(loopIntegrity(loop));
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
    assert(!boundaryContains(edge));
    boundary.push_back(edge);
}

bool Front::areConsecutive(Loop &loop, Loop::iterator it1, Loop::iterator it2) {
    assert(loop.size() >= 2);
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