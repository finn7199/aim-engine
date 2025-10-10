#include "Cloth.h"
#include <limits>
#include <chrono>
#include <iostream>

Cloth::Cloth(float width, float height, int numX, int numY)
    : numParticlesX(numX), numParticlesY(numY), spatialHash(0.3f) {

    particles.resize(numX * numY);

    // create particles in a grid
    for (int y = 0; y < numY; ++y) {
        for (int x = 0; x < numX; ++x) {
            int i = y * numX + x;
            particles[i].position = glm::vec3(
                width * ((float)x / (numX - 1) - 0.5f),
                height * ((float)y / (numY - 1) - 0.5f),
                0.0f
            );
            particles[i].oldPosition = particles[i].position;
        }
    }

    // pin the top corners
    particles[(numY - 1) * numX].isFixed = true;
    particles[numParticlesX * numY - 1].isFixed = true;

    // create springs
    springs.clear();
    for (int y = 0; y < numY; ++y) {
        for (int x = 0; x < numX; ++x) {
            int currentIdx = y * numX + x;
            // Structural springs
            if (x < numX - 1) {
                int rightIdx = y * numX + (x + 1);
                float restLength = glm::distance(particles[currentIdx].position, particles[rightIdx].position);
                springs.push_back({ &particles[currentIdx], &particles[rightIdx], restLength });
            }
            if (y < numY - 1) {
                int downIdx = (y + 1) * numX + x;
                float restLength = glm::distance(particles[currentIdx].position, particles[downIdx].position);
                springs.push_back({ &particles[currentIdx], &particles[downIdx], restLength });
            }

            // Shear springs (diagonals)
            if (x < numX - 1 && y < numY - 1) {
                int diagIdx = (y + 1) * numX + (x + 1);
                float restLength = glm::distance(particles[currentIdx].position, particles[diagIdx].position);
                springs.push_back({ &particles[currentIdx], &particles[diagIdx], restLength });
            }
            if (x > 0 && y < numY - 1) {
                int diagIdx = (y + 1) * numX + (x - 1);
                float restLength = glm::distance(particles[currentIdx].position, particles[diagIdx].position);
                springs.push_back({ &particles[currentIdx], &particles[diagIdx], restLength });
            }

            // Bending springs (skip-one connections)
            if (x < numX - 2) {
                int skipIdx = y * numX + (x + 2);
                float restLength = glm::distance(particles[currentIdx].position, particles[skipIdx].position);
                springs.push_back({ &particles[currentIdx], &particles[skipIdx], restLength });
            }
            if (y < numY - 2) {
                int skipIdx = (y + 2) * numX + x;
                float restLength = glm::distance(particles[currentIdx].position, particles[skipIdx].position);
                springs.push_back({ &particles[currentIdx], &particles[skipIdx], restLength });
            }
        }
    }

    // indices for rendering
    for (int y = 0; y < numY - 1; ++y) {
        for (int x = 0; x < numX - 1; ++x) {
            indices.push_back(y * numX + x);
            indices.push_back(y * numX + x + 1);
            indices.push_back((y + 1) * numX + x);

            indices.push_back((y + 1) * numX + x);
            indices.push_back(y * numX + x + 1);
            indices.push_back((y + 1) * numX + x + 1);
        }
    }
    updateVertexData();
}

