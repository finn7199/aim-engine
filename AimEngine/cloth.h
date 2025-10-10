#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>
#include "spatialhash.h"

struct Particle {
    bool isFixed = false;
    float mass = 1.0f;
    glm::vec3 position;
    glm::vec3 oldPosition;
    //glm::vec3 acceleration;

    // trust region collition check
    glm::vec3 positionBeforeUpdate;
    float conservativeBound;
};

struct Spring {
    Particle* p1;
    Particle* p2;
    float restLength;
    float stiffness = 200.0f; // how strong the spring is
};

class Cloth {
public:
    Cloth(float width, float height, int numParticlesX, int numParticlesY);

    void update(float deltaTime);
    void applyForce(const glm::vec3& force); // apply a force to the whole cloth
    void applyForceToPoint(const glm::vec3& point, float radius, const glm::vec3& force); // bullet point force

    const std::vector<glm::vec3>& getVertexPositions() const { return vertexPositions; }
    const std::vector<unsigned int>& getIndices() const { return indices; }
    const std::vector<glm::vec3>& getNormals() const { return normals; }

private:
    void updateVertexData();
    void computeNormals();

    int numParticlesX, numParticlesY;
    std::vector<Particle> particles;
    std::vector<Spring> springs;

    // rendering data
    std::vector<glm::vec3> vertexPositions;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    // Spatial hashing
    SpatialHash spatialHash;
    void updateSpatialHash();
};