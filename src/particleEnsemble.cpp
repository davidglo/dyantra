#include "particleEnsemble.h"

ParticleEnsemble::ParticleEnsemble() {
    // Default constructor
}

void ParticleEnsemble::initialize(const std::vector<glm::vec3>& initialPositions) {
    positions = initialPositions;
    last_positions = initialPositions;
    v.resize(positions.size(), glm::vec3(0, 0, 0));
    f.resize(positions.size(), glm::vec3(0, 0, 0));
    last_f.resize(positions.size(), glm::vec3(0, 0, 0));
    radii.resize(positions.size(), 1.0f); // Example radius initialization
    masses.resize(positions.size(), 1.0f); // Example mass initialization
}

void ParticleEnsemble::translate(const ofPoint& offset) {
    for (auto& position : positions) {
        position += offset;
    }
}

void ParticleEnsemble::resize(float scale, const ofPoint& centroid) {
    for (auto& position : positions) {
        position = centroid + (position - centroid) * scale;
    }
}

void ParticleEnsemble::draw() const {
    ofFill();
    for (size_t i = 0; i < positions.size(); ++i) {
        ofDrawCircle(positions[i], radii[i]);
    }
}
