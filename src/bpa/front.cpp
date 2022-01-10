#include "bpa_details.h"
#include "../helpers/helpers.h"

using namespace BPA;

Front::Front(Vertices &v) : vertices(v) {}

std::optional<Edge> Front::getActiveEdge() {
    for (Loop loop : front) {
        for (Edge edge : loop) {
            for (Edge boundaryEdge : boundary) {
                if (edge != boundaryEdge) return edge;
            }
        }
    }
    return std::nullopt;
}

void Front::join(Edge edge, VertexIndex vertexIndex) {
    for (Loop loop : front) {
        if (Helpers::contains<Edge>(loop, edge)) {   
            loop.remove(edge);
            // TODO loop order is not yet maintained here
            loop.push_back({edge.i, vertexIndex});
            loop.push_back({vertexIndex, edge.j});
            return;
        }
    }
}

void Front::glue(Edge edge1, Edge edge2) {
    std::list<Loop>::iterator loop1Iterator;
    std::list<Edge>::iterator edge1Iterator;
    std::list<Loop>::iterator loop2Iterator;
    std::list<Edge>::iterator edge2Iterator;
    for (auto loopIter = front.begin(); loopIter != front.end(); loopIter++) {
        Loop loop = *loopIter;
        auto it1 = std::find(loop.begin(), loop.end(), edge1);
        auto it2 = std::find(loop.begin(), loop.end(), edge2);
        if (it1 != loop.end() && it2 != loop.end()) {
            // on same loop
            if (loop.size() <= 2) {
                // trivial loop
                front.erase(loopIter);
                return;

            } if (areConsecutive(loop, it1, it2)) {
                // consecutive on same loop
                loop.erase(edge1Iterator);
                loop.erase(it2);
                return;
            }
            // not consecutive on same loop
            Loop secondLoop;
            loop.splice(secondLoop.begin(), loop, it1, it2);
            secondLoop.erase(it1);
            loop.erase(it2);
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
    loop1Iterator->splice(edge1Iterator, *loop2Iterator, loop2Iterator->begin(), edge2Iterator);
    loop1Iterator->splice(edge1Iterator, *loop2Iterator, edge2Iterator, loop2Iterator->end());
    loop1Iterator->erase(edge1Iterator);
    loop1Iterator->erase(edge2Iterator);
    front.erase(loop2Iterator);
}

void Front::insertSeedTriangle(Edge edge1, Edge edge2, Edge edge3) {
    Loop loop = { edge1, edge2, edge2 };
    front.push_back(loop);
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

void Front::markAsBoundary(Edge edge) {
    boundary.push_back(edge);   
}

bool Front::areConsecutive(Loop &loop, Loop::iterator edge1Iterator, Loop::iterator edge2Iterator) {
    if (edge1Iterator == loop.end()) return std::prev(edge1Iterator) == edge2Iterator;
    if (edge2Iterator == loop.end()) return std::prev(edge2Iterator) == edge1Iterator;
    return std::next(edge1Iterator) == edge2Iterator
        || std::next(edge2Iterator) == edge1Iterator;
}