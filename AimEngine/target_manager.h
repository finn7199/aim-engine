#pragma once
#include "target.h"
#include <vector>

class TargetManager {
public:
    std::vector<Target> targets;

    TargetManager(int count, float minX, float maxX, float minY, float maxY, float z, float radius) {
        std::vector<Target*> existingTargets;
        targets.reserve(count); // Reserve memory to avoid reallocations

        for (int i = 0; i < count; ++i) {
            // Create a new target, passing the list of already-created targets for collision checks
            targets.emplace_back(minX, maxX, minY, maxY, z, radius, existingTargets);
            // Add the new target's address to the list for the next iteration
            existingTargets.push_back(&targets.back());
        }
    }

    bool CheckHits(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) {
        bool hitAny = false;
        for (auto& target : targets) {
            if (!target.hit && target.CheckRayIntersection(rayOrigin, rayDirection)) {
                target.hit = true;
                hitAny = true;
            }
        }
        return hitAny;
    }

    void ResetHitTargets(float minX, float maxX, float minY, float maxY, float z) {
        std::vector<Target*> targetPtrs;
        for (auto& target : targets) {
            targetPtrs.push_back(&target);
        }

        for (auto& target : targets) {
            if (target.hit) {
                target.Reset(minX, maxX, minY, maxY, z, targetPtrs);
            }
        }
    }
};