void Cloth::update(float deltaTime) {
    const glm::vec3 gravity(0.0f, -9.81f, 0.0f);
    const float damping = 0.98f;
    const int numIterations = 7;
    const float gamma_p = 0.3f; // Safety factor

    static int frameCount = 0;
    bool shouldUpdateSpatialHash = (frameCount % 8 == 0); // Update every x frames
    bool shouldUpdateBounds = (frameCount % 16 == 0); // Update bounds every 6 frames
    frameCount++;

    //Only update spatial has
    if (shouldUpdateSpatialHash) {
        updateSpatialHash();
    }

    //Only ompute conservative bounds
    if (shouldUpdateBounds) {
        auto boundsStart = std::chrono::high_resolution_clock::now();
#pragma omp parallel for
        for (int i = 0; i < particles.size(); ++i) {
            auto& p = particles[i];
            if (p.isFixed) {
                p.conservativeBound = 0.0f;
                continue;
            }

            // Estimate movement from previous frame to guess query size
            glm::vec3 prevMovement = p.position - p.oldPosition;
            float estimatedMovement = glm::length(prevMovement) * 2.0f + 0.1f;

            std::vector<uint32_t> potentialColliders = spatialHash.getNearbyTriangles(p.position);

            float minDistance = 1.0f; // Default safe distance
            // Only check first 10 triangles (most likely candidates)
            int checks = std::min(10, (int)potentialColliders.size());
            for (int j = 0; j < checks; ++j) {
                uint32_t triStartIdx = potentialColliders[j];
                unsigned int i1 = indices[triStartIdx];
                unsigned int i2 = indices[triStartIdx + 1];
                unsigned int i3 = indices[triStartIdx + 2];

                // Skip adjacent triangles
                if (&p == &particles[i1] || &p == &particles[i2] || &p == &particles[i3])
                    continue;

                const glm::vec3& v1 = particles[i1].position;
                const glm::vec3& v2 = particles[i2].position;
                const glm::vec3& v3 = particles[i3].position;

                // Very fast distance approximation
                glm::vec3 centroid = (v1 + v2 + v3) * 0.333f;
                float dist = glm::distance(p.position, centroid);
                minDistance = std::min(minDistance, dist);
            }
            p.conservativeBound = minDistance * gamma_p;
            p.positionBeforeUpdate = p.position;
        }
        auto boundsEnd = std::chrono::high_resolution_clock::now();
        auto boundsTime = std::chrono::duration_cast<std::chrono::microseconds>(boundsEnd - boundsStart);
        //std::cout << "Bounds: " << boundsTime.count() << "us" << std::endl;
    }

    // Verlet integration
    for (auto& p : particles) {
        if (!p.isFixed) {
            glm::vec3 temp = p.position;
            p.position = p.position + (p.position - p.oldPosition) * damping + gravity * deltaTime * deltaTime;
            p.oldPosition = temp;
        }
    }

    // Spring constraints
    for (int i = 0; i < numIterations; ++i) {
        for (auto& spring : springs) {
            glm::vec3 delta = spring.p2->position - spring.p1->position;
            float currentLength = glm::length(delta);
            if (currentLength < 0.001f) continue;

            float stretch = currentLength - spring.restLength;
            if (abs(stretch) > 0.001f) {
                glm::vec3 direction = delta / currentLength;
                glm::vec3 force = direction * stretch * spring.stiffness * deltaTime;

                if (!spring.p1->isFixed) spring.p1->position += force * 0.5f;
                if (!spring.p2->isFixed) spring.p2->position -= force * 0.5f;
            }
        }
        //  trust region application
        if (shouldUpdateBounds) {
            for (auto& p : particles) {
                if (p.isFixed) continue;
                glm::vec3 displacement = p.position - p.positionBeforeUpdate;
                float displacementLength = glm::length(displacement);
                if (displacementLength > p.conservativeBound && p.conservativeBound > 0.01f) {
                    p.position = p.positionBeforeUpdate +
                        (displacement / displacementLength) * p.conservativeBound;
                }
            }
        }
    }
    updateVertexData();
}

void Cloth::applyForce(const glm::vec3& force) {
    for (auto& p : particles) {
        if (!p.isFixed) p.position += force / p.mass;
    }
}

void Cloth::applyForceToPoint(const glm::vec3& point, float radius, const glm::vec3& force) {
    for (auto& p : particles) {
        if (!p.isFixed && glm::distance(point, p.position) < radius) {
            p.oldPosition += force / p.mass;
        }
    }
}

void Cloth::updateVertexData() {
    vertexPositions.resize(particles.size());
    for (size_t i = 0; i < particles.size(); ++i) {
        vertexPositions[i] = particles[i].position;
    }
    computeNormals();
}

void Cloth::computeNormals() {
    normals.assign(particles.size(), glm::vec3(0.0f));
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i1 = indices[i];
        unsigned int i2 = indices[i + 1];
        unsigned int i3 = indices[i + 2];

        const glm::vec3& p1 = vertexPositions[i1];
        const glm::vec3& p2 = vertexPositions[i2];
        const glm::vec3& p3 = vertexPositions[i3];

        glm::vec3 normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));

        normals[i1] += normal;
        normals[i2] += normal;
        normals[i3] += normal;
    }
    for (auto& n : normals) {
        n = glm::normalize(n);
    }
}

void Cloth::updateSpatialHash() {
    spatialHash.clear();

    const auto& vertices = getVertexPositions();
    const auto& indices = getIndices();

    for (size_t i = 0; i < indices.size(); i += 3) {
        const glm::vec3& v1 = vertices[indices[i]];
        const glm::vec3& v2 = vertices[indices[i + 1]];
        const glm::vec3& v3 = vertices[indices[i + 2]];

        spatialHash.insertTriangle(i, v1, v2, v3);
    }
}