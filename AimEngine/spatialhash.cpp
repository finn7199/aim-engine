#include "SpatialHash.h"
#include <algorithm>

SpatialHash::SpatialHash(float cellSize) : cellSize(cellSize) {}

uint64_t SpatialHash::hash(const glm::vec3& pos) const {
    int x = static_cast<int>(std::floor(pos.x / cellSize));
    int y = static_cast<int>(std::floor(pos.y / cellSize));
    int z = static_cast<int>(std::floor(pos.z / cellSize));

    // Simple hash combining x, y, z
    return ((uint64_t)(x & 0xFFFF) << 32) | ((uint64_t)(y & 0xFFFF) << 16) | (z & 0xFFFF);
}

void SpatialHash::clear() {
    grid.clear();
}

void SpatialHash::insertTriangle(uint32_t triIndex, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
    // Find bounding box of triangle
    glm::vec3 min = glm::min(glm::min(v1, v2), v3);
    glm::vec3 max = glm::max(glm::max(v1, v2), v3);

    // Expand slightly for safety margin
    min -= glm::vec3(cellSize * 0.5f);
    max += glm::vec3(cellSize * 0.5f);

    // Convert to grid coordinates
    glm::ivec3 minCell = glm::ivec3(std::floor(min.x / cellSize),
        std::floor(min.y / cellSize),
        std::floor(min.z / cellSize));
    glm::ivec3 maxCell = glm::ivec3(std::floor(max.x / cellSize),
        std::floor(max.y / cellSize),
        std::floor(max.z / cellSize));

    // Insert into all overlapping cells
    for (int x = minCell.x; x <= maxCell.x; ++x) {
        for (int y = minCell.y; y <= maxCell.y; ++y) {
            for (int z = minCell.z; z <= maxCell.z; ++z) {
                uint64_t cellHash = ((uint64_t)(x & 0xFFFF) << 32) |
                    ((uint64_t)(y & 0xFFFF) << 16) |
                    (z & 0xFFFF);
                grid[cellHash].push_back(triIndex);
            }
        }
    }
}

std::vector<uint32_t> SpatialHash::getNearbyTriangles(const glm::vec3& pos) const {
    std::vector<uint32_t> result;

    // Check 3x3x3 neighborhood around the position
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
                glm::vec3 neighborPos = pos + glm::vec3(dx * cellSize, dy * cellSize, dz * cellSize);
                uint64_t cellHash = hash(neighborPos);

                auto it = grid.find(cellHash);
                if (it != grid.end()) {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }
    }

    // Remove duplicates (same triangle in multiple cells)
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}