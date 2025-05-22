#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

class Target {
public:
    glm::vec3 position;
    float radius;
    bool hit;

    Target(float minX, float maxX, float minY, float maxY, float z, float radius, const std::vector<Target*>& existingTargets)
        : radius(radius), hit(false) {
        static bool seeded = false;
        if (!seeded) {
            std::srand(static_cast<unsigned>(std::time(nullptr)));
            seeded = true;
        }

        // Try up to 100 times to find a non-overlapping position
        for (int attempt = 0; attempt < 100; attempt++) {
            position.x = minX + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (maxX - minX)));
            position.y = minY + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (maxY - minY)));
            position.z = z;

            bool validPosition = true;
            for (const Target* other : existingTargets) {
                if (other != this) {
                    float distance = glm::distance(position, other->position);
                    if (distance < (radius + other->radius) * 1.2f) { // 1.2f for some spacing
                        validPosition = false;
                        break;
                    }
                }
            }

            if (validPosition) break;
        }
    }

    bool CheckRayIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) {
        // Sphere-ray intersection test
        glm::vec3 oc = rayOrigin - position;
        float a = glm::dot(rayDirection, rayDirection);
        float b = 2.0f * glm::dot(oc, rayDirection);
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        return discriminant > 0;
    }

    void Reset(float minX, float maxX, float minY, float maxY, float z, const std::vector<Target*>& existingTargets) {
        // Same positioning logic as constructor
        for (int attempt = 0; attempt < 100; attempt++) {
            position.x = minX + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (maxX - minX)));
            position.y = minY + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (maxY - minY)));
            position.z = z;

            bool validPosition = true;
            for (const Target* other : existingTargets) {
                if (other != this) {
                    float distance = glm::distance(position, other->position);
                    if (distance < (radius + other->radius) * 1.2f) {
                        validPosition = false;
                        break;
                    }
                }
            }

            if (validPosition) break;
        }
        hit = false;
    }
};