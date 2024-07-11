#pragma once

#include "ofMain.h"

class particleEnsemble {
public:
    particleEnsemble(); // Constructor

    void draw() const;
    void initialize(const std::vector<glm::vec3>& initialPositions); // Initialization function
    void translate(const ofPoint& offset); // Translate function
    void resize(float scale, const ofPoint& centroid); // Resize function
    void ZeroForces(); // Zero forces function

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> last_positions; // Data member to save positions from the previous timestep
    std::vector<glm::vec3> v; // Data member to save velocities for the present timestep
    std::vector<glm::vec3> f; // Data member to save forces acting on each particle at the present timestep
    std::vector<glm::vec3> last_f; // Data member to save forces acting on each particle at the previous timestep

    std::vector<float> radii;
    std::vector<float> masses;
};

inline void particleEnsemble::ZeroForces() {
    for (auto& force : f) {
        force = glm::vec3(0, 0, 0);
    }
}
