#pragma once

#include "ofMain.h"

class ParticleEnsemble {
public:
    void draw() const;

    std::vector<glm::vec3> positions;
    std::vector<float> radii;
    std::vector<float> masses;
};
