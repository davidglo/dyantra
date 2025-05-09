#pragma once

#include "ofMain.h"
#include "attractor.h"  // Include the attractor class

class particleEnsemble {
public:
    particleEnsemble(); // Constructor

    void draw() const;
    void drawVBO();
    void initialize(const std::vector<glm::vec3>& initialPositions); // Initialization function
    void ZeroForces() {  // Zero forces function
        for (auto& force : f) {
            force = glm::vec3(0, 0, 0);
        }
    };

    void radial_update(float dt, float angularVelocity, const glm::vec3& midpoint); // New update function
    void vv_propagatePositionsVelocities(const std::vector<attractor>& attractorVec, float dt); // Velocity Verlet update function
    
    void reinitialize(const std::vector<glm::vec3>& initialPositions);
    void update(const std::vector<glm::vec3>& initialPositions);
    
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> last_positions; // Data member to save positions from the previous timestep
    std::vector<glm::vec3> v; // Data member to save velocities for the present timestep
    std::vector<glm::vec3> f; // Data member to save forces acting on each particle at the present timestep
    std::vector<glm::vec3> last_f; // Data member to save forces acting on each particle at the previous timestep

    std::vector<float> radii;
    std::vector<float> masses;
    
    const std::vector<glm::vec3>& getPositions() const {
        return positions;
    }
    
private:
    glm::vec3 calculateGaussianForce(const attractor& attractorObject, const glm::vec3& particlePosition) const; // Helper function
    
    ofImage texture;
    ofVbo vbo;
    ofShader shader;
};

