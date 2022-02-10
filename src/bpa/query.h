#include "primitives.h"

#include <tuple>
#include <map>

class Query {
    private:
        const std::vector<int64_t> OFFSETS;
        const Vertices &vertices;
        const float voxelSize;
        std::map<std::tuple<int64_t, int64_t, int64_t>, std::vector<VertexIndex>> grid;
    public:
        Query(const Vertices &vertices, const float voxelSize);
        std::vector<VertexIndex> getNeighbourhood(VertexIndex vertexIndex);
        std::vector<VertexIndex> getNeighbourhood(Edge edge);
};