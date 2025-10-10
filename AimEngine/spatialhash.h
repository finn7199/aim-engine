#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <cstdint>

class SpatialHash {
public:
    SpatialHash(float cellSize = 0.2f);
    void clear();
    void insertTriangle(uint32_t triIndex, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
    std::vector<uint32_t> getNearbyTriangles(const glm::vec3& pos) const;
    void setCellSize(float size) { cellSize = size; }

private:
    uint64_t hash(const glm::vec3& pos) const;

    float cellSize;
    std::unordered_map<uint64_t, std::vector<uint32_t>> grid;
};