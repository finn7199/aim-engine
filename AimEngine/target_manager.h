#pragma once
#include "target.h"
#include <vector>

class TargetManager {
public:
    std::vector<Target> targets;

    TargetManager(int count, float minX, float maxX, float minY, float maxY, float z, float radius) {
        // First create all targets with empty existing list
        for (int i = 0; i < count; ++i) {
            targets.emplace_back(minX, maxX, minY, maxY, z, radius, std::vector<Target*>());
        }

        // Then reset them with proper collision checking
        std::vector<Target*> targetPtrs;
        for (auto& target : targets) {
            targetPtrs.push_back(&target);
        }
        for (auto& target : targets) {
            target.Reset(minX, maxX, minY, maxY, z, targetPtrs);
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