#include "primitives.h"

bool operator==(const Edge edge1, const Edge edge2) {
    return edge1.i == edge2.i && edge1.j == edge2.j;
}

bool operator!=(const Edge edge1, const Edge edge2) {
    return !(edge1 == edge2);
}

bool operator==(const Triangle triangle1, const Triangle triangle2) {
    return triangle1.i == triangle2.i && triangle1.j == triangle2.j && triangle1.k == triangle2.k;
}

bool operator!=(const Triangle triangle1, const Triangle triangle2) {
    return !(triangle1 == triangle2);
}