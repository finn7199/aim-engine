#include "Cloth.h"

Cloth::Cloth(float width, float height, int numX, int numY)
    : numParticlesX(numX), numParticlesY(numY) {

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
    for (int y = 0; y < numY; ++y) {
        for (int x = 0; x < numX; ++x) {
            auto addSpring = [&](int x1, int y1, int x2, int y2) {
                if (x1 >= 0 && x1 < numX && y1 >= 0 && y1 < numY &&
                    x2 >= 0 && x2 < numX && y2 >= 0 && y2 < numY) {
                    Particle* p1 = &particles[y1 * numX + x1];
                    Particle* p2 = &particles[y2 * numX + x2];
                    springs.push_back({ p1, p2, glm::distance(p1->position, p2->position) });
                }
                };

            // Structural springs
            addSpring(x, y, x + 1, y);
            addSpring(x, y, x, y + 1);

            // Shear springs
            addSpring(x, y, x + 1, y + 1);
            addSpring(x, y, x - 1, y + 1);

            // Bending springs
            addSpring(x, y, x + 2, y);
            addSpring(x, y, x, y + 2);
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
    const float damping = 0.99f;
    const int numIterations = 20; // solve constraints multiple times for stability

    for (auto& p : particles) {
        //p.acceleration += gravity; // add gravity
    }

    // verlet integration
    for (auto& p : particles) {
        if (!p.isFixed) {
            glm::vec3 temp = p.position;
            p.position += (p.position - p.oldPosition) * damping + gravity * deltaTime * deltaTime;
            p.oldPosition = temp;
        }
    }

    // apply spring forces
    for (int i = 0; i < numIterations; ++i) {
        for (auto& spring : springs) {
            glm::vec3 delta = spring.p2->position - spring.p1->position;
            float currentLength = glm::length(delta);
            if (currentLength == 0.0f) continue;

            float diff = (currentLength - spring.restLength) / currentLength;
            glm::vec3 correction = 0.5f * diff * delta;

            if (!spring.p1->isFixed)
                spring.p1->position += correction;
            if (!spring.p2->isFixed)
                spring.p2->position -= correction;
        }
    }

    updateVertexData(); // update vertex data for render
